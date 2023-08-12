#include <limits.h>
#include "gtest/gtest.h"
#include "consistenthash.hpp"
#include <unordered_set>
namespace {
class CHTest : public testing::Test {
protected:
    void SetUp() override {
        srand((unsigned)time(NULL));
        for (int i=0; i<3; i++)
        {
            std::string ip = std::to_string(rand()%256)+"."
                                +std::to_string(rand()%256)+"."
                                +std::to_string(rand()%256)+"."
                                +std::to_string(rand()%256)+":8080";
            iplist.push_back(ip);
        }
        ch.RefreshIPList(iplist);
    }
    ConsistentHash ch;
    std::vector<std::string> iplist;
};

TEST_F(CHTest, Balance) {
    std::map<std::string,int> ipmap;
    for(int i=0; i<3; i++) ipmap.insert({iplist[i],i});

    double cnt[3];
    memset(cnt, 0, sizeof(cnt));
    int n = 10000;
    for (int i=0; i<n; i++)
    {
        std::string s0 = ConsistentHash::RandStr(20);
        std::string s = ch.GetServerIndex(s0);
        cnt[ipmap[s]]++;
    }
    cnt[0] = cnt[0] / n;
    cnt[1] = cnt[1] / n;
    cnt[2] = cnt[2] / n;
    EXPECT_LE(abs(cnt[0]-0.3),0.1);    
    EXPECT_LE(abs(cnt[1]-0.3),0.1);    
    EXPECT_LE(abs(cnt[2]-0.3),0.1);    
}

TEST_F(CHTest, Delete) {
    for (int i=0; i<1000; i++)
    {
        std::string s0 = ConsistentHash::RandStr(20);
        std::string s1 = ch.GetServerIndex(s0);
        for (auto& ip:iplist)
        {
            ch.DeleteServer(ip);
            std::string s2 = ch.GetServerIndex(s0);
            EXPECT_FALSE(s2 != s1 && ip != s1);
            ch.AddServer(ip);
        }
    }
}

TEST_F(CHTest, Add) {
    for (int i=0; i<1000; i++)
    {
        std::string newip = std::to_string(rand()%256)+"."
                            +std::to_string(rand()%256)+"."
                            +std::to_string(rand()%256)+"."
                            +std::to_string(rand()%256)+":8080";

        std::string s0 = ConsistentHash::RandStr(20);
        std::string s1 = ch.GetServerIndex(s0);
        ch.AddServer(newip);
        std::string s2 = ch.GetServerIndex(s0);
        EXPECT_FALSE(s2 != s1 && newip != s2);
        ch.DeleteServer(newip);

    }
}

TEST_F(CHTest, BackUp) {
    std::unordered_set<std::string> myset;
    std::string ip0 = iplist[0];
    std::string ip = ip0;
    int cnt = 0;
    while(1)
    {
        ip = ch.GetBackUpServer(ip);
        bool flag = myset.find(ip) == myset.end();
        if (flag)
        {
            myset.emplace(ip);
            cnt++;
            ip = ch.GetBackUpServer(ip);
        }
        else break;
    }
    EXPECT_EQ(cnt,3);
    EXPECT_EQ(ip,ch.GetBackUpServer(ip0));
}

}//namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}