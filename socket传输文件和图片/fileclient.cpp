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
 
void FileData(const int sockfd)
{
    FILE *fq;
    int len = 0;
    const int MAXLINE = 4096;//定义buferr最大存储
    char  buffer[MAXLINE];

    char filePath[1000];
    printf("请输入要传输的文件路径：\n");
    gets(filePath);

    //获取文件名称 及所占字符长度
    char* tmp = filePath;
    while (*(tmp++) != '\0');
    int fileNameLen = 0;
    while (*(--tmp) != '/')//以 / 分割
        fileNameLen++;

    //发送文件名
    char str[50];
    memcpy(str,tmp,fileNameLen);
    if(send(sockfd ,str,sizeof(str),0) == -1)
    {
        printf("发送文件名失败\n");
        return;
    }

    //判断是否接收文件
    char  szResult[50] = {0};
    recv(sockfd,szResult,sizeof(szResult),0);
    if(0 != strcmp(szResult,"yes"))
    {
        printf("不接受文件\n");
        return;
    }  
    
    //发送文件内容 并按行读取
    if( ( fq = fopen(filePath,"rb") ) == NULL )
    {
        printf("File open error.\n");
        close(sockfd);
        exit(1);
    }

    bzero(buffer,sizeof(buffer));
    while(!feof(fq))
    {
        len = fread(buffer, 1, sizeof(buffer), fq);
        if(len != write(sockfd, buffer, len))
        {
            printf("write.\n");
            break;
        }
    }
    fclose(fq);
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
    FileData(socketfd);

    // 关闭socket，释放资源。
    close(socketfd);
    return 0;
}