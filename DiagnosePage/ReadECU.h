#pragma once

#include <QWidget>
#include "ECU.h"
#include <QStandardItemModel>
#include <QThread>
#include <QTime>
#include "Global.h"
#include <QVector>
#include <QMap>
#include <boost/thread/thread.hpp> 

class ReadECU : public PageInit
{
	Q_OBJECT

public:
	ReadECU(QWidget *parent = Q_NULLPTR);
	~ReadECU();

	QVector<ECUInfoData> readecu_analyse_vector;
	ECUTableData current_ecutable_data;
	QMap<QString, QVector<BYTE> > receivecontent_map;
	int muti_frame_byte_number;
	int muti_frame_byte_counter;
	QVector<ECUInfoData> writeecu_analyse_vector;
	QVector<Record> Read_Record_Vector;
	QTime* receive_timeout;
	qlonglong max_index;
	boost::thread readecu_thread;
	boost::thread writeecu_thread;

	void TableWrite(QString message_content, int message_type);
	void write_ecu_data();
	void read_ecu_data();
	void Get_Record();
	void send_command(SendData send_data);
	QVector<BYTE> receive_command(SendData send_data, ReceiveDiagloseData temp_receive_data);
	QString AnalyseReceiveData(ReceiveAnalyseData temp_data_setting, QVector<BYTE> receive_byte_vector);
	QString AnalyseFormula(QString temp_formula, double temp_value);
	QVector<ECUData> GetAnalyseReceiveDataResult(ReceiveDiagloseData temp_data_setting, QVector<BYTE> receive_byte_vector);
	bool DealReceiveData(ReceiveDiagloseData temp_receive_data, QVector<BYTE> receive_byte_vector);
	void startReadECUThread();
	void startWriteECUThread();
	void resizeEvent(QResizeEvent *event);


signals:
	void send_ECU_result_Signal(ECUTableData);
	void send_writeECU_result_Signal(bool, QString);
	void display_messageSignal(QString, int);
	public slots :
		void send_ECU_result_Slot(ECUTableData temp_ecutable_data);
		void edit_pushbutton_click_Slot();
		void input_resultSlot(InputWriteECUData input_writeecudata);
		void send_writeECU_result_Slot(bool write_flag,QString writeitem_name);
		void Init();
private:
	Ui_ECUForm ui;
	
};
