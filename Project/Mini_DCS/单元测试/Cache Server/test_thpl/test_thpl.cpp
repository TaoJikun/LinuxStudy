#include "thrd_pool.hpp"
#include "gtest/gtest.h"
namespace{
    void Sleep_ms(int n){
        struct timeval delay;
        delay.tv_sec = 0;
        delay.tv_usec = n * 1000;
        select(0, NULL, NULL, NULL, &delay);
    }
    void function(void *arg){
        int count=0;
        while(1){
            count++;
            sleep(1);
        }
    }
    TEST(ThreadPool, getAliveNumber){
        Task T(function, nullptr);
        int maxx = 20;
        ThreadPool TP(10,maxx);
        EXPECT_EQ(10, TP.getAliveNumber());
        for (int i=1; i<=50; i++){
            Task T(function, nullptr);
            TP.addTask(T);
            Sleep_ms(10);
            if(i<=maxx)
                EXPECT_EQ(10, TP.getAliveNumber());
            else{
                EXPECT_TRUE(10<TP.getAliveNumber()&&20>=TP.getAliveNumber());
            }
        }
    }
}
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    return 0;
}

