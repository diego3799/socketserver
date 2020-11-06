
#include <netinet/in.h> /* TCP/IP library                      */
#include <arpa/inet.h>  /* Newer functionality for  the TCP/IP */
                        /* library                             */
#include <sys/socket.h> /* sockets library                     */
#include <sys/types.h>  /* shared data types                   */
#include <stdio.h>      /* standard input/output               */
#include <unistd.h>     /* unix standard functions             */
#include <string.h>     /* text handling functions             */
#include <pthread.h>    /* libraries for thread handling       */
#define BUFFERSIZE 1024 /* buffer size                         */

struct data
{
  int data_type;                                  /* type of data sent         */
  int chat_id;                                    /* chat id sent by server    */
  char data_text[BUFFERSIZE - (sizeof(int) * 2)]; /* data sent                 */
};

typedef struct arguments
{
  void *socket_des;
  void *socket_write;
  void *messageHeartbeat;
} Args;

void *
print_message(void *ptr)
{
  int *sock_desc;         /* pointer for parameter              */
  int read_char;          /* read characters                    */
  char text1[BUFFERSIZE]; /* reading buffer                     */

  sock_desc = (int *)ptr;
  while (1)
  {
    read_char = recvfrom(*sock_desc, text1, BUFFERSIZE, 0, NULL, NULL);
    text1[read_char] = '\0';
    printf("%s\n", text1);
  }
}

void *send_heart_beat(void *ptr)
{
  Args *argumentsTh = (Args *)ptr;
  struct data heartBeatMessage;
  int *chat_id = (int *)argumentsTh->messageHeartbeat;
  heartBeatMessage.chat_id = *chat_id;
  heartBeatMessage.data_type = 10;
  strcpy(heartBeatMessage.data_text, "Im still alive!");
  int *sfd = (int *)argumentsTh->socket_des;
  struct sockaddr_in *sock_write = (struct sockaddr_in *)argumentsTh->socket_write;
  while (1)
  {
    sendto(*sfd, (struct data *)&(heartBeatMessage), sizeof(struct data), 0, (struct sockaddr *)&(*sock_write), sizeof(*sock_write));
    sleep(10);
  }
}

main()
{
  struct sockaddr_in sock_write; /* structure for the write socket      */
  struct data message;           /* message to sendto the server        */
  char text1[BUFFERSIZE];        /* reading buffer                      */
  char *auxptr;                  /* auxiliar char pointer               */
  int read_char;                 /* read characters                     */
  int i;                         /* counter                             */
  int sfd;                       /* socket descriptor                   */
  int chat_id;                   /* identificator in the chat session   */
  int iret1;                     /* thread return value                 */
  pthread_t thread1;             /* thread id                           */
  pthread_t heartbeat;
  Args args;

  /* ---------------------------------------------------------------------- */
  sock_write.sin_family = AF_INET;
  sock_write.sin_port = 27007;

  printf("Diego Adrián Hernández Zárate\nJuan Carlos Lara Anaya\nLuis Gerardo Huerta Sanchez\nCarlos Edoardo Leon Vera\n");
  // inet_aton("200.13.89.15", (struct in_addr *)&sock_write.sin_addr);
  inet_aton("localhost", (struct in_addr *)&sock_write.sin_addr);
  memset(sock_write.sin_zero, 0, 8);

  sfd = socket(AF_INET, SOCK_DGRAM, 0);

  printf("Please provide an alias: ");
  message.chat_id = 0;
  message.data_type = 0; /* data_type 0 is used to send alias   */
  message.data_text[0] = '\0';
  fgets(message.data_text, BUFFERSIZE - (sizeof(int) * 2), stdin);
  for (auxptr = message.data_text; *auxptr != '\n'; ++auxptr)
    ;
  *auxptr = '\0';

  /* sending of information to log in chat room                             */
  sendto(sfd, (struct data *)&(message), sizeof(struct data), 0, (struct sockaddr *)&(sock_write), sizeof(sock_write));
  read_char = recvfrom(sfd, (int *)&(chat_id), sizeof(int), 0, NULL, NULL);
  if (chat_id == -1) /* client rejected                     */
  {
    printf("Client could not join. Too many participants in room\n");
    close(sfd);
    return (0);
  }
  args.socket_des = (void *)&sfd;
  args.socket_write = (void *)&sock_write;
  args.messageHeartbeat = (void *)&chat_id;
  // args.messageHeartbeat
  /**Creation of the heartbeat thread*/
  pthread_create(&heartbeat, NULL, send_heart_beat, (void *)&args);

  /* Creation of reading thread                                            */
  iret1 = pthread_create(&thread1, NULL, print_message, (void *)(&sfd));

  while ((strcmp(message.data_text, "exit") != 0) && (strcmp(message.data_text, "shutdown") != 0))
  {
    printf("$ ");

    /* assembling of the message to send                                  */
    message.data_type = 1; /* data_type 1 is used to send message */
    fgets(message.data_text, BUFFERSIZE - (sizeof(int) * 2), stdin);
    for (auxptr = message.data_text; *auxptr != '\n'; ++auxptr)
      ;
    *auxptr = '\0';
    message.chat_id = chat_id;

    sendto(sfd, (struct data *)&(message), sizeof(struct data), 0, (struct sockaddr *)&(sock_write), sizeof(sock_write));
  }
  close(sfd);
  return (0);
}
