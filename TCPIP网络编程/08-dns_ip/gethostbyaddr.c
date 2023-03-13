#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

void error_handling(char *message);

int main(int argc, char *argv[]){
    struct hostent *host;
    struct sockaddr_in addr;
    if(argc!=2){
        printf("Usage: %s <IP>\n", argv[0]);
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    //addr.sin_addr.s_addr = inet_addr(argv[1]);
    inet_aton(argv[1],&addr.sin_addr);
    host = gethostbyaddr((char*)&addr.sin_addr,4,AF_INET);
    if (!host)
    {
        error_handling("Gethost ... error!");
    }

    printf("Offical name: %s \n",host->h_name);

    for (int i = 0; host->h_aliases[i]; i++)
    {
        /* code */
        printf("Aliases %d: %s \n",i+1,host->h_aliases[i]);
    }

    printf("Adress type is: %s \n",(host->h_addrtype==AF_INET?"AF_INET":"AF_INET6"));
    
    for (int i = 0; host->h_addr_list[i]; i++)
    {
        /* code */
        printf("IP Adress %d: %s \n",i+1,inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
    }

    return 0;

}


void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}
