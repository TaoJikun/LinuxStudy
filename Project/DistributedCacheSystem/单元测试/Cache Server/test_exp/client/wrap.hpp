#ifndef WRAP_HPP
#define WRAP_HPP
#include <iostream>
#include "unistd.h"
#include "arpa/inet.h"

struct OneIPList
{
public:
    std::string ip;
    int port;
};

void Sleep_ms(int n){
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = n * 1000;
    select(0, NULL, NULL, NULL, &delay);
}

void PerrExit(const char *s)
{
    perror(s);
    exit(-1);
}
int Socket(int family, int type, int protocol)
{
    int n;
    if ((n = socket(family, type, protocol)) < 0)
        PerrExit("socket error");
    return n;
}
int Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
    int n;
    n = connect(fd, sa, salen);
    if (n < 0)
        PerrExit("connect error");
    return n;
}
int Close(int fd)
{
    int n;
    if ((n = close(fd)) < 0)
        PerrExit("close error");
    return n;
}
//默认重读三次，读不到则error
ssize_t Read(int fd, char *ptr, size_t nbyte)
{
    ssize_t n;
    int j = 0;
    do{
        Sleep_ms(100);
        do{
            n = read(fd, ptr, nbyte);
            if (n >= 0)
                return n;
        } while (errno == EINTR);
        j++;
        if(j > 2)
            break;
    }while(n <= 0);
    PerrExit("read error");
    return -1;
}
ssize_t Write(int fd, const void* ptr, size_t nbytes){
    ssize_t n;
    do{
        n = write(fd, ptr, nbytes);
        if(n>=0)
            return n;
    }while(errno==EINTR);
    return -1;
}

OneIPList GetIPList(std::string cacheip){
    int cut = cacheip.find_last_of(':');
    OneIPList iptest;
    iptest.ip = cacheip.substr(0,cut);
    iptest.port = std::stoi(cacheip.substr(cut+1));
    //std::cout << iptest.ip << " " << iptest.port << std::endl;
    return iptest;
}

std::string ReadTxt(std::string filename)
{
	int line;
    //line行数限制 1 - lines
	std::ifstream text;
	text.open(filename, std::ios::in);
    if(!text.is_open ())
    {
        std::cout << "Open file failure" << std::endl;
    }
	std::vector<std::string> strVec;
	while (!text.eof())  //行0 - 行lines对应strvect[0] - strvect[lines]
	{
		std::string inbuf;
		std::getline(text, inbuf, '\n');
		strVec.push_back(inbuf);
	}
    std::cout << "keylist总共有" << strVec.size()-1 << "行，读取第几行数据？" << std::endl;
    std::cin >> line;
    return strVec[line];
}

std::string Randstr(const int len)
{
    std::string ans;
    int i;
    for (i = 0; i < len; i++)
    {
        char c;
        switch ((rand() % 3))
        {
        case 1:
            c = 'A' + rand() % 26;
            break;
        case 2:
            c = 'a' + rand() % 26;
            break;
        default:
            c = '0' + rand() % 10;
        }
        ans += c;
    }
    ans[++i] = '\0';
    return ans;
}

#endif /* WRAP_HPP */
