#pragma once

#include <QWidget>
#include "OControl.h"
#include <QStandardItemModel>
#include <QThread>
#include <QTime>
#include "Global.h"
#include <QVector>
#include <QMap>
#include <QPair>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <boost/thread/thread.hpp> 


class OperateControl : public PageInit
{
	Q_OBJECT

public:
	OperateControl(QWidget *parent = Q_NULLPTR);
	~OperateControl();

	QVector<LoaderData> communicationcontrol_command_vector;
	QVector<ControlData> controldata_vector;
	int current_select_row;
	boost::thread communicationcontrol_thread;
	LoaderData this_command;
	int muti_frame_byte_number;
	int muti_frame_byte_counter;
	QTime* receive_timeout;
	qlonglong max_index;
	QVector<Record> communicationcontrol_Record_Vector;
	QMap<QString, QVector<BYTE> > receive_map_data;
	int stator_temperature;

	void startThread();
	void TableWrite(QString message_content, int message_type);
	void Init();
	void Get_Record();
	void send_realtime_data(SendData send_data);
	QVector<BYTE> receive_realtime_data(SendData send_data, ReceiveDiagloseData temp_receive_data);
	bool DealReceiveData(ReceiveDiagloseData temp_receive_data, QVector<BYTE> receive_byte_vector);
	void deal_send_receive();
	QVector<BYTE> analyse_send_singleframe(QString data_content);

signals:
	void SendCommunicationControlSignal(QString);
	void communicationcontrol_result_Signal(bool, QString);
	void display_messageSignal(QString, int);
	public slots :
	void communicationcontrolButtontSlot();
	void communicationcontrol_result_Slot(bool flag, QString function_name);
	void start_operate_Slot();
	void end_operate_Slot();
	void ChangeModeSlot(int temp_row);
private:
	Ui_OConrolForm ui;
};
