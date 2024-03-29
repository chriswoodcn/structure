#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 128

void serv_list(int connfd);
void serv_put(int connfd, char buf[]);
void serv_get(int connfd);

int main(int argc, char *argv[]) {
  int sockfd, connfd;
  struct sockaddr_in servaddr, cliaddr;
  socklen_t peerlen;
  char buf[BUFFER_SIZE];

  if (argc < 3) {
    printf("Usage : %s <ip> <port>\n", argv[0]);
    exit(-1);
  }

  /*建立Socket连接*/
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(-1);
  }
  printf("sockfd = %d\n", sockfd);

  /*设置sockaddr_in 结构体中相关参数*/
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2]));
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);

  /*绑定函数bind()*/
  if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind error");
    exit(-1);
  }
  printf("bind success!\n");

  /*调用listen()函数，设置监听模式*/
  if (listen(sockfd, 10) == -1) {
    perror("listen error");
    exit(-1);
  }
  printf("Listening....\n");

  /*调用accept()函数，等待客户端的连接*/
  peerlen = sizeof(cliaddr);
  while (1) {
    if ((connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &peerlen)) < 0) {
      perror("accept error");
      exit(-1);
    }
    printf("connfd = %d\n", connfd);
    memset(buf, 0, sizeof(buf));
    while (1) {
      ssize_t recved = recv(connfd, buf, BUFFER_SIZE, 0);
      if (recved == -1) {
        perror("accept recv error");
        break;
      } else if (recved == 0) {
        close(connfd);
        break;
      } else {
        if (buf[strlen(buf) - 1] == '\n') {
          buf[strlen(buf) - 1] = '\0';
        }
        switch (buf[0]) {
        case 'L':
          printf("list command ======\n");
          serv_list(connfd);
          break;
        case 'P':
          printf("put command ======\n");
          serv_put(connfd, buf + 2);
          break;
        case 'G':
          printf("get command ======\n");
          serv_get(connfd);
          break;
        default:
          printf("Unknow command\n");
          sprintf(buf, "%s", "Unknow command");
          send(connfd, buf, sizeof(buf), 0);
          break;
        }
      }
    }
  }
  close(sockfd);
  exit(0);
}

void serv_list(int connfd) {
  DIR *dist = opendir(".");
  struct dirent *dir_info;
  char buf[BUFFER_SIZE] = {0};
  while ((dir_info = readdir(dist)) != NULL) {
    printf("d_name = %s\n", dir_info->d_name);
    printf("d_type = %d\n", dir_info->d_type);
    if (dir_info->d_type == 8) {
      // send(connfd, dir_info->d_name, sizeof(dir_info->d_name), 0);
      memset(buf, 0, BUFFER_SIZE);
      strcpy(buf, dir_info->d_name);
      send(connfd, buf, sizeof(buf), 0);
    }
  }
  sprintf(buf, "%s", "QUIT");
  send(connfd, buf, sizeof(buf), 0);
  printf("list command ====== OK\n");
}

void serv_put(int connfd, char *name) {
  char buf[BUFFER_SIZE] = {0};
  int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0) {
    perror("open file error");
    sprintf(buf, "%s", "ERROR");
    send(connfd, buf, sizeof(buf), 0);
    return;
  }
  int recvSize;
  while ((recvSize = recv(connfd, buf, sizeof(buf), 0)) != 0) {
    int compare = strncmp(buf, "FINISH", 6);
    printf("compare = %d\n", compare);
    printf("buf = %s\n", buf);
    if (compare == 0) {
      break;
    } else {
      write(fd, buf, recvSize);
      usleep(500);
    }
  }
  close(fd);
  printf("put %s command ====== OK\n", name);
}

void serv_get(int connfd) {}
