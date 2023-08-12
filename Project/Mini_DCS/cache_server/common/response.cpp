#include "response.h"


Response::Response(){

}



void Response::makeResponse(Buffer & buff,std::string  msg){
    //将S信息写入写缓冲区
    buff.append(msg);
}