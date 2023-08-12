//
// Created by admin on 2022/5/18.
//
#include "log.h"


LogFile::LogFile(LOG_LEVEL level)
{
    cached_logs.clear();
    log_mutex = PTHREAD_MUTEX_INITIALIZER;
    plogfile = NULL;
    pthread_mutex_init(&log_mutex,NULL);
    sem_init(&log_semphore,0,0);
    m_level = level;
}

LogFile::~LogFile()
{
    pthread_mutex_destroy(&log_mutex);
    sem_destroy(&log_semphore);
    if (plogfile != NULL) fclose(plogfile);
}

//打开日志文件
void LogFile::OpenLogFile(const char* filename)
{
    //如果已经有了，那么需要先关闭这个文件了！
    if (plogfile != NULL) fclose(plogfile);
    plogfile = fopen(filename,"a+");
}

//写入到我们需要的文件
bool LogFile::WriteToFile(const std::string& line)
{
    if (plogfile == NULL) return false;
    if (fwrite((void*)line.c_str(),1,line.length(),plogfile) != line.length()) return false;
    fflush(plogfile);
    return true;
}

//消费者模型，开始拉取对应的文件,单独的线程来运行...
void* LogFile::LogFileConsumer(void* arg1)
{
    LogFile* arg = (LogFile*)arg1;
    std::string line;
    while (true)
    {
        sem_wait(&arg->log_semphore);
        pthread_mutex_lock(&arg->log_mutex);
        if (!arg->cached_logs.empty())
        {
            line = arg->cached_logs.front();
            arg->cached_logs.pop_front();
        }  
        pthread_mutex_unlock(&arg->log_mutex);
        //如果是空行，则先置空!!!
        if (line.empty())
        {
            std::cout<<"line is empty:"<<std::endl;
            sleep(1);
            continue;
        }
        arg->WriteToFile(line);
        line.clear();
    }
}




void LogFile::GetTime(char* timeStr)
{
    time_t rawtime;
    struct tm *ptminfo;
    time(&rawtime);
    ptminfo = localtime(&rawtime);
    snprintf(timeStr,64,"[%04d-%02d-%02d %02d:%02d:%02d]",
              ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,
              ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec);
}


//生产者日志
void LogFile::WriteLog(LOG_LEVEL level, std::string func, int line, const char* fmt, ...)
{
    std::ostringstream ss;
    //time
    char strTime[TimeStirngLen] = {};
    GetTime(strTime);
    ss << strTime;
    //level
    const char* LogLevelNames[] = {"VERBOSE","TRACE","INFO","WARNING","ERROR","FATAL"};
    ss << "[" << LogLevelNames[level] << "]";
    ss << "[tid:" << gettid()<<"]";
    ss << "[" << func << ":" << line <<"]";
    //content
    va_list ap;
    va_start(ap,fmt);
    int len = vsnprintf(NULL,0,fmt,ap);
    va_end(ap);
    va_start(ap,fmt);
    char* content = new char[len+1];
    vsnprintf(content,len+1,fmt,ap);
    va_end(ap);
    ss << content;
    ss << "\n";
    
    pthread_mutex_lock(&log_mutex);
    cached_logs.push_back(ss.str());
    pthread_mutex_unlock(&log_mutex);
    sem_post(&log_semphore);
}