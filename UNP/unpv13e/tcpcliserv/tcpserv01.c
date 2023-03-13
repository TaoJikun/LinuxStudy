#include	"unp.h"

int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	/*
	 *正常情况下服务器终止后会有一个time_waite状态，
	 *然后马上重启服务器进程时，会bind失败（因为还处于time_wait状态,熟知端口号尚被占用），
	 *设置SO_RESUEADDR套接字选项后，允许启动一个监听服务器并捆绑其众所周知端口，
	 *即使以前建立的将该端口用作它们的本地端口的连接仍存在
	 *当然这只是其中一个作用，详见UNP165页
	 */
	int option;
	socklen_t optlen;
	optlen=sizeof(option);
	option=1;
	Setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(void*)&option,optlen);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);

		if ( (childpid = Fork()) == 0) {	/* child process */
			Close(listenfd);	/* close listening socket */
			str_echo(connfd);	/* process the request */
			exit(0);
		}
		Close(connfd);			/* parent closes connected socket */
	}
}
