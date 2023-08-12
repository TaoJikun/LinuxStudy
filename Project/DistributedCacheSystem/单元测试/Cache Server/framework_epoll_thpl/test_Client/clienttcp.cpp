#include "iostream"
#include "unistd.h"
#include "cstring"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "json.hpp"
#include "fcntl.h"
#define SERV_IP "127.0.0.1"
#define SERV_PORT 8000
#define KEY_VALUE_REQUEST 0
#define KEY_VALUE_RESPOND 1
using json = nlohmann::json;
json KeyValuejs()
{
    json js, data;
    data["flag"] = true;
    data["key"] = "abcdefg";
    data["value"] = "hijklmnopq";
    js["type"] = KEY_VALUE_RESPOND;
    js["data"] = data;
    return js;
}
json KeyValuejsrequest()
{
    json js, data;
    data["flag"] = true;
    data["key"] = "abcdefg";
    data["value"] = "";
    js["type"] = KEY_VALUE_REQUEST;
    js["data"] = data;
    return js;
}
void Sleep_ms(int n){
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = n * 1000;
    select(0, NULL, NULL, NULL, &delay);
}
int main(int argc, char *argv[]){
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_len;
    char buf[BUFSIZ];
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(SERV_PORT);
    connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    json js = KeyValuejs();
    std::string buff = js.dump();
    int n;
    n = write(cfd, (char *)buff.data(), buff.length());
    if(n>0)
        std::cout<<js<<std::endl;
    Sleep_ms(100);
    js = KeyValuejsrequest();
    buff = js.dump();
    write(cfd, (char *)buff.data(), buff.length());
    n = read(cfd, buf, BUFSIZ);
    if(n>0)
        std::cout<<json::parse(buf)<<std::endl; 
    sleep(1);
    close(cfd);
    return 0;
}

