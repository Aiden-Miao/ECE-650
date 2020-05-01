#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>
#include <string>
#include <vector>

#include "potato.h"

class ringmaster {
 public:
  int status;
  int sock_fd;
  struct addrinfo host_info;
  struct addrinfo * host_info_list;
  const char * hostname;
  const char * port;
  int num_player;
  int num_hops;
  std::vector<int> player_fd;
  std::vector<int> player_port;
  std::vector<std::string> player_ip;
  //<port_num>, <num_player>, <num_hops>
  //initialize status,hostname,sock_fd
  ringmaster(char ** argv) :
      status(0),
      sock_fd(0),
      hostname(NULL),
      port(argv[1]),
      num_player(atoi(argv[2])),
      num_hops(atoi(argv[3])) {
    //change the size of the vectors to fit all players
    player_fd.resize(num_player);
    player_port.resize(num_player);
    player_ip.resize(num_player);
  }

  ~ringmaster() {
    for (int i = 0; i < num_player; i++) {
      close(player_fd[i]);
    }
    close(sock_fd);
  }
  void get_address_info() {
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
      std::cerr << "Error: cannot get address info for host" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      //do we need to clear host_info here?
      exit(EXIT_FAILURE);
    }
  }

  //create a socket, bind it to a port and start listening
  void build_socket() {
    sock_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
    if (sock_fd == -1) {
      std::cerr << "Error: cannot create socket" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      //do we need to clear?
      exit(EXIT_FAILURE);
    }

    //bind socket to port
    int yes = 1;
    status = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(sock_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
      std::cerr << "Error: cannot bind socket" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      //do we need to clear?
      exit(EXIT_FAILURE);
    }

    //start listen
    status = listen(sock_fd, 100);
    if (status == -1) {
      std::cerr << "Error: cannot listen on socket" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      //do we need to clear?
      exit(EXIT_FAILURE);
    }
    freeaddrinfo(host_info_list);
  }

  //connect ringmaster with all the player and send info to them
  void start_connect() {
    for (int i = 0; i < num_player; i++) {
      struct sockaddr_storage socket_addr;
      socklen_t socket_addr_len = sizeof(socket_addr);
      player_fd[i] = accept(sock_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
      if (player_fd[i] == -1) {
        std::cerr << "Error: cannot accept connection on socket" << std::endl;
        exit(EXIT_FAILURE);
      }
      //get the player ip
      struct sockaddr_in * antelope = (struct sockaddr_in *)&socket_addr;
      player_ip[i] = inet_ntoa(antelope->sin_addr);
      //send player id and num_players to all the other players
      send(player_fd[i], &i, sizeof(i), 0);
      send(player_fd[i], &num_player, sizeof(num_player), 0);
      //recieve the port number of all the players
      recv(player_fd[i], &player_port[i], sizeof(player_port[i]), MSG_WAITALL);
    }
  }

  //send all the info of our player's right neighbour
  void send_neigh_info() {
    //send info of 0 to 5, 5 to 4, 4 to 3, 3 to 2...
    for (int i = 0; i < num_player; i++) {
      //send the info of i to (i+1)%(num)

      //send port number;
      send(player_fd[(i + 1) % (num_player)], &player_port[i], sizeof(player_port[i]), 0);

      //because player doesn't know the size of the address, we need to send the size of the address to our player first
      int sizeofip = player_ip[i].length();
      send(player_fd[(i + 1) % (num_player)], &sizeofip, sizeof(sizeofip), 0);

      //send the ip address to other player
      send(player_fd[(i + 1) % (num_player)], player_ip[i].c_str(), sizeofip, 0);

      //get the success signal

      int temp;
      recv(player_fd[(i + 1) % (num_player)], &temp, sizeof(temp), MSG_WAITALL);
      // (i + 1) % (num_player));
      printf("Player %d is ready to play\n", temp);
    }
  }

  void start_game() {
    srand(time(0));
    potato mypotato;
    mypotato.hops = num_hops;
    mypotato.total = num_hops;  //store the total number
    if (num_hops == 0) {        //if nums_hops=0, end the game
      for (int i = 0; i < num_player; i++) {
        if (send(player_fd[i], &mypotato, sizeof(mypotato), 0) != sizeof(mypotato)) {
          perror("send a broken potato!\n");
        }
      }
      return;
    }
    //add seed in here to get real random number

    int rand_player = rand() % num_player;
    std::cout << "Ready to start the game, sending potato to player " << rand_player
              << std::endl;
    if (send(player_fd[rand_player], &mypotato, sizeof(mypotato), 0) !=
        sizeof(mypotato)) {
      perror("send a broken potato!\n");
    }

    //clear read buffer
    fd_set readfds;
    FD_ZERO(&readfds);
    for (int i = 0; i < num_player; i++) {
      FD_SET(player_fd[i], &readfds);
    }
    int rv = select(player_fd[num_player - 1] + 1, &readfds, NULL, NULL, NULL);
    if (rv == -1) {
      perror("select");
    }
    for (int i = 0; i < num_player; i++) {
      if (FD_ISSET(player_fd[i], &readfds)) {
        if (recv(player_fd[i], &mypotato, sizeof(mypotato), MSG_WAITALL) !=
            sizeof(mypotato)) {
          perror("recieved a broken potato!");
        }

        //close all the player
        //assert(mypotato.hops == 0);
        mypotato.print_trace();
        potato endpotato;
        //printf("test1\n");
        for (int j = 0; j < num_player; j++) {
          send(player_fd[i], &endpotato, sizeof(endpotato), 0);
        }
        return;
      }
    }
    return;
  }
};
int main(int argc, char ** argv) {
  ringmaster * master = new ringmaster(argv);
  printf("Potato Ringmaster\n");
  printf("Players = %d\n", master->num_player);
  printf("Hops = %d\n", master->num_hops);
  master->get_address_info();
  master->build_socket();
  master->start_connect();
  master->send_neigh_info();
  master->start_game();
  delete master;
  return EXIT_SUCCESS;
}
