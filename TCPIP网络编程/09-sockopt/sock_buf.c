#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

void error_handling(char *message);

int main(int argc, char *argv[]){
    int tcp_sock,udp_sock;
    socklen_t buflen;
    int state,rcv_buf,snd_buf;

    buflen = sizeof(snd_buf);
    tcp_sock = socket(PF_INET,SOCK_STREAM,0);
    udp_sock = socket(PF_INET,SOCK_DGRAM,0);

    state = getsockopt(tcp_sock,SOL_SOCKET,SO_SNDBUF,(void*)&snd_buf,&buflen);
    if(state){
        error_handling("getsockopt error!\n");
    }
    printf("TCP Socket SndBuf: %d \n",snd_buf);

    state = getsockopt(tcp_sock,SOL_SOCKET,SO_RCVBUF,(void*)&rcv_buf,&buflen);
    if(state){
        error_handling("getsockopt error!\n");
    }
    printf("TCP Socket RcvBuf: %d \n",rcv_buf);


    state = getsockopt(udp_sock,SOL_SOCKET,SO_SNDBUF,(void*)&snd_buf,&buflen);
    if(state){
        error_handling("getsockopt error!\n");
    }
    printf("UDP Socket SndBuf: %d \n",snd_buf);


    state = getsockopt(udp_sock,SOL_SOCKET,SO_RCVBUF,(void*)&rcv_buf,&buflen);
    if(state){
        error_handling("getsockopt error!\n");
    }
    printf("UDP Socket RcvBuf: %d \n",rcv_buf);

    

    return 0;
}


void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}
