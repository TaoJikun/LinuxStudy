#include <limits.h>
#include <time.h>
#include "gtest/gtest.h"
#include "client.hpp"
#define TEST_LOG_FILE_NAME "clienttestlog"

namespace {

class QuickTest : public testing::Test {
protected:
    // Remember that SetUp() is run immediately before a test starts.
    // This is a good place to record the start time.
    void SetUp() override { start_time_ = time(nullptr); }

    // TearDown() is invoked immediately after a test finishes.  Here we
    // check if the test was too slow.
    void TearDown() override {
        // Gets the time when the test finishes
        const time_t end_time = time(nullptr);
        EXPECT_TRUE(end_time - start_time_ <= 10) << "The test took too long.";
    }
    time_t start_time_;
};

class FunctionTest : public QuickTest {
   
};

TEST_F(FunctionTest, GetIPList) {
    // Tests function of create iplist
    std::string ip = "172.0.0.1:8000";
    auto iplist = GetIPList(ip);
    EXPECT_EQ(8000, iplist.port);
    EXPECT_STREQ("172.0.0.1", iplist.ip.c_str());

    ip = "192.78.62.136:22";
    iplist = GetIPList(ip);
    EXPECT_EQ(22, iplist.port);
    EXPECT_STREQ("192.78.62.136", iplist.ip.c_str());
}

TEST_F(FunctionTest, ReadTxt) {
    std::string rkeyvalue = ReadTxt(KEY_FILE_NAME);
    int cut1 = rkeyvalue.find_first_of(' ');
    int cut2 = rkeyvalue.find_last_of(' ');
    std::string rkey = rkeyvalue.substr(0,cut1);
    EXPECT_EQ(20,rkey.size());
}

TEST_F(FunctionTest, Randstr) {
    
    EXPECT_EQ(5,Randstr(5).size());
    EXPECT_EQ(20,Randstr(20).size());
    EXPECT_EQ(200,Randstr(200).size());

}

class ClientTest : public QuickTest {
protected:
    void SetUp() override {
        QuickTest::SetUp();
        logfile.OpenLogFile(TEST_LOG_FILE_NAME);
        cacheiplist = 
        {
        "127.0.0.1:8000",
        "127.0.0.1:8001",
        "127.0.0.1:8002"
        };
        this->cli = new Client(cacheiplist,&logfile);
    }

    virtual void TearDown() override {
        delete this->cli;
        QuickTest::TearDown();
    }

    Client* cli;
    LogFile logfile;
    std::vector<std::string> cacheiplist;
};

TEST_F(ClientTest, DefaultConstructor) {
    
    EXPECT_EQ(3, cli->alliplist.size());
    EXPECT_TRUE(std::count(cacheiplist.begin(), cacheiplist.end(), "127.0.0.1:8000"));
    EXPECT_TRUE(std::count(cacheiplist.begin(), cacheiplist.end(), "127.0.0.1:8001"));
    EXPECT_TRUE(std::count(cacheiplist.begin(), cacheiplist.end(), "127.0.0.1:8002"));

}

TEST_F(ClientTest, CountTime) {
    
    EXPECT_FALSE(cli->CountTime());
    sleep(5);
    EXPECT_FALSE(cli->CountTime()) << "5s fails";
    sleep(1);
    EXPECT_TRUE(cli->CountTime()) << "1s fails";
}

TEST_F(ClientTest, Writejs) {
    
    json kvwritejs = cli->Writejs(KEY_VALUE_RESPOND);
    EXPECT_EQ(20, kvwritejs["data"]["key"].get<std::string>().size());
    EXPECT_EQ(200, kvwritejs["data"]["value"].get<std::string>().size());
    EXPECT_TRUE(kvwritejs["data"]["flag"]);
    EXPECT_EQ(KEY_VALUE_RESPOND,kvwritejs["type"]);

    json kvreqjs = cli->Writejs("abcdefg");
    auto key = kvreqjs["data"]["key"].get<std::string>();
    EXPECT_STREQ("abcdefg",key.c_str());
    EXPECT_EQ(KEY_VALUE_REQUEST,kvreqjs["type"]);

    json disreqjs = cli->Writejs(DISTRIBUTION_REQUEST);
    EXPECT_EQ(DISTRIBUTION_REQUEST,disreqjs["type"]);
    
}

}

int main(int argc, char** argv) {
    // The following line must be executed to initialize Google Mock
    // (and Google Test) before running the tests.
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
