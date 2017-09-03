#include <stdio.h>     
#include <sys/types.h>     
#include <sys/socket.h>     
#include <netinet/in.h>     
#include <arpa/inet.h>   
#include <stdlib.h>  
#include <string.h>  
#include <sys/epoll.h>  
 
#define BUFFER_SIZE 40  
#define MAX_EVENTS 10  
typedef struct cliData
{
	int fd;
	int buf;
} CliData;
int tmp = 1;
int main(int argc, char * argv[])     
{  
    int server_sockfd;// ���������׽���     
    int client_sockfd;// �ͻ����׽���     
    int len;     
    struct sockaddr_in my_addr;   // �����������ַ�ṹ��     
    struct sockaddr_in remote_addr; // �ͻ��������ַ�ṹ��     
    int sin_size;     
    char buf[BUFFER_SIZE];  // ���ݴ��͵Ļ�����     
    memset(&my_addr,0,sizeof(my_addr)); // ���ݳ�ʼ��--����     
    my_addr.sin_family=AF_INET; // ����ΪIPͨ��     
    my_addr.sin_addr.s_addr=INADDR_ANY;// ������IP��ַ--�������ӵ����б��ص�ַ��     
    my_addr.sin_port=htons(8000); // �������˿ں�     
    // �������������׽���--IPv4Э�飬��������ͨ�ţ�TCPЭ��  
    if((server_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)     
    {       
        perror("socket");     
        return 1;     
    }     
    // ���׽��ְ󶨵��������������ַ��  
    if (bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)     
    {     
        perror("bind");     
        return 1;     
    }     
    // ������������--�������г���Ϊ5   
    listen(server_sockfd,5);     
    sin_size=sizeof(struct sockaddr_in);   
    // ����һ��epoll���  
    int epoll_fd;  
    epoll_fd=epoll_create(MAX_EVENTS);  
    if(epoll_fd==-1)  
    {  
        perror("epoll_create failed");  
        exit(EXIT_FAILURE);  
    }  
    struct epoll_event ev;// epoll�¼��ṹ��  
    struct epoll_event events[MAX_EVENTS];// �¼���������  
    ev.events=EPOLLIN;  
    ev.data.fd=server_sockfd;  
    // ��epollע��server_sockfd�����¼�  
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_sockfd,&ev)==-1)  
    {  
        perror("epll_ctl:server_sockfd register failed");  
        exit(EXIT_FAILURE);  
    }  
    int nfds;// epoll�����¼������ĸ���  
    // ѭ�����ܿͻ�������      
    while(1)  
    {  
        // �ȴ��¼�����  
        nfds=epoll_wait(epoll_fd,events,MAX_EVENTS,-1);  
        if(nfds==-1)  
        {  
            perror("start epoll_wait failed");  
            exit(EXIT_FAILURE);  
        }  
        int i;  
        for(i=0;i<nfds;i++)  
        {  
            // �ͻ������µ���������  
            if(events[i].data.fd==server_sockfd)  
            {  
                // �ȴ��ͻ����������󵽴�  
                if((client_sockfd=accept(server_sockfd,(struct sockaddr *)&remote_addr,&sin_size))<0)  
                {     
                    perror("accept client_sockfd failed");     
                    exit(EXIT_FAILURE);  
                }  
                // ��epollע��client_sockfd�����¼�  
                ev.events=EPOLLIN;
				CliData* lpData = (CliData*)malloc(sizeof(CliData));
				lpData->fd = client_sockfd;
				lpData->buf = tmp++;
                ev.data.ptr = lpData;			
                // ev.data.fd=client_sockfd;  
                if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_sockfd,&ev)==-1)  
                {  
                    perror("epoll_ctl:client_sockfd register failed");  
                    exit(EXIT_FAILURE);  
                }  
                printf("accept client %s\n",inet_ntoa(remote_addr.sin_addr)); 
			    fflush(stdout);
            }  
            // �ͻ��������ݷ��͹���  
            else  
            {
				memset(buf,0,sizeof(buf));
                len=recv(((CliData*)(events[i].data.ptr))->fd,buf,BUFFER_SIZE,0);  
                if(len<0)  
                {  
                    perror("receive from client failed");  
                    exit(EXIT_FAILURE);  
                } 
				if(len == 0)
				{
					printf("client close the connection\n");
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL,((CliData*)(events[i].data.ptr))->fd,&ev);
					close(((CliData*)(events[i].data.ptr))->fd);
					free(events[i].data.ptr);
					continue;
				}
                printf("receive from client %d :%s\n",((CliData*)(events[i].data.ptr))->buf,buf); 
				fflush(stdout);				
                send(((CliData*)(events[i].data.ptr))->fd,"I have received your message.",30,0);  
            }  
        }  
    }
	if(server_sockfd > 0)
	{
		close(server_sockfd);
	}
	if(epoll_fd > 0)
	{
		close(epoll_fd);
	}
    return 0;     
}