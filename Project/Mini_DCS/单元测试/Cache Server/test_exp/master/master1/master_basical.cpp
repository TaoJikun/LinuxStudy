//master的基础配置
#include "master_json.h"
using json = nlohmann::json;
void* masterhandler(void* arg1)
{
	LogFile* arg = (LogFile*)arg1;
    int openmax = 1000;
	int num=0;//client num
	char buf[BUFSIZ], str[openmax];
	struct sockaddr_in saddr,caddr;
	struct epoll_event tep, ep[openmax];
	master_handler master1;
	int listen_num = 10;
	//建立套接字//使用 IPv4 格式的 ip 地址,使用流式的传输协议,使用默认的协议
	int server_fd=Socket(AF_INET,SOCK_STREAM,0);
	// 设置端口复用
	int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	//绑定本机的IP和端口
	bzero(&saddr,sizeof(saddr));
	saddr.sin_family=AF_INET;/* 地址族协议: AF_INET */
	saddr.sin_port=htons(MASTER_PORT);//设置端口,2字节-> 大端//htons将一个短整形从主机字节序 -> 网络字节序
	saddr.sin_addr.s_addr=inet_addr(MASTER_IP); /* IP地址, 4字节 -> 大端  *///点分十进制-》换成32位的网络字节序二进制值(大端整形)
	int ret=Bind(server_fd,(struct sockaddr *)&saddr,sizeof(saddr));
	//设置监听套接字（设置同时来链接数）
	ret=Listen(server_fd,listen_num);		//server_fd设置为监听套接字
	// 所有的文件描述符对应读写缓冲区状态都是委托内核进行检测的epoll
	// 创建一个epoll模型
	int efd = Epoll_create(openmax);
	if(efd == -1)
    {
        perror("epoll_create");
        exit(0);
    }
	// 往epoll实例中添加需要检测的节点, 现在只有监听的文件描述符
    tep.events = EPOLLIN | EPOLLET;// 设置边沿模式
    tep.data.fd = server_fd;
    int res = Epoll_ctl(efd, EPOLL_CTL_ADD, server_fd, &tep);
	printf("等待客户端连接...\n");
	for(;;)
	{
		int nready = Epoll_wait(efd, ep, openmax, -1);
		//等待对端链接
		for(int i=0;i<nready;i++)
		{
            if(0>nready)
                continue;
			if(ep[i].data.fd==server_fd)//当前的文件描述符是不是用于监听的
			{
				socklen_t clilen = sizeof(caddr);
				int client_fd = Accept(server_fd,(struct sockaddr *)&caddr,&clilen);//建立新的连接
				//Accept()返回值：函数调用成功，得到一个文件描述符，用于和建立连接的这个客户端通信，调用失败返回 -1
				//inet_ntop将大端的整形数, 转换为小端的点分十进制的IP地址
				//ntohs将一个短整形从网络字节序 -> 主机字节序
				cout<<"received from "<<\
                inet_ntop(AF_INET, &caddr.sin_addr, str, sizeof(str))<<\
                " at PORT "<<ntohs(caddr.sin_port)<<endl;
                cout<<" cfd "<<client_fd<<"---client"<<++num<<endl;
                //显示对端的ip和端口
                printf("对端的ip为：%s\n",inet_ntoa(caddr.sin_addr));
                printf("对端的port为：%d\n",ntohs(caddr.sin_port));
				// 设置完成之后, 读写都变成了非阻塞模式
				auto flag = fcntl(client_fd, F_GETFL);
                fcntl(client_fd, F_SETFL, flag | O_NONBLOCK);
				// 新得到的文件描述符添加到epoll模型中, 下一轮循环的时候就可以被检测了
                tep.events = EPOLLIN | EPOLLOUT | EPOLLET;
                tep.data.fd = client_fd;
                res = Epoll_ctl(efd, EPOLL_CTL_ADD, client_fd, &tep);// 添加待检测节点到epoll实例中
				if(res == -1)
                {
                    perror("epoll_ctl-accept");
                    exit(0);
                }
			}
			else
			{
				//memset(buf, 0, sizeof(buf));
				//while(1)
				//{
				int sockfd = ep[i].data.fd;
				//buf[0] = '\0';
				/*如果连接没有断开，接收端接收不到数据，接收数据的函数会阻塞等待数据到达，
				数据到达后函数解除阻塞，开始接收数据，当发送端断开连接，接收端无法接收到
				任何数据，但是这时候就不会阻塞了，函数直接返回0。*/      
				//n,实际接收的字节数
				//buf: 指向一块有效内存，用于存储接收是数据        
				ssize_t n = unblock_read_net(sockfd, buf, sizeof(buf));
				if(0==n){//对方断开了连接
					res = Epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
					Close(sockfd);
					cout<<"client["<<sockfd<<"] closed connection"<<endl;
				}
				else if(0>n)
				{//接收数据失败了
					if(errno == EAGAIN)
					{
						printf("数据读完了...\n");
						break;
					}
					else
					{
						perror("recv");
						Close(sockfd);
						exit(0);
					}
				}
				else{
					printf("%s\n",buf);
					//json info = json::parse(buf);
					json info;
					if(json::accept(buf))
						info = json::parse(buf);
					//回应client的读数据请求
					if(info["type"]==DISTRIBUTION_REQUEST)
					{
						master1.Distributionresponse(sockfd);
					}
					//主动缩容,比如要关闭Cache A,这时候需要有个对象向master发送shutdown信息
					else if(info["type"]==SHUTDOWN_CACHE)
					{
						//响应CLEAR用户发送给master的SHUTDOWN指令，并将数据导入出来
						vector<string> userdata=info["data"]["iplist"];//127.0.0.1:8000,127.0.0.1:8001
						for(int i=0;i<userdata.size();i++)
						{
							int cut = userdata[i].find_last_of(':');
							shutipnow.push_back(userdata[i].substr(0,cut));
							shutportnow.push_back(userdata[i].substr(cut+1));
						}
						num4=shutportnow.size();
						flag2=1;
						flag5=1;
					}
					//主动扩容,处理心跳包数据，如果有新的IP，那么我就更新IPlist列表，然后向所有cache发送更新后的列表
					else if(info["type"]==HEART_BEAT)
					{
						//提取出IP和port
						string heartdata=info["data"]["iplist"];//127.0.0.1:8000
						bool heartstate = info["data"]["state"];
						string str1 = heartdata;
						int cut = str1.find_last_of(':');
						heartip=str1.substr(0,cut);
						heartport=str1.substr(cut+1);
						//接收到某个port的心跳包，就加1
						auto it = cachestatemap.find(heartport);
						if(it != cachestatemap.end())//find it
						{
							(it->second)++;  //接收心跳包的时间
						}
						//检测心跳包的state状态
						if(heartstate==false)
						{
							flag5=2;
						}
						if(flag5==1)//用户主动关闭某几个cache
						{
							//如果这个heartport在shutportnow里面，就设置shutip和shutport
							vector<string>::iterator iterr1 = find(shutportnow.begin(), shutportnow.end(), heartport); 
							if ( iterr1 != shutportnow.end()) //找到
							{
								shutport=*iterr1;
							}
							vector<string>::iterator iterr2 = find(shutipnow.begin(), shutipnow.end(), heartip); 
							if ( iterr2 != shutipnow.end()) //找到
							{
								shutip=*iterr2;
							}
						}
						else if(flag5==2)//检测心跳包的false状态，cache主动请求关闭自己；
						{
							shutport=heartport;
							shutip=heartip;
							flag2=1;
						}
						//查找发送心跳包的port是否出现过
						vector<int>::iterator result = find(Portlist.begin(), Portlist.end(), atoi(heartport.c_str())); 
						if ( result == Portlist.end()) //没找到,//更新IPlist表
						{
							//防重连
							if(num5<num4)
							{
								vector<string>::iterator iterrr = find(shutportnow.begin(), shutportnow.end(), heartport); 
								//if(flag4==1 && (shutport==heartport))
								if((iterrr != shutportnow.end()))
									flag4=1;
								num5++;
							}
							else
							{
								flag4=0;
								num5=0;
							}
							switch (flag4)
							{
							case 0:
								IPlist.push_back(heartip);
								Portlist.push_back(atoi(heartport.c_str()));
								IPportlist.push_back(str1);
								cachemap.emplace(heartport,0);
								cachemapshut.emplace(heartport,0);
								cachestatemap.emplace(heartport,0);
								masterupdatemap.emplace(heartport,0);
								flag1=1;//扩容标志位
								break;
							case 1:
								flag4=0;
								break;
							default:
								break;
							}
						}
						//向IPlist维护的所有的cache都发送最新的列表
						if(flag1==1)
						{
							vector<string>().swap(shutportnow);
							vector<string>().swap(shutipnow);
							arg->LogInfo("add new cache : ip = %s, port = %s",(char*)heartip.c_str(),(char*)heartport.c_str());
							auto it = cachemap.find(heartport);
							//else if(it != cachemap.end() && flag4==0)//find it
							if(it != cachemap.end())
							{
								//add yixia
								if(it->second == 0)
								{
									it->second=1;
									num1++;
									refreship(sockfd);
								}
							}
							if(num1 == cachemap.size())//全发过再flag=0;
							{
								flag1=0;
								num1=0;
								for(auto& v : cachemap)
									v.second=0;
							}	
						}
						//判断是否主动缩容,主动关闭的cache发来了心跳包
						if (shutport == heartport && flag2==1)
						{
							//删去维护的IP列表中即将关闭的IP、port和IP:Port列表\cachemap
							for(vector<string>::iterator iter=IPlist.begin();iter!=IPlist.end();iter++)//从vector中删除指定IP
							{        
								if(*iter==shutip){
									IPlist.erase(iter);
									break;
								}
							}
							for(vector<int>::iterator iter=Portlist.begin();iter!=Portlist.end();iter++)//从vector中删除指定IP
							{
								if(*iter==atoi(shutport.c_str()))
								{
									Portlist.erase(iter);
									break;
								}
							}
							for(vector<string>::iterator iter=IPportlist.begin();iter!=IPportlist.end();iter++)//从vector中删除指定IP
							{        
								if(*iter==str1){
									IPportlist.erase(iter);
									break;
								}
							}
							auto iter = cachemap.find(heartport);
							if(iter != cachemap.end())//find it
							{
								cachemap.erase(iter);
							}
							auto iter2 = cachemapshut.find(heartport);
							if(iter2 != cachemapshut.end())//find it
							{
								cachemapshut.erase(iter2);
							}
							auto iter3 = cachestatemap.find(heartport);
							if(iter3 != cachestatemap.end())//find it
							{
								cachestatemap.erase(iter3);
							}
							auto iter4 = precachestatemap.find(heartport);
							if(iter4 != precachestatemap.end())//find it
							{
								precachestatemap.erase(iter4);
							}
							auto iter5 = masterupdatemap.find(heartport);
							if(iter5 != masterupdatemap.end())//find it
							{
								masterupdatemap.erase(iter5);
							}
							//向待关闭的cache发送SHUTSDOWN指令
							refreship(sockfd);
							arg->LogInfo("delete normal cache : ip = %s, port = %s",(char*)heartip.c_str(),(char*)shutport.c_str());
							num3++;//统计是否向所有要关闭的port都发送了
							flag8=1;
							if(((flag5==1) || (flag5==2)) && (num3==num4) && flag8==1)
							{
								num3=0;//发送置0，等待下次发送
								flag2=0;//缩容完毕,不再向待关闭的cache发送SHUTSDOWN指令
								flag3=1;//向待关闭的cache发送SHUTSDOWN指令后再向其他cache发送refresh_ip指令
								flag4=1;//防重连
								flag5=0;//不再设置shutip和shutport
								flag8=0;//
							}
						}
						else if(flag6==1)
						{
							//删去维护的IP列表中即将关闭的IP、port和IP:Port列表\cachemap
							for(vector<string>::iterator iter=IPlist.begin();iter!=IPlist.end();iter++)//从vector中删除指定IP
							{        
								if(*iter==psshutip){
									IPlist.erase(iter);
									break;
								}
							}
							for(vector<int>::iterator iter=Portlist.begin();iter!=Portlist.end();iter++)//从vector中删除指定IP
							{
								if(*iter==atoi(psshutport.c_str()))
								{
									Portlist.erase(iter);
									break;
								}
							}
							string str2=psshutip+":"+psshutport;
							for(vector<string>::iterator iter=IPportlist.begin();iter!=IPportlist.end();iter++)//从vector中删除指定IP
							{        
								if(*iter==str2){
									IPportlist.erase(iter);
									break;
								}
							}
							auto iter = cachemap.find(psshutport);
							if(iter != cachemap.end())//find it
							{
								cachemap.erase(iter);
							}
							auto iter2 = cachemapshut.find(psshutport);
							if(iter2 != cachemapshut.end())//find it
							{
								cachemapshut.erase(iter2);
							}
							auto iter3 = cachestatemap.find(psshutport);
							if(iter3 != cachestatemap.end())//find it
							{
								cachestatemap.erase(iter3);
							}
							auto iter4 = precachestatemap.find(psshutport);
							if(iter4 != precachestatemap.end())//find it
							{
								precachestatemap.erase(iter4);
							}
							auto iter5 = masterupdatemap.find(heartport);
							if(iter5 != masterupdatemap.end())//find it
							{
								masterupdatemap.erase(iter5);
							}
							arg->LogInfo("delete unhealthy cache : ip = %s, port = %s",(char*)heartip.c_str(),(char*)psshutport.c_str());
							flag3=1;//向待关闭的cache发送SHUTSDOWN指令后再向其他cache发送refresh_ip指令
							flag4=1;//防重连
							num4=100;//一次关闭一个,只可能出现一个重连的心跳包
							flag8=0;
						}
						if(flag3==1 && flag8==0)
						{
							auto it = cachemapshut.find(heartport);
							if(it != cachemapshut.end())//find it
							{
								if(it->second == 0)
								{
									it->second=1;
									num2++;
									refreship(sockfd);
								}
							}
							if(num2 == cachemapshut.size())//全发过再flag=0;
							{
								flag3=0;
								flag6=0;
								num2=0;
								for(auto& v : cachemapshut)
									v.second=0;
							}
						}
						/*if(flag7==1)//此时需要向其他cache发送新的master信息
						{
							auto it = masterupdatemap.find(heartport);
							if(it != masterupdatemap.end())
							{
								//add yixia
								if(it->second == 0)
								{
									it->second=1;
									num6++;
									refreshmaster(sockfd);
								}
							}
							if(num6 == masterupdatemap.size())//全发过再flag=0;
							{
								flag7=0;
								num6=0;
								for(auto& v : masterupdatemap)
									v.second=0;
							}
						}*/
					}
					else if (info["type"]==SPARE_MASTER)
					{
						string masterdata=info["data"]["IP"];//127.0.0.1:8000
						vector<string> vec=info["data"]["iplist"];
						if(vec.size()>0)
							IPportlist.assign(vec.begin(), vec.end());
						//masterip=masterdata.substr(0,9);
						//masterport=masterdata.substr(10,4);
						int cut = masterdata.find_last_of(':');
						masterip = masterdata.substr(0,cut);
						masterport = masterdata.substr(cut+1);
						//接收到某个port的心跳包，就加1
						auto it = masterstatemap.find(masterport);
						if(it != masterstatemap.end())//find it
						{
							
							(it->second)++;
						}
						else
						{
							masterstatemap.emplace(masterport,0);
						}
					}
				}
			}
		}
	}
    Close(server_fd);
}
int main(void)
{
	LogFile logfile;
    logfile.OpenLogFile("../master1.log");
	//pthread_t producer_thread_id;
	//pthread_create(&producer_thread_id,NULL,producer_thread_proc,&logfile);
	//master处理程序
	pthread_t tid_master;
    Pthread_create(&tid_master, NULL, masterhandler, &logfile);
	//检测cache心跳包状态
	pthread_t tid_heartbeat;
    Pthread_create(&tid_heartbeat, NULL, heartstate, &logfile);
	//发送心跳包
	pthread_t tid_masterheartbeat;
    Pthread_create(&tid_masterheartbeat, NULL, masterupdate, &logfile);
	//检测是否所有心跳包都正常
	pthread_t tid_allcachehealthy;
    Pthread_create(&tid_allcachehealthy, NULL, testallshut, &logfile);
	pthread_t log_consumer_thread_id;
    pthread_create(&log_consumer_thread_id,NULL,LogFile::LogFileConsumer,&logfile);
	
	pthread_join(log_consumer_thread_id,NULL);
	return 0;
}
