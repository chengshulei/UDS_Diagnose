#include "Log.h"
#include <QDir>
#include <QFile>
#include <QCoreApplication>


Log* Log::p = new Log;
QString Log::file_path = "";
QTextStream* Log::out = NULL;
bool Log::init_flag = false;
bool Log::file_open_flag = false;
bool Log::falg_exist = false;
QMutex* Log::file_mutex = new QMutex;
QFile* Log::file = NULL;

Log* Log::initance()//��ʼ����־��
{
	file_mutex->lock();
	if (!init_flag)//�Ƿ��Ѿ���ʼ�����ļ�����ֻȷ��һ��
	{
		
		QString temp_path = QCoreApplication::applicationDirPath();

		QDir *temp_dir = new QDir;

		QString dir_whole_path = temp_path + "/" + QStringLiteral("��־");
		if (temp_dir->exists(dir_whole_path))
		{
			QString temp_filename = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log";
			QString file_whole_path = dir_whole_path + "/" + temp_filename;

			file_path = file_whole_path;
		}
		else
		{
			if (temp_dir->mkdir(dir_whole_path))
			{
				QString temp_filename = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log";
				QString file_whole_path = dir_whole_path + "/" + temp_filename;
				file_path = file_whole_path;
			}
		}
		delete temp_dir;
		init_flag = true;

		file = new QFile(file_path);
		//if (file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) //���ı���ʽ׷��д��
		if (file->open(QFile::WriteOnly | QFile::Append)) //׷��д��
		{
			out = new QTextStream(file);
			out->setCodec("UTF-8");//����UTF-8����
			file_open_flag = true;
		}
	}
	file_mutex->unlock();
	return p;
}
