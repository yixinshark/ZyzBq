#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "packet.c"
#define N 1024
int main(int argc,char *argv[])
{
    int i=0;
	int listenfd,connectfd;
    unsigned char buf[N] ={0};
	unsigned int nbyte = 10;
	struct sockaddr_in s_addr,c_addr;
	socklen_t len = sizeof(c_addr);


	if(3 != argc)
	{
		puts("input argc error!");
		exit(-1);
	}
	
	bzero(&s_addr,sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(atoi(argv[2]));
	s_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	if (-1 == (listenfd = socket(AF_INET,SOCK_STREAM,0)))
	{
		perror("socket");
		exit(-1);
	}
	printf("listenfd = %d\n",listenfd);
	if(-1 == bind(listenfd,(struct sockaddr *)&s_addr,sizeof(s_addr)))
	{
		perror("bind");
		exit(-1);
	}

    listen(listenfd,5);
    while(1)
    {
		puts("===================================");
		if(-1 == (connectfd = accept(listenfd,(struct sockaddr *)&c_addr,&len)))
		{
			perror("accept");
			exit(-1);
		}
		printf("connectfd   = %d \n",connectfd);
		printf("c_addr ip   = %s \n",inet_ntoa(c_addr.sin_addr));
		printf("c_addr port = %d \n",ntohs(c_addr.sin_port));

		while(1)
		{	
	    	bzero(buf,N);
			nbyte = recv(connectfd,buf,N,0);
			if(nbyte  == -1)
			{
				perror("recv");
				exit(-1);
			}
			if(nbyte == 0)
			{
				break;
			}
            for(i=0;i<buf[0]+4;i++)
            {
                printf("%02x",buf[i]);
                printf(":");
            }
            putchar(10);

            if(*(buf+buf[0]+2)==0x40)
            {
                if(send(connectfd,peer0_1,peer0_1[0]+4,0)==-1)
                {
                    perror("send\n");
                    exit(-1);
                }
            }
/*
			bzero(buf,N);
			puts("plz input message:");
			   scanf("%[^\n]",buf);
			   getchar();
			fgets(buf,N,stdin);
			buf[strlen(buf)-1] = '\0';
			if(send(connectfd,buf,N,0) == -1)
			{
				perror("send");
				exit(-1);
			}
*/
		}
		puts("client is quit");
		close(connectfd);
	}
	close(listenfd);

	return 0;
}

