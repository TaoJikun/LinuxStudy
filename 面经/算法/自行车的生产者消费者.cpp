

#include <iostream>
#include <vector>
#include "pthread.h"
std::vector<int> vctBody;
std::vector<int> vctWheel;

pthread_mutex_t body;
pthread_mutex_t wheel;

pthread_cond_t bodyCon;
pthread_cond_t wheelCon;


void* BodyProducer(void *)
{
    while(1)
    {
        pthread_mutex_lock(&body);
        while(vctBody.size()>=10)
        {
            pthread_cond_wait(&bodyCon,&body);
        }
    
        vctBody.push_back(1);
    
        std::cout<<"Body Producer Num is "<<vctBody.size()<<std::endl;
        pthread_mutex_unlock(&body);
    }


    return NULL;
}


void* WheelProducer(void*)
{
    while(1)
    {
       pthread_mutex_lock(&wheel);
       while(vctWheel.size()>=20)
       {
          pthread_cond_wait(&wheelCon,&wheel);
       }
       vctWheel.push_back(1);
       std::cout<<"Wheel Producer Num is "<<vctBody.size()<<std::endl;
       pthread_mutex_unlock(&wheel);
    }
    return NULL;

}

void* CycleConsumer(void*)
{
    while(1)
    {
         pthread_mutex_lock(&body);
         int bodyNum = vctBody.size();
         pthread_mutex_lock(&wheel);
         int wheelNum = vctWheel.size();
         bool isContinue = bodyNum>0&&wheelNum>=2;
         if(!isContinue)
         {
             pthread_mutex_unlock(&body);
             pthread_mutex_unlock(&wheel);
             continue;
         }
         vctBody.pop_back();
         bodyNum = vctBody.size();
         if(bodyNum<5)
         {
             pthread_cond_signal(&bodyCon);
         }
         vctWheel.pop_back();
         vctWheel.pop_back();
         wheelNum = vctWheel.size();
         if(wheelNum<10)
         {
             pthread_cond_signal(&wheelCon);
         }
     
         std::cout<<"Cycle Consumer"<<std::endl;
         pthread_mutex_unlock(&body);
         pthread_mutex_unlock(&wheel);
    }
    return NULL;
}


int main()
{
    pthread_t bodyPro;
    pthread_t wheelPro;
    pthread_t cycleCon;

    pthread_create(&bodyPro,NULL,BodyProducer,NULL);
    pthread_create(&wheelPro,NULL,WheelProducer,NULL);
    pthread_create(&cycleCon,NULL,CycleConsumer,NULL);

    pthread_join(bodyPro,NULL);
    pthread_join(wheelPro,NULL);
    pthread_join(cycleCon,NULL);


    system("pause");
    return 0;

}