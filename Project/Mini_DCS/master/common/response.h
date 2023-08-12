#ifndef RESPONSE_H
#define RESPONSE_H

#include "buffer.h"

class Response{
public:
    Response();
    ~Response()=default;
    
    void makeResponse(Buffer& buffer,std::string s);
private:
    
};

#endif