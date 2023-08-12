#ifndef CACHESYSTEM_LOG_H
#define CACHESYSTEM_LOG_H

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

#endif //CACHESYSTEM_LOG_H
