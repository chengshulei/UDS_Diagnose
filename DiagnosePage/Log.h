#pragma once

#pragma execution_character_set("utf-8")//����֧��

#include <QString>
#include <QTextStream>
#include <QFile>
#include <QMutex>
#include <QDateTime>
using namespace std;

#define NORMAL_FLAG "NORMAL: "//4����־�����־
#define ERROR_FLAG "ERROR: "
#define WARNING_FLAG "WARNING: "
#define BACKUP_FLAG "BACKUP: "


#define LOG         HandLog::GetLogHandle(NORMAL_FLAG)
#define LOG_E       HandLog::GetLogHandle(ERROR_FLAG)
#define LOG_W       HandLog::GetLogHandle(WARNING_FLAG)
#define LOG_B       HandLog::GetLogHandle(BACKUP_FLAG)

//ʹ��LOG<<  LOG_E<<  LOG_W<<  LOG_B<<  ����������
//ע�⣺������ܵ��ӣ�ֻ�����һ�Σ�������ӻ��У���β�Զ����

//����ʹ���˵���ģʽ������ƣ������ظ�����
//������ 20170307


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
	inline QTextStream& operator<<(const QString &str) //�������غ���,���
	{
		file_mutex->lock();//��������ֹͬʱд��
		if (file_open_flag)
		{
			if (!falg_exist)
			{
				if (str == NORMAL_FLAG || str == ERROR_FLAG || str == WARNING_FLAG || str == BACKUP_FLAG)//4����־�����־
				{
					falg_exist = true;//�����ظ����4�������־
					*out << str << QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz    ");
				}
			}
			else
			{
				if (str != NORMAL_FLAG && str != ERROR_FLAG && str != WARNING_FLAG && str != BACKUP_FLAG)
				{
					//*out << str << endl;//ע�������Զ���ӻ���
					*out << str << "\r\n";//�� \r\n �滻endl��Ϊ�˼��±�����ȷ����
					out->flush(); //endl�е���flush �����������������
					falg_exist = false;//�����ظ����4�������־
				}
			}
		}
		file_mutex->unlock();
		return *out;
	}

	class DeleteLog //������Log����������
	{
	public:

		~DeleteLog()
		{
			if (Log::p)
			{
				if (Log::file_open_flag)
				{
					//Log::out->setGenerateByteOrderMark(true);//������true�Ļ� ���Ǳ����"UTF-8 + BOM"��false�Ļ� ���Ǳ����"UTF-8"
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

	static Log GetLogHandle(QString temp_str)//�����־��־
	{
		Log * log_system = Log::initance();
		*log_system << temp_str;
		return *log_system;
	}
};