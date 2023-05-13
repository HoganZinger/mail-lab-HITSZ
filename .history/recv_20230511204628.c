#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535

char buf[MAX_SIZE+1];

#define swap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF))

int new_recv(int send_fd, char* buf, char* err_message)
{
    int r_size = -1;
    if((r_size = recv(send_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror(err_message);
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);
    return r_size;
}

void recv_mail()
{
    const char* host_name = "smtp.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = ""; // TODO: Specify the user
    const char* pass = ""; // TODO: Specify the password
    char dest_ip[16];
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server

    if((s_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in* servaddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in)) ;
    servaddr->sin_family = AF_INET;
    servaddr->sin_port = swap16(port);
    bzero(&servaddr->sin_zero,sizeof(servaddr->sin_zero));
    servaddr->sin_addr = (struct in_addr){inet_addr(dest_ip)};

    if(connect(s_fd,(struct sockaddr *)servaddr,sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // TODO: Send user and password and print server response
    sprintf(buf,"user %s@qq.com\r\n",user);
    send(s_fd,buf,strlen(buf),0);
    printf("\033[1;32m%s\033[0m", buf);
    new_recv(s_fd,buf,"send_username");

    sprintf(buf,"pass %s\r\n",pass);
    send(s_fd,buf,strlen(buf),0);
    printf("\033[1;32m%s\033[0m", buf);
    new_recv(s_fd,buf,"send_password");


    // TODO: Send STAT command and print server response
    send(s_fd, "STAT\r\n", strlen("STAT\r\n"), 0);
    print_msg(&r_size, s_fd);

    // TODO: Send LIST command and print server response
    const char* LIST = "LIST\r\n";
    send(s_fd,LIST,strlen(LIST),0);
    printf("\033[1;32m%s\033[0m", LIST);
    new_recv(s_fd,buf,"LIST");
    // length of the first mail
    int total_length = atoi(buf+6); 

    // TODO: Retrieve the first mail and print its content
    const char* RETR = "RETR 1\r\n";
    send(s_fd,RETR,strlen(RETR),0);
    printf("\033[1;32m%s\033[0m", RETR);
    r_size = new_recv(s_fd,buf,"RETR");
    total_length -= r_size;                             
    while(total_length > 0)                            // if total length of mail > buf's length;we should read more than one times to get the entire mail
    {
        r_size = new_recv(s_fd,buf,"RETR");
        total_length -= r_size;
    }

    // TODO: Send QUIT command and print server response
    const char* QUIT = "QUIT\r\n";
    send(s_fd,QUIT,strlen(QUIT),0);
    printf("\033[1;32m%s\033[0m", QUIT);
    new_recv(s_fd,buf,"quit");
    close(s_fd);
}

int main(int argc, char* argv[])
{
    recv_mail();
    exit(0);
}
