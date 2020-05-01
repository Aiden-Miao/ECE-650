#include <arpa/inet.h>
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

class player {
 public:
  int num_player;
  int ringmaster_fd;
  //int client_fd;
  int server_fd;        //fd as server
  int to_neighbour_fd;  //fd to neighbour
  int new_fd;           //the fd for server accept
  //int client_connectio;
  int player_id;
  player() : num_player(0), ringmaster_fd(-1), player_id(-1) {}

  ~player() {
    //close(client_fd);
    close(new_fd);
    close(ringmaster_fd);
    close(server_fd);
    close(to_neighbour_fd);
  }
  void ringmaster_connect(char ** argv) {
    int status;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;
    const char * hostname;
    const char * port;
    hostname = argv[1];
    port = argv[2];
    //getaddress_info
    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
      std::cerr << "Error: cannot get address info for host" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    }

    //get the socket of ringmaster

    ringmaster_fd = socket(host_info_list->ai_family,
                           host_info_list->ai_socktype,
                           host_info_list->ai_protocol);
    if (ringmaster_fd == -1) {
      std::cerr << "Error: cannot create socket" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    }

    //connect to ringmaster
    status = connect(ringmaster_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
      std::cerr << "Error: cannot connect to socket" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    }

    //recieve player id
    recv(ringmaster_fd, &player_id, sizeof(player_id), 0);

    //recieve player numbers
    recv(ringmaster_fd, &num_player, sizeof(num_player), 0);

    //initialize player server, so we can get the port number
    int port_number = init_player_server();

    //send the port number to ringmaster
    send(ringmaster_fd, &port_number, sizeof(port_number), 0);
    printf("Connected as player %d out of %d total players\n", player_id, num_player);
  }

  //initialize our player as server, and send back the port number
  int init_player_server() {
    int status;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;
    //const char * hostname = NULL;
    const char * port = "0";  //default port here
    char hostname[100];

    //get the hostname of our local machine
    if (gethostname(hostname, sizeof(hostname)) == -1) {
      perror("unable to get host name!\n");
    }

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo(hostname, NULL, &host_info, &host_info_list);
    if (status != 0) {
      std::cerr << "Error: cannot get address info for host" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    }

    //let os assign our port number
    struct sockaddr_in * addr_in = (struct sockaddr_in *)(host_info_list->ai_addr);
    addr_in->sin_port = 0;

    //creat socket()
    server_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (server_fd == -1) {
      std::cerr << "Error: cannot create socket" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    }

    //bind socket with port
    int yes = 1;
    status = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(server_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
      std::cerr << "Error: cannot bind socket" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    }

    //start listen
    status = listen(server_fd, 100);
    if (status == -1) {
      std::cerr << "Error: cannot listen on socket" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    }
    //printf("this is executed twice!!\n");
    //send back the port number
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(server_fd, (struct sockaddr *)&sin, &len);
    return ntohs(sin.sin_port);
  }

  void connect_circle() {
    int addr_size;
    char addr[100];
    int port;
    //make sure addr is clean
    memset(&addr, 0, 100);
    //recieve the port number
    recv(ringmaster_fd, &port, sizeof(port), MSG_WAITALL);

    //recieve the length of ip address
    recv(ringmaster_fd, &addr_size, sizeof(addr_size), MSG_WAITALL);
    //recieve the ip address
    recv(ringmaster_fd, &addr, addr_size, MSG_WAITALL);
    //connect with the right neighbours

    int status;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    char real_port[100];
    //cast port from int to char[]
    //double check here later
    sprintf(real_port, "%d", port);
    status = getaddrinfo(addr, real_port, &host_info, &host_info_list);
    if (status != 0) {
      std::cerr << "Error: cannot get address info for host" << std::endl;
      //change hostname to to address in here
      std::cerr << "  (" << addr << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    }

    //get the socket

    to_neighbour_fd = socket(host_info_list->ai_family,
                             host_info_list->ai_socktype,
                             host_info_list->ai_protocol);
    if (to_neighbour_fd == -1) {
      std::cerr << "Error: cannot create socket" << std::endl;
      //change host_name here too
      std::cerr << "  (" << addr << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    }

    //connect()
    status =
        connect(to_neighbour_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
      std::cerr << "Error: cannot connect to socket" << std::endl;
      std::cerr << "  (" << addr << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    }

    //after we succeed, send a message to ringmaster
    int success = player_id;
    send(ringmaster_fd, &success, sizeof(success), 0);

    //accept connect from the left neighbour
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    new_fd = accept(server_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (new_fd == -1) {
      std::cerr << "Error: cannot accept connection on socket" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  int bigger(int x, int y) {
    if (x > y) {
      return x;
    }
    else {
      return y;
    }
  }
  void pass_potato() {
    //potato player_potato;  //use this potato to store the potato we get
    fd_set readfds;
    int left_neigh = (player_id + 1) % num_player;
    int right_neigh = (player_id - 1 + num_player) % num_player;
    int fds[] = {new_fd, to_neighbour_fd, ringmaster_fd};
    int max_fd = bigger((bigger(new_fd, ringmaster_fd)), to_neighbour_fd) + 1;
    srand(time(0));
    while (true) {
      potato player_potato;  //use this potato to store the potato we get
      FD_ZERO(&readfds);
      //add all 3 direction to fd_sets to listen
      for (int i = 0; i < 3; i++) {
        FD_SET(fds[i], &readfds);
      }

      int status = select(max_fd, &readfds, NULL, NULL, NULL);
      if (status == -1) {
        perror("select");
      }

      for (int i = 0; i < 3; i++) {
        if (FD_ISSET(fds[i], &readfds)) {
          //int temp;
          recv(fds[i], &player_potato, sizeof(player_potato), MSG_WAITALL);
          //  sizeof(player_potato)) {
          //perror("iii recieved a broken potato!");

          break;
        }
      }
      //printf("The Hop is:%d\n", player_potato.hops);
      //before we throw the potato, see if the hop is 0
      if (player_potato.hops == 0 && player_potato.hops_cnt == 0) {
        break;
      }
      //choose left or right to send
      //srand(time(0));
      int randnum = rand() % 2;

      //normal situation
      player_potato.hops--;
      player_potato.trace[player_potato.hops_cnt] = player_id;
      player_potato.hops_cnt++;
      if (player_potato.hops == 0) {
        printf("I'm it\n");
        //send back to ringmaster
        send(ringmaster_fd, &player_potato, sizeof(player_potato), 0);
        //sleep(1);
        break;
      }

      if (randnum == 0) {
        printf("Sending potato to %d\n", left_neigh);
        send(new_fd, &player_potato, sizeof(player_potato), 0);
      }
      else {
        printf("Sending potato to %d\n", right_neigh);
        send(to_neighbour_fd, &player_potato, sizeof(player_potato), 0);
      }
    }
  }
};

int main(int argc, char ** argv) {
  player * myplayer = new player();
  myplayer->ringmaster_connect(argv);
  //myplayer->init_player_server();
  myplayer->connect_circle();
  //  printf("Connected as player %d out of %d total players\n",
  //     myplayer->player_id,
  //     myplayer->num_player);
  myplayer->pass_potato();
  delete myplayer;
  return EXIT_SUCCESS;
}
