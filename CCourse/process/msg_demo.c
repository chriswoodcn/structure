#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>

struct msgbuf {
  long mtype;
  char mtext[80];
};

static void usage(char *prog_name, char *msg, FILE *stream) {
  if (stream == NULL) {
    stream = stdout
  }
  if (msg != NULL)
    fputs(msg, stream);

  fprintf(stream, "Usage: %s [options]\n", prog_name);
  fprintf(stream, "Options are:\n");
  fprintf(stream, "-s        send message using msgsnd()\n");
  fprintf(stream, "-r        read message using msgrcv()\n");
  fprintf(stream, "-t        message type (default is 1)\n");
  fprintf(stream, "-k        message queue key (default is 1234)\n");
  exit(EXIT_FAILURE);
}

static void send_msg(int qid, int msgtype) {
  time_t t;
  struct msgbuf msg;

  msg.mtype = msgtype;

  time(&t);
  snprintf(msg.mtext, sizeof(msg.mtext), "a message at %s", ctime(&t));

  if (msgsnd(qid, &msg, sizeof(msg.mtext), IPC_NOWAIT) == -1) {
    perror("msgsnd error");
    exit(EXIT_FAILURE);
  }
  printf("sent: %s\n", msg.mtext);
}

static void get_msg(int qid, int msgtype) {
  struct msgbuf msg;

  if (msgrcv(qid, &msg, sizeof(msg.mtext), msgtype, MSG_NOERROR | IPC_NOWAIT) ==
      -1) {
    if (errno != ENOMSG) {
      perror("msgrcv");
      exit(EXIT_FAILURE);
    }
    printf("No message available for msgrcv()\n");
  } else {
    printf("message received: %s\n", msg.mtext);
  }
}

int main(int argc, char *argv[]) {
  int qid, opt;
  int mode = 0; /* 1 = send, 2 = receive */
  int msgtype = 1;
  int msgkey = 1234;

  while ((opt = getopt(argc, argv, "srt:k:")) != -1) {
    switch (opt) {
    case 's':
      mode = 1;
      break;
    case 'r':
      mode = 2;
      break;
    case 't':
      msgtype = atoi(optarg);
      if (msgtype <= 0)
        usage(argv[0], "-t option must be greater than 0\n", NULL);
      break;
    case 'k':
      msgkey = atoi(optarg);
      break;
    default:
      usage(argv[0], "Unrecognized option\n", NULL);
    }
  }

  if (mode == 0)
    usage(argv[0], "must use either -s or -r option\n", NULL);

  qid = msgget(msgkey, IPC_CREAT | 0666);

  if (qid == -1) {
    perror("msgget");
    exit(EXIT_FAILURE);
  }

  if (mode == 2)
    get_msg(qid, msgtype);
  else
    send_msg(qid, msgtype);

  exit(EXIT_SUCCESS);
}