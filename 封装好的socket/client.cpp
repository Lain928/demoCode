#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int InitSocket(const char* addr, const char* post)
{
    // 第1步：创建客户端的socket。
    int sockfd;
    if ( (sockfd = socket(AF_INET,SOCK_STREAM,0))==-1) 
    { 
        perror("socket"); 
        return -1; 
    }
    return sockfd;
}

int MyConnect(const int sockfd, const char* addr,const char* post)
{
    // 第2步：向服务器发起连接请求。
    struct hostent* h;
    if ( (h = gethostbyname(addr)) == 0 )   // 指定服务端的ip地址。
    { 
        printf("gethostbyname failed.\n"); 
        close(sockfd); 
        return -1; 
    }
    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(post)); // 指定服务端的通信端口。
    memcpy(&servaddr.sin_addr,h->h_addr,h->h_length);
    if (connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)  // 向服务端发起连接清求。
    { 
        perror("connect"); 
        close(sockfd); 
        return -1; 
    }
    return 0;
}
void DataInteraction(const int sockfd)
{
    char buffer[1024];
    //发送
    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,"这是来自客户端的信息%d",sockfd);
    int iret;
    if ( (iret=send(sockfd,buffer,strlen(buffer),0))<=0) // 向服务端发送请求报文。
    { 
        perror("send");
        return;
    }
    printf("发送：%s\n",buffer);

    //接收
    memset(buffer,0,sizeof(buffer));
    if ( (iret=recv(sockfd,buffer,sizeof(buffer),0))<=0) // 接收服务端的回应报文。
    {
        printf("iret=%d\n",iret);
    }
    printf("接收：%s\n",buffer);
}
 
int main(int argc,char *argv[])
{
    if (argc!=3)
    {
        printf("Using:./client ip port\nExample:./client 127.0.0.1 5005\n\n"); return -1;
    }
    //创建socket
    int socketfd = InitSocket(argv[1],argv[2]);
    if(socketfd == -1)
        return -1;

    //连接服务器
    if(MyConnect(socketfd, argv[1],argv[2]) != 0)
    {
        printf("connect error");
        return -1;
    }
    //数据传输
    DataInteraction(socketfd);

    // 关闭socket，释放资源。
    close(socketfd);
    return 0;
}
