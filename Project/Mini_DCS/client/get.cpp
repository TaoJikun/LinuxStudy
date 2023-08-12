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

#include <iostream>
#include <vector>
#include <string>
#include "client.h"
using namespace std;


int main(int argc,char* argv[])
{

    Client *cli = new Client();
    std::cout<<"x"<<std::endl;
    while(1)
    {
        string rkeyvalue=ReadText(KEY_FILE_NAME);
        int cut1=rkeyvalue.find_first_of(' ');
        int cut2=rkeyvalue.find_last_of(' ');

        string rkey=rkeyvalue.substr(0,cut1);
        string rvalue=rkeyvalue.substr(cut1+1,20);

       string value=cli->keyValueRequest(rkey);

        if(value==rvalue)
            cout<<"求取正确!"<<endl;
        else
            cout<<"求取错误!"<<endl;
    }

    return 0;

}




