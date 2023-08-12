// Copyright 2022 icf
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CLIENT_
#define CLIENT_

#include "json.h"
#include "socketTool.h"
#include "nodeConHash.h"
#include "signal.h"


#define KEY_VALUE_REQUEST 0         //client向cache请求数据
#define KEY_VALUE_RESPOND 1         //set

#define DISTRIBUTION_REQUEST 0      //client向master拉取节点分布

#define DISTRIBUTION_RESPOND 1      //解析返回的数据需要这样子选择

#define TIME_OUT 5                  //超时时间

#define MASTER_IP               "127.0.0.1:7000"
#define MASTER_IP_BACKUP        "127.0.0.1:7001"
#define KEY_FILE_NAME           "keydata.txt"

using json = nlohmann::json;


enum MACHINE_TYPE{
    CACHE_SERVER,
    CLIENT,
    MASTER,
};

class Client{
    private:
        //一致性哈希
        NodeConHash hash;
        //生成json
        json WriteJs(int type);
        //根据key生成,用作像缓存服务器请求
        json WriteJs(string key);
        //连接远程的服务器
        int ConnectServ(ipport_pair oneip,int& cfd);
        //讲数据写入文件描述符
        int WriteData(int& cfd,json js);
        //从文件描述符中读取数据
        json ReadData(int& cfd);
        //请求分布
        void DistributionRequest();
        //判断是否超时
        void CountTime();
        //写文件描述符、请求描述符号、和master连接的文件描述符
        int kvwritecfd,kvreqcfd,mastercfd;

        struct sockaddr_in serv_addr;
        socklen_t serv_addr_len;

        //用来计时的事件
        time_t start,end;
        struct timeval timeout={3,0};
    public:
        Client();
        Client(vector<string> iplist);
        ~Client(){};
        //
        vector<string> alliplist;
        //set
        void keyValueWrite();
        //get
        string keyValueRequest(const string key);
};

#endif // DEBUG
