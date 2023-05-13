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

void new_recv(int send_fd, char* buf, char* err_message)
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
    
    // TODO: Authentication. Server response should be printed out.

    // TODO: Send MAIL FROM command and print server response
    
    // TODO: Send RCPT TO command and print server response
    
    // TODO: Send DATA command and print server response

    // TODO: Send message data
    
    // TODO: Message ends with a single period

    // TODO: Send QUIT command and print server response

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
