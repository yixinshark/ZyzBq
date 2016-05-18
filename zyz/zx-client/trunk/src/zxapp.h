/*************************************************************************
	> File Name: zxapp.h
	> Author: 
	> Mail: 
	> Created Time: 2015年02月06日 星期五 10时02分08秒
 ************************************************************************/

#ifndef _ZXAPP_H
#define _ZXAPP_H

#include <tea/tea.h>
#include <unistd.h>
#include "packet1.c"
#define N 1024

typedef struct 
{
        int clientfd;
        struct pollfd pfd;
        int send_ctrl1;
        int send_ctrl2;
        int connect_ctrl; 

        char ip[16];
        unsigned int tcp_port;
        unsigned int username;

}session_t;

__BEGIN_DECLS

static int send_data(unsigned char buf[], size_t len, session_t *session);
static int recv_data(const void *buffer, int count, session_t *session);
static int handle_data(unsigned char buf[], int len, struct N_node* nn);
static int done_first(session_t *session);

__END_DECLS

#endif
