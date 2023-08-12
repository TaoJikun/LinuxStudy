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

#include "../src/nodeConHash.h"

#include <ctime>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <unordered_map>
using namespace std;

int main(){

    vector<string> ip_Port = {"172.168.1.111:8080","172.168.1.112:8080","192.168.1.113:8080","172.168.1.114:8080","192.168.1.115:8080","192.168.1.116:8080","192.168.1.117:8080"};

    NodeConHash conHash;

    for(int i=0;i<ip_Port.size();i++) {
        cout<<ip_Port[i]<<endl;
        conHash.addPhysicalMachine(ip_Port[i]);
    }

    for(auto c:conHash.mp_itoIpPort){

        //cout<<"hashValue: "<<c.first<<" "<<"ip_port: "<<c.second<<endl;
    }

    vector<string> keys = {"xAWEGF","ddASDQGSDGDSA!saf","dsgadsaaaEEGSgasg","adsgsssGrty","hgfakfazdv","wrewgdxvc","wshjq","sg","hsfdaaad","ewtgavdwgrd","rehwgg   rwE2W"};

    //time(NULL);
    srand(time(0));
    for(int i=0;i<10000;i++){
        //srand(time(0));
        int t = rand();
        //cout<<t<<endl;
        keys.push_back(to_string(t));
    }


    unordered_map<string,int> res;

    for(auto c:keys){
        string ipPort = conHash.findPhysicalAccordKey(c);
        //cout<<"ipPort: "<<ipPort<<endl;
        res[ipPort]++;
    }

    for(auto c:res){
        cout<<c.second<<endl;
    }


    return 0;
}