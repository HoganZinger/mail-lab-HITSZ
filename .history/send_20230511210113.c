#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"

#define MAX_SIZE 4095

char buf[MAX_SIZE+1];

#define swap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF))

void print_msg(int send_fd, char* buf, char* err_message)
{
    int r_size = -1;
    if((r_size = recv(send_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror(err_message);
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);
}

char *read_file(char* path)
{
    FILE* fp = fopen(path,"r");
    fseek(fp,0,SEEK_END);
    int fileLen = ftell(fp);
	char *tmp = (char *) malloc(sizeof(char) * fileLen);
	fseek(fp, 0, SEEK_SET);
	fread(tmp, sizeof(char), fileLen, fp);
    fclose(fp);
    return tmp;
}

// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = "smtp.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = "3550517913"; // TODO: Specify the user
    const char* pass = "664936524zjh"; // TODO: Specify the password
    const char* from = "3550517913@qq.com"; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
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

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
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

    // Send EHLO command and print server response
    const char* EHLO = "EHLO qq.com\r\n"; // TODO: Enter EHLO command here
    send(s_fd, EHLO, strlen(EHLO), 0);
    printf("\033[1;32m%s\033[0m", EHLO);

    // TODO: Print server response to EHLO command
    print_msg(s_fd,buf,"recv ehlo");

    // TODO: Authentication. Server response should be printed out.
    const char* AUTH = "AUTH login\r\n";
    send(s_fd, AUTH, strlen(AUTH), 0);
    printf("\033[1;32m%s\033[0m", AUTH);
    new_recv(s_fd,buf,"recv auth");

    char* user_name = encode_str(user);
    strcat(user_name,"\r\n");
    send(s_fd, user_name, strlen(user_name), 0);
    printf("\033[1;32m%s\033[0m", user_name);
    new_recv(s_fd,buf,"recv username");
    free(user_name);

    char* password = encode_str(pass);
    strcat(password,"\r\n");
    send(s_fd, password, strlen(password), 0);
    printf("\033[1;32m%s\033[0m", password);
    new_recv(s_fd,buf,"recv password");
    if(strstr(buf,"Authentication successful") == NULL)
    {
        perror("auth");
        exit(EXIT_FAILURE);
    }
    free(password);

    // TODO: Send MAIL FROM command and print server response
    sprintf(buf,"MAIL FROM:<%s>\r\n",from);
    send(s_fd,buf,strlen(buf),0);
    printf("\033[1;32m%s\033[0m", buf);
    new_recv(s_fd,buf,"mail from");

    // TODO: Send RCPT TO command and print server response
    sprintf(buf,"RCPT TO:<%s>\r\n",receiver);
    send(s_fd,buf,strlen(buf),0);
    printf("\033[1;32m%s\033[0m", buf);
    new_recv(s_fd,buf,"rcpt to");

    // TODO: Send DATA command and print server response
    const char* DATA = "DATA\r\n";
    send(s_fd,DATA,strlen(DATA),0);
    printf("\033[1;32m%s\033[0m", DATA);
    new_recv(s_fd,buf,"data");

    // TODO: Send message data
    sprintf(buf,"From: %s\r\nTo: %s\r\nMIME-Version: 1.0\r\nContent-Type: multipart/mixed; boundary=qwertyuiopasdfghjklzxcvbnm\r\n",from,receiver);
    send(s_fd,buf,strlen(buf),0);
    printf("\033[1;32m%s\033[0m", buf);
    if(subject != NULL)
    {
        sprintf(buf,"Subject: %s\r\n\r\n",subject);
        send(s_fd,buf,strlen(buf),0);
        printf("\033[1;32m%s\033[0m", buf);
    } 
    if(msg != NULL)
    {
        const char* msg_head = "--qwertyuiopasdfghjklzxcvbnm\r\nContent-Type: text/plain\r\n\r\n";
        send(s_fd,msg_head,strlen(msg_head),0);
        if(access(msg,F_OK) == 0)
        {
            char* msg_content = read_file(msg);
            send(s_fd,msg_content,strlen(msg_content),0);
            printf("\033[1;32m%s\033[0m", msg_content);
        }
        else
        {
            send(s_fd,msg,strlen(msg),0);
            printf("\033[1;32m%s\033[0m", msg);
        }
        const char* msg_end = "\r\n";
        send(s_fd,msg_end,strlen(msg_end),0);
        printf("\033[1;32m%s\033[0m", msg_end);
    }
    if(att_path != NULL)
    {
        sprintf(buf,"--qwertyuiopasdfghjklzxcvbnm\r\nContent-Type: application/octet-stream\r\nContent-Transfer-Encoding: Base64\r\nContent-Disposition: attachment;filename=%s\r\n\r\n",att_path);
        send(s_fd,buf,strlen(buf),0);
        printf("\033[1;32m%s\033[0m", buf);
        FILE* fp = fopen(att_path,"r");
        if(fp == NULL)
        {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        FILE* fp64 = fopen("tmp.zip","w");
        encode_file(fp,fp64);
        fclose(fp);
        fclose(fp64);
        char* file_content = read_file("tmp.zip");
        send(s_fd,file_content,strlen(file_content),0);
        free(file_content);
        const char* send_end = "--qwertyuiopasdfghjklzxcvbnm\r\n";
        send(s_fd,send_end,strlen(send_end),0);
        printf("\033[1;32m%s\033[0m", send_end);
    }

    // TODO: Message ends with a single period
    send(s_fd,end_msg,strlen(end_msg),0);
    printf("\033[1;32m%s\033[0m", end_msg);
    new_recv(s_fd,buf,"msg");

    // TODO: Send QUIT command and print server response
    const char* QUIT = "quit\r\n";
    send(s_fd,QUIT,strlen(QUIT),0);
    printf("\033[1;32m%s\033[0m", QUIT);
    new_recv(s_fd,buf,"quit");
    close(s_fd);
    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}
