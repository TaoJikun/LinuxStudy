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

#include "client.h"
using namespace std;

Client::Client(){
    DistributionRequest();
}

Client::Client(vector<string> iplist){
    alliplist.swap(iplist);
    //

    hash.RefreshIpList(alliplist);

    time(&start);
}


json Client::WriteJs(int type){
    json js,data;
    if(type==KEY_VALUE_RESPOND)
    {
        data["key"]=RandStr(20);
        data["value"]=RandStr(20);
        data["flag"]=true;
    }
    else if(type==DISTRIBUTION_REQUEST){
        data=json::object();
    }
    js["machineType"] = CLIENT;
    js["req_type"]=type;
    js["data"]=data;
    return js;
}

//返回请求key的value的json变量
json Client::WriteJs(string key)
{
    json data;
    data["flag"]=true;
    data["key"]=key;
    data["value"]="";

    json js;
    js["machineType"] = CACHE_SERVER;
    js["req_type"]=KEY_VALUE_REQUEST;
    js["data"]=data;

    return js;
}

int Client::ConnectServ(ipport_pair oneip,int &cfd){
    //
    int tryCount = 0;
    cfd = Socket(AF_INET,SOCK_STREAM,0);
    int ret  = setsockopt(cfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET,(oneip.ip.c_str()),&serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(oneip.port);

    //连接成功
    int n = connect(cfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    if(n<0){
        do{
            Close(cfd);
            cfd = Socket(AF_INET,SOCK_STREAM,0);
            ret = setsockopt(cfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
            n = connect(cfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
            sleep(1);
            if(n>0){
                break;
            }
            tryCount++;
        }while(tryCount<=3);
    }
    
    return n;
}

//向cfd写入json文件
int Client::WriteData(int& cfd,json js){
    int nwrite;
    int j = 0;
    string buf = js.dump();
    buf+='\0';
    nwrite = Write(cfd,(char*)buf.data(),buf.length()+1);
    return nwrite;
}

json Client::ReadData(int &cfd){
    char bufin[BUFSIZ];
    int j = 0;
    Sleep_ms(100);
    int n = Read(cfd,bufin,BUFSIZ);
    if(n<=0){
        return json::object();
    }
    if(!json::accept(bufin)){
        perr_exit("no json data");
    }
    json js = json::parse(bufin);
    return js;
}

//拉取分布
void Client::DistributionRequest(){
    //解析master_ip
    ipport_pair masterip = get_ipport(MASTER_IP);
    //拉取分布式节点
    json client_to_master = WriteJs(DISTRIBUTION_REQUEST);

    int n,nwrite;
    do{
        n = ConnectServ(masterip,mastercfd);
        //如果没有连接成功，关闭文件描述符，连接备份服务器
        if(n!=0){
            Close(mastercfd);
            masterip = get_ipport(MASTER_IP_BACKUP);
            n = ConnectServ(masterip,mastercfd);
            if(n!=0){
                perr_exit("connction to master faileds");
            }
        }
        nwrite = WriteData(mastercfd,client_to_master);
    }while(nwrite<0);

    json js = ReadData(mastercfd);
    string s = js.dump();
    if(js["req_type"]!=DISTRIBUTION_RESPOND){
        perr_exit("distribution request returns fault");
    }
    //vector<string> ["","",""]
    alliplist=js["data"]["iplist"].get<vector<string>>();
    //如果是空的,那么
    if(alliplist.empty()){
        perr_exit("invalid distribution");
    }
    //刷新已经有的的列表
    hash.RefreshIpList(alliplist);
    time(&start);
    std::cout<<"distribution request once"<<std::endl;
    close(mastercfd);
}

//超时是否重载
void Client::CountTime(){
    time(&end);
    if(end<start){
        perr_exit("time out error");
    }
    time_t interval = end-start;
    bool _isOut = (interval>TIME_OUT)?true:false;
    if(_isOut){
        DistributionRequest();
    }
}

void Client::keyValueWrite(){
    signal(SIGPIPE,SIG_IGN);
    ofstream outfile;
    outfile.open(KEY_FILE_NAME,ios::app);
    if(!outfile.is_open()){
        perr_exit("open file failed");
    }

    //随机生成了key_Value
    json js = WriteJs(KEY_VALUE_RESPOND);
    string key   = js["data"]["key"].get<string>();
    string value = js["data"]["value"].get<string>();
    //判读是否需要重新拉取分布
    CountTime();
    //hash到应该连接的服务器
    ipport_pair oneipbf=get_ipport(hash.GetServerIndex(key));
    //js写入到kvWritecfd文件描述符中
    int nWrite = WriteData(kvwritecfd,js);
    if(nWrite<0){
        //将port设置为0，一定能够保证下一次的和这一次需要发送的服务器不一致，因此
        oneipbf.port = 0;
    }
    //输出到文件描述符中!!!
    outfile<<key<<" "<<value<<" "<<oneipbf.port<<endl;

    while(1){
        js = WriteJs(KEY_VALUE_RESPOND);
        key = js["data"]["key"].get<string>();
        value = js["data"]["value"].get<string>();
        CountTime();
        ipport_pair oneip = get_ipport(hash.GetServerIndex(key));
        //每次发送前
        if(oneip.ip!=oneipbf.ip||oneip.port!=oneipbf.port){
            close(kvwritecfd);
            int nWrite = ConnectServ(oneip,kvwritecfd);
            //一直都在拉取分布，直到成功
            while(nWrite<0){
                Close(kvwritecfd);
                sleep(1);
                DistributionRequest();
                oneip=get_ipport(hash.GetServerIndex(key));
                int nWrite=ConnectServ(oneip,kvwritecfd);
            }
        }
        int nWrite = WriteData(kvwritecfd,js);
        if(nWrite<0){
            oneipbf.port = 0;
            sleep(1);
            continue;
        }
        //设置新的ipport
        oneipbf = oneip;
        outfile<<key<<" "<<value<<" "<<oneip.port<<std::endl;
        Sleep_ms(300);
    }
    
    Close(kvwritecfd);
    outfile.close();
}

string Client::keyValueRequest(const string key){
    json js = WriteJs(key);
    json jsin;
    ipport_pair oneip;

    int j = 0;
    
    while(1){
        CountTime();
        //获取应该到哪儿缓存服务器中
        oneip = get_ipport(hash.GetServerIndex(key));
        //连接服务器直到连接成功
        int n = ConnectServ(oneip,kvreqcfd);
        while(n<0){
            //关闭文件描述符
            close(kvreqcfd);
            //休眠一下
            sleep(1);
            //重新拉取分布
            DistributionRequest();
            //获取这一次应该在哪个cacheServer
            oneip = get_ipport(hash.GetServerIndex(key));
            //建立连接
            n = ConnectServ(oneip,kvreqcfd);
        }
        //Write,连接成功，但是发送失败直接下一个
        int nwrite=WriteData(kvreqcfd,js);
        if(nwrite<0)
        {
            Close(kvreqcfd);
            DistributionRequest();
            continue;
        }
        //读取对应的返回值
        jsin=ReadData(kvreqcfd);
        //开始解析返回值
        if(jsin.empty())
        {
            Close(kvreqcfd);
            perr_exit("connection with cache fails");
        }else if(jsin["data"]["flag"]==false){
            Close(kvreqcfd);
            DistributionRequest();
            j++;
            if(j>2)
                perr_exit("Data lose");
            continue;
        }else if(jsin["type"]==KEY_VALUE_REQUEST&&jsin["data"]["key"]==key){
            break;
        }else{
            perr_exit("key value request returns fault");
        }
    }

    Close(kvreqcfd);
    string value = jsin["data"]["value"].get<string>();
    return value;
}
