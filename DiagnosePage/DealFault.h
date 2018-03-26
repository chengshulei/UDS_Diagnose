#pragma once

#include <QWidget>
#include "Fault.h"
#include <QStandardItemModel>
#include "Global.h"
#include <QThread>
#include <boost/thread/thread.hpp> 
#include <QTimer>
#include "LoadDialog.h"


class DealFault : public PageInit
{
	Q_OBJECT

public:
	DealFault(QWidget *parent = Q_NULLPTR);
	~DealFault();

	QStandardItemModel *table2;
	FaultTableData current_fault_data;
	LoaderData this_command;
	int muti_frame_byte_number;
	int muti_frame_byte_counter;
	QTime* receive_timeout;
	qlonglong max_index;
	QVector<Record> Fault_Record_Vector;
	QMap<QString, QVector<BYTE> > receive_map_data;
	boost::thread fault_thread;
	boost::thread clearfault_thread;
	QTimer * thread_wait_timer;
	int thread_wait_timer_count;
	LoadDialog* temp_load_dialog;

	void TableWrite(QString message_content, int message_type);
	void startFaultThread();
	void startClearFaultThread();
	QVector<FaultInfo> AnalyseFaultInfo(QVector<BYTE> temp_data, LitteRecord this_record);
	QVector<SnapshotRecord> AnalyseSnapShotInfo(QVector<BYTE> temp_data, LitteRecord this_record);
	QVector<FaultSort> AnalyseFaultSort(QVector<BYTE> temp_data, LitteRecord this_record);
	SnapshotLib AnalyseSnapshotLib(QVector<BYTE> temp_data);
	SnapshotLib AnalyseSnapshotLib(QString SnapshotDID, QVector<BYTE> temp_data);
	void Get_Record();
	void send_command_data(SendData send_data);
	QVector<BYTE> receive_command_data(SendData send_data, ReceiveDiagloseData temp_receive_data);
	bool DealReceiveData(ReceiveDiagloseData temp_receive_data, QVector<BYTE> receive_byte_vector);
	QMap<QString, QString> GetAnalyseReceiveDataResult(ReceiveDiagloseData temp_data_setting, QVector<BYTE> receive_byte_vector);
	QString AnalyseFormula(QString temp_formula, double temp_value);
	void deal_send_receive();
	void resizeEvent(QResizeEvent *event);
	void clear_fault();

public:
signals :
	void send_fault_result_Signal(FaultTableData);
		void send_snapshot_result_Signal(SnapShotInfo);
		void send_faultsort_result_Signal(FaultSortTableData);
		void send_clearfault_result_Signal(bool);
		void display_messageSignal(QString, int);
		void start_loading_Signal();
		void close_loading_Signal();
		public slots:
		void send_fault_result_Slot(FaultTableData faulttable_data);
		void send_snapshot_result_Slot(SnapShotInfo snapshottable_data);
		void send_faultsort_result_Slot(FaultSortTableData faultsorttable_data);
		void Init();
		void ReadECUFaultSort();
		void ClearECUFault();
		void send_clearfault_result_Slot(bool temp_flag);
		void BackToECUFault();
		void shotcut_pushbutton_click_Slot();
		void doubleClicked_Slot(QModelIndex index);
		void start_loading_Slot();
		void timeoutSlot();
		void close_loading_Slot();
		
private:
	Ui_FaultForm ui;
};
