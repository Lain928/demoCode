/*
文件和图片的传输
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int InitSocket(const char* post,const char* addr = NULL)//默认使用任意地址
{
    // 第1步：创建服务端的socket。
    int listenfd;
    if ( (listenfd = socket(AF_INET,SOCK_STREAM,0))==-1) 
    { 
        printf("creat socket error!"); 
        return -1; 
    }

    // 第2步：把服务端用于通信的地址和端口绑定到socket上。
    struct sockaddr_in servaddr;    // 服务端地址信息的数据结构。
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;  // 协议族，在socket编程中只能是AF_INET。
    servaddr.sin_port = htons(atoi(post));  // 指定通信端口。

    //默认使用任意地址
    if(addr == NULL)
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);          // 任意ip地址。
    else
        servaddr.sin_addr.s_addr = inet_addr(addr); // 指定ip地址。
    //将socket和端口号绑定
    if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) != 0 )
    { 
        printf("bind error");
        close(listenfd); 
        return -1; 
    }

    // 第3步：把socket设置为监听模式。
    if (listen(listenfd,5) != 0 ) 
    {
        printf("listen error!");
        close(listenfd); 
        return -1; 
    }
    return listenfd;
}

int WaitAccept(const int listenfd)
{
    // 第4步：接受客户端的连接。
    int  clientfd;                  // 客户端的socket。
    int  socklen=sizeof(struct sockaddr_in); // struct sockaddr_in的大小
    struct sockaddr_in clientaddr;  // 客户端的地址信息。
    clientfd=accept(listenfd,(struct sockaddr *)&clientaddr,(socklen_t*)&socklen);
    printf("客户端（%s）已连接。\n",inet_ntoa(clientaddr.sin_addr));
    return clientfd;
}


void FileData(const int connfd)
{
    char ssbuff[1024] = {0};//存储文件名
    const int MAX_SIZE = 1000;
    int nlen;
    if(recv(connfd,ssbuff,sizeof(ssbuff),0) == -1)
    {
        printf("接收文件名失败\n");
        return;
    }
    printf("是否要接受文件\n");
    char str[10];
    gets(str);
    if(send(connfd,str,sizeof(str),0) == -1)
    {
        printf("发送是否接收文件答复失败\n");
        return;
    }

    //设置存储位置
    char szPath[MAX_SIZE] = {0};
    printf("请输入要存储的路径：\n");
    gets(szPath);
    //拼接路径和文件名
    char szPathName[MAX_SIZE] = {0};
    sprintf(szPathName,"%s/%s",szPath,ssbuff);//拼接路径和文件名
    printf("%s\n",szPathName);

    //打开文件写入
    FILE* fp;
    const int MAXLINE = 4096;
    char buff[MAXLINE];
    int n = 0;

    if((fp = fopen(szPathName,"ab") ) == NULL )
    {
        printf("File open error.\n");
        return;
    }

    while(1)
    {
        n = read(connfd, buff, MAXLINE);
        if(n == 0)
            break;
        fwrite(buff, 1, n, fp);
    }
    buff[n] = '\0';
    fclose(fp);
}
 
int main(int argc,char *argv[])
{
    if (argc!=2)
    {
        printf("Using:./server port\nExample:./server 5005\n\n"); return -1;
    }

    //初始化socket 并获取listenfd
    int listenfd = InitSocket(argv[1]);
    if(listenfd == -1)
    {
        printf("initsocket error");
        return -1;
    }
    //等待连接
    int clientfd = WaitAccept(listenfd);

    //数据传输
    //DataInteraction(clientfd);
    FileData(clientfd);

    // 第6步：关闭socket，释放资源。
    close(listenfd); 
    close(clientfd); 
}