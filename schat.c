//Puthipong Yomabut 6513134
//Phattaradanai Sornsawang 6513172
//Patiharn Kamenkit 6513170
//Praphasiri wannawong 6513116
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

//===================================================
// command for looking for message queue is ipcs -q
//===================================================

/*
  command for execute this file is ./qchat 1
  where 1 is sender and 2 is receiver automatically.
  we have to execute on another shell as a argument 2
*/

#define MEM_SIZE 4096
#define key_value 21930

struct shm_st {
  int type;
  char mtext[BUFSIZ];
};

static void sig_end() { exit(EXIT_SUCCESS); }

int main(int argc, char *argv[]) {
  int shmid;
  void *sh_mem = NULL;
  struct shm_st *sh_area;
  pid_t pid;

  if (argc < 2 || (strcmp(argv[1], "1") != 0 && strcmp(argv[1], "2") != 0) || argc > 2) {
    fprintf(stderr, "Usage: %s [1 or 2]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  shmid = shmget((key_t)key_value, MEM_SIZE, 0666 | IPC_CREAT);
  if (shmid == -1) {
    perror("shmget");
    exit(EXIT_FAILURE);
  }
  sh_mem = shmat(shmid, NULL, 0);
  if (sh_mem == (void *)-1) {
    perror("shmat");
    exit(EXIT_FAILURE);
  }

  sh_area = (struct shm_st *)sh_mem;
  sh_area->type = 0;

  signal(SIGUSR1, sig_end);
  argv++;

  if (strcmp(*argv, "1") == 0) {
    pid = fork();
    switch (pid) {
    case -1:
      perror("Forking failed");
      exit(EXIT_FAILURE);
    case 0:
      while (strncmp(sh_area->mtext, "end chat", 8)) {
        if (sh_area->type == 2) {
          printf("%s", sh_area->mtext);
          sh_area->type = 0;
        }
      }
      break;
    default:
      while (strncmp(sh_area->mtext, "end chat", 8)) {
        fgets(sh_area->mtext, MEM_SIZE, stdin);
        sh_area->type = 1;
      }
      break;
    }
  } else if (strcmp(*argv, "2") == 0) {
    pid = fork();
    switch (pid) {
    case -1:
      perror("Forking failed");
      exit(EXIT_FAILURE);
    case 0:
      while (strncmp(sh_area->mtext, "end chat", 8)) {
        if (sh_area->type == 1) {
          printf("%s", sh_area->mtext);
          sh_area->type = 0;
        }
      }
      break;
    default:
      while (strncmp(sh_area->mtext, "end chat", 8)) {
        fgets(sh_area->mtext, MEM_SIZE, stdin);
        sh_area->type = 2;
      }
      break;
    }
  }

  if (pid > 0)
    kill(pid, SIGUSR1);
  else if (pid == 0)
    kill(getppid(), SIGUSR1);

  if (shmdt(sh_mem) == -1) {
    perror("shmdt");
    exit(EXIT_FAILURE);
  }

  if (shmctl(shmid, IPC_RMID, 0) == -1) {
    perror("shmctl");
    exit(EXIT_FAILURE);
  }

  return 0;
}
