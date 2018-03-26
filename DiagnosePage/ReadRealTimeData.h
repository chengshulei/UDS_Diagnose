#pragma once

#include <QWidget>
#include "RealTimeData.h"
#include <QStandardItemModel>
#include <QThread>
#include <QTime>
#include "Global.h"
#include <QVector>
#include <QMap>
#include <QPair>
#include <boost/thread/thread.hpp> 

class treeitem_package
{
public:
	/*treeitem_package(QString str, Qt::CheckState temp_state)
	{
	m_checkstate = temp_state;
	content = str;
	}
	treeitem_package()
	{
	m_checkstate = Qt::Unchecked;
	content = "";
	}
	treeitem_package(treeitem_package& temp_item)
	{
	m_checkstate = temp_item.m_checkstate;
	content = temp_item.content;
	}*/

	Qt::CheckState m_checkstate;
	QString content;
};


class ReadRealTimeData : public PageInit
{
	Q_OBJECT

public:
	ReadRealTimeData(QWidget *parent = Q_NULLPTR);
	~ReadRealTimeData();

	QStandardItemModel *tree1;
	QStandardItemModel *table1;
	int refresh_flag;
	//QMap<int, QMap<int, Qt::CheckState> > allcheckState_map;
	//QMap<int, QMap<int, QString> > alltreecontent_map;
	QMap<int, QMap<int, treeitem_package> > alltreeitem_map;
	QVector<LoaderData> read_realtimedata_command_vector;
	int muti_frame_byte_number;
	int muti_frame_byte_counter;
	QTime* receive_timeout;
	qlonglong max_index;
	QVector<Record> ReadRealTime_Record_Vector;
	boost::thread readdata_thread;
	boost::thread realtimedata_thread;
	bool startthread_flag;

	void Init();
	QVector<ECUInfoData> AnalyseDiagnoseSettingFile(QString file_data);
	SendData GetSendData(QString file_data, QString send_id);
	QString GetCaption(QString file_data, QString caption_id);
	ReceiveDiagloseData GetReceiveData(QString file_data, QString receive_id);
	void startReadThread();
	void startRealTimeDataThread();
	void TableWrite(QString message_content, int message_type);
	ReceiveAnalyseData GetByteAnalyseData(QString file_data, QString ByteAnalyse_id);
	QMap<int, QString> GetContent(QString file_data, QString content_id);
	void refresh_table();
	void stopThread();
	void Get_Record();
	void send_realtime_data(SendData send_data);
	QVector<BYTE> receive_realtime_data(SendData send_data, ReceiveDiagloseData temp_receive_data);
	QString AnalyseReceiveData(ReceiveAnalyseData temp_data_setting, QVector<BYTE> receive_byte_vector);
	QString AnalyseFormula(QString temp_formula, double temp_value);
	UnOrderMap<QString, QString> GetAnalyseReceiveDataResult(ReceiveDiagloseData temp_data_setting, QVector<BYTE> receive_byte_vector);
	void read_realtimedata();
	void read_data();

signals:
	void send_realtimedata_result_Signal(TreeData);
	void display_messageSignal(QString, int);
	void stop_refresh_Signal();
	public slots :
	void send_realtimedata_result_Slot(TreeData temp_tree_data);
	void Check_Signal_Slot();
	void Start_Refresh_Slot();
	void stop_refresh_Slot();
	void treeItemChanged(QStandardItem * item);
	void ReadData_Slot();

private:
	Ui_RealTimeDataForm ui;
};
