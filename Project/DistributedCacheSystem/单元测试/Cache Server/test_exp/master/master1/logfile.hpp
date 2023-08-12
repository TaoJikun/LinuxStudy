#ifndef LOGFILE_HPP
#define LOGFILE_HPP

#include <pthread.h>
#include <semaphore.h>
#include <list>
#include <sstream>
#include <unistd.h>
#include <ctime>
#include <stdarg.h>

#include <iostream>

enum LOG_LEVEL
{
	VERBOSE = 0,
	TRACE = 1,
	INFO = 2,
	WARNING = 3,
	ERROR = 4,
	FATAL = 5
};


#define LogVERBOSE(...) WriteLog(VERBOSE,__func__,__LINE__,__VA_ARGS__)
#define LogTRACE(...) WriteLog(TRACE,__func__,__LINE__,__VA_ARGS__)
#define LogInfo(...) WriteLog(INFO,__func__,__LINE__,__VA_ARGS__)
#define LogWARNING(...) WriteLog(WARNING,__func__,__LINE__,__VA_ARGS__)
#define LogERROR(...) WriteLog(ERROR,__func__,__LINE__,__VA_ARGS__)
#define LogFATAL(...) WriteLog(FATAL,__func__,__LINE__,__VA_ARGS__)
#define TimeStirngLen 32

class LogFile
{
public:
	LogFile(LOG_LEVEL level = INFO);
	~LogFile();

    void OpenLogFile(const char* filename);
    static void* LogFileConsumer(void* arg1);
    void WriteLog(LOG_LEVEL level, std::string func, int line, const char* fmt, ...);
    // static void WriteLog(LOG_LEVEL level, std::string func, int line, LogFile* logfile, const char* fmt, ...);

private:
	std::list<std::string> cached_logs;
	pthread_mutex_t log_mutex;
	sem_t log_semphore;
	FILE * plogfile;
	int m_level;

    bool WriteToFile(const std::string& line);
    static void GetTime(char* timeStr);
};

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

void LogFile::OpenLogFile(const char* filename)
{
    if (plogfile != NULL) fclose(plogfile);
    plogfile = fopen(filename,"a+");
}

bool LogFile::WriteToFile(const std::string& line)
{
    if (plogfile == NULL) return false;
    if (fwrite((void*)line.c_str(),1,line.length(),plogfile) != line.length()) return false;
    fflush(plogfile);
    return true;
}

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
        if (line.empty())
        {
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

#endif // LOGFILE_HPP