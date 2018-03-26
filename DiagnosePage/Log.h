#pragma once

#pragma execution_character_set("utf-8")//中文支持

#include <QString>
#include <QTextStream>
#include <QFile>
#include <QMutex>
#include <QDateTime>
using namespace std;

#define NORMAL_FLAG "NORMAL: "//4种日志输出标志
#define ERROR_FLAG "ERROR: "
#define WARNING_FLAG "WARNING: "
#define BACKUP_FLAG "BACKUP: "


#define LOG         HandLog::GetLogHandle(NORMAL_FLAG)
#define LOG_E       HandLog::GetLogHandle(ERROR_FLAG)
#define LOG_W       HandLog::GetLogHandle(WARNING_FLAG)
#define LOG_B       HandLog::GetLogHandle(BACKUP_FLAG)

//使用LOG<<  LOG_E<<  LOG_W<<  LOG_B<<  即可完成输出
//注意：输出不能叠加，只能输出一次，不用添加换行，行尾自动添加

//这里使用了单例模式进行设计，避免重复定义
//程书雷 20170307


class Log 
{
protected:
	Log(){}
private:
	static Log* p;
public:
	static Log* initance();

	static bool init_flag;
	static bool file_open_flag;

	static QString file_path;
	static QTextStream* out;

	static QFile* file;

	static QMutex* file_mutex;
	static bool falg_exist;
	inline QTextStream& operator<<(const QString &str) //定义重载函数,输出
	{
		file_mutex->lock();//加锁，防止同时写入
		if (file_open_flag)
		{
			if (!falg_exist)
			{
				if (str == NORMAL_FLAG || str == ERROR_FLAG || str == WARNING_FLAG || str == BACKUP_FLAG)//4种日志输出标志
				{
					falg_exist = true;//避免重复输出4种输出标志
					*out << str << QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz    ");
				}
			}
			else
			{
				if (str != NORMAL_FLAG && str != ERROR_FLAG && str != WARNING_FLAG && str != BACKUP_FLAG)
				{
					//*out << str << endl;//注意这里自动添加换行
					*out << str << "\r\n";//用 \r\n 替换endl是为了记事本能正确换行
					out->flush(); //endl有调用flush 函数，所以这里加上
					falg_exist = false;//避免重复输出4种输出标志
				}
			}
		}
		file_mutex->unlock();
		return *out;
	}

	class DeleteLog //它代替Log的析构函数
	{
	public:

		~DeleteLog()
		{
			if (Log::p)
			{
				if (Log::file_open_flag)
				{
					//Log::out->setGenerateByteOrderMark(true);//参数是true的话 就是保存成"UTF-8 + BOM"，false的话 就是保存成"UTF-8"
					Log::file->close();
					delete Log::out;
				}
				delete Log::file_mutex;
				delete Log::file;
				delete Log::p;

				Log::p = NULL;
				Log::out = NULL;
				Log::file_mutex = NULL;
				Log::file = NULL;
			}
		}
	};

	static DeleteLog deletelog;
};

class HandLog
{
public:

	static Log GetLogHandle(QString temp_str)//输出日志标志
	{
		Log * log_system = Log::initance();
		*log_system << temp_str;
		return *log_system;
	}
};