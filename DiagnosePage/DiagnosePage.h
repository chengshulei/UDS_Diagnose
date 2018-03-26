#pragma once

#include <QtWidgets/QMainWindow>
#include "Diagnosis.h"
#include "SelectModelPage.h"
#include "ReadECU.h"
#include <QTimer>
#include <QThread>
#include "Global.h"
#include <QStandardItemModel>
#include "ReadRealTimeData.h"
#include "DealFault.h"
#include "SpecialFunction.h"
#include "CommunicationControl.h"
#include "OperateControl.h"
#include <boost/thread/thread.hpp> 
#include <boost/coroutine/coroutine.hpp>

typedef boost::coroutines::coroutine< void >::pull_type pull_coro_t;

class DiagnosePage : public QMainWindow
{
    Q_OBJECT

public:
    DiagnosePage(QWidget *parent = Q_NULLPTR);
	~DiagnosePage();

	int GetDeviceTypeValue(QString temp_devicetype);
	bool CANDeviceInit(int devicetype_value, int index, int cannum, QString baud_setting);
	void showEvent(QShowEvent *event);
	void startThread();
	void startDataThread();
	void dealwith_sendreceive();
	void send_EnterAndHeart_data(SendData send_data);
	void Get_Record();
	bool receive_EnterAndHeart_data(SendData send_data, ReceiveDiagloseData temp_receive_data);
	QVector<BYTE> analyse_send_singleframe(QString data_content);
	QVector<BYTE> ComputeKey(QVector<BYTE> seed);
	void send_receive();
	void Send_Data();
	void Receive_Data();
	void TakeDataToRecord(VCI_CAN_OBJ temp_frame);

	SelectModelPage* selectmodelpage;
	ReadECU* readecupage;
	ReadRealTimeData * realtimedata_page;
	DealFault* dealfault_page;
	SpecialFunction* specialfunction_page;
	CommunicationControl * communicationcontrol_page;
	OperateControl *operatecontrol_page;

	ModelInfo select_model;
	QVector<LoaderData> init_analyse_vector;
	UINT select_receive_frame_id;
	UINT select_send_frame_id;
	Exist_Device useful_device;
	boost::thread send_receive_thread;
	boost::thread data_thread;
	qlonglong max_index;
	QTime * receivetime_count;
	QVector<Record> EnterAndHeart_Record_Vector;
	QMap<QString, QVector<BYTE> > receive_map_data;
	qlonglong temp_index;
	bool startthread_flag;
	int thread_index;
	QMap<int, PageInit*> function_map;
	int last_select_list_index;

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);

	bool        mMoveing;
	QPoint      mMovePosition;


signals:
	void receivedataSignal();
	void DiagnoseEnterSiganl();
	void display_messageSignal(QString, int);
	void stop_realtimedata_Siganl();
	public slots :
	void ReceiveSelectModelSlot(ModelInfo selected_modelinfo);
	void DiagnoseEnterSlot();
	void ChangeModeSlot(int temp_row);
	void TableWrite(QString message_content, int message_type);
	void communication_resultSlot(Exist_Device temp_commu_setting);
	void close_windowSlot();
	void min_windowSlot();

private:
	Ui_MainWindow ui;
};
