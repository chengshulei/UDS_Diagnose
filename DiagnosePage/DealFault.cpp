#include "DealFault.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQueue>
#include <QMutex>
#include <QBitArray>
#include <QScrollBar>
#include <boost/function/function0.hpp>   
#include <boost/thread/condition.hpp>  
#include <boost/thread/mutex.hpp>  
#include <boost/atomic.hpp>  
#include "DealSettingFile.h"


extern bool CAN_open;
extern QQueue<QVector<VCI_CAN_OBJ> > send_can_data_queue;
extern QMutex send_queue_mutex;
extern QQueue<Record> CAN_Record;
extern QMutex record_mutex;
extern FunctionBlock current_page_block;
extern boost::mutex data_thread_mutex;
extern boost::condition_variable_any heart_thread_condition;
extern boost::atomic_int current_thread_order;
extern DealSettingFile deal_settingfile;

DealFault::DealFault(QWidget *parent)
{
	ui.setupUi(this);

	Global::loadQSS(this, "default.qss");

	muti_frame_byte_counter = 0;
	muti_frame_byte_number = 0;
	receive_timeout = new QTime;
	receive_timeout->start();
	thread_wait_timer = new QTimer(this);
	thread_wait_timer_count = 0;
	max_index = -1;

	ui.pushButton->setText(QStringLiteral("��ȡ����"));
	ui.pushButton_2->setText(QStringLiteral("��������"));
	ui.pushButton_3->setText(QStringLiteral("�������"));
	ui.pushButton_5->setText(QStringLiteral("����"));

	ui.tableWidget->clear();
	ui.tableWidget->setColumnCount(6);
	ui.tableWidget->setShowGrid(false);//��������
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//���ɱ༭
	ui.tableWidget->verticalHeader()->setVisible(false);
	ui.tableWidget->setItemDelegate(new NoFocusRectangleStyle());
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//������

	QStringList tableheader2;
	tableheader2 << QStringLiteral("���ϴ���") << QStringLiteral("����") << QStringLiteral("����") << QStringLiteral("״̬") << QStringLiteral("����ԭ��") << QStringLiteral("ά��ָ��");
	ui.tableWidget->setHorizontalHeaderLabels(tableheader2);
	ui.pushButton->setVisible(true);
	ui.pushButton_2->setVisible(true);//��������Ĭ�����أ�ֱ����ȡ���Ϻ͹����������
	ui.pushButton_3->setVisible(true);
	ui.pushButton_5->setVisible(false);
	ui.tableWidget->setVisible(true);
	ui.tableView_2->setVisible(false);

	table2 = new QStandardItemModel();
	ui.tableView_2->setModel(table2);
	//ui.tableView_2->horizontalHeader()->setStretchLastSection(true);//���һ���������հ�
	ui.tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//������
	ui.tableView_2->setShowGrid(false);//��������
	ui.tableView_2->setEditTriggers(QAbstractItemView::NoEditTriggers);//���ɱ༭
	ui.tableView_2->setItemDelegate(new NoFocusRectangleStyle());

	

	connect(this, SIGNAL(send_fault_result_Signal(FaultTableData)), this, SLOT(send_fault_result_Slot(FaultTableData)));
	connect(this, SIGNAL(send_snapshot_result_Signal(SnapShotInfo)), this, SLOT(send_snapshot_result_Slot(SnapShotInfo)));
	connect(this, SIGNAL(send_faultsort_result_Signal(FaultSortTableData)), this, SLOT(send_faultsort_result_Slot(FaultSortTableData)));
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(Init()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(ReadECUFaultSort()));
	connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(ClearECUFault()));
	connect(ui.pushButton_5, SIGNAL(clicked()), this, SLOT(BackToECUFault()));
	connect(this, SIGNAL(send_clearfault_result_Signal(bool)), this, SLOT(send_clearfault_result_Slot(bool)));
	connect(ui.tableWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked_Slot(QModelIndex)));
	connect(this, SIGNAL(close_loading_Signal()), this, SLOT(close_loading_Slot()));
	connect(this, SIGNAL(start_loading_Signal()), this, SLOT(start_loading_Slot()));
	connect(thread_wait_timer, SIGNAL(timeout()), this, SLOT(timeoutSlot()));
}

DealFault::~DealFault()
{
}

void DealFault::Init()
{
	LoaderData temp_command = deal_settingfile.GetSingleCommandFromDiagnoseSettingFile("DtcInf", QStringLiteral("��ECU����"));

	if (temp_command.send_data.frame_id == 0)
	{
		TableWrite("�����ļ�����ʧ�ܣ�û�л���������ݣ�", 5);
	}
	else
	{
		this_command = temp_command;
		if (CAN_open)
		{
			TableWrite("��ʼ��ȡ�����룡", 1);
			startFaultThread();
		}
	}
}

void DealFault::ReadECUFaultSort()
{
	LoaderData temp_command = deal_settingfile.GetSingleCommandFromDiagnoseSettingFile("DtcInf", QStringLiteral("��������"));

	if (temp_command.send_data.frame_id == 0)
	{
		TableWrite("�����ļ�����ʧ�ܣ�û�л���������ݣ�", 5);
	}
	else
	{
		this_command = temp_command;
		if (CAN_open)
		{
			TableWrite("��ʼ��ȡ��������", 1);
			startFaultThread();
		}
	}
}

void DealFault::ClearECUFault()
{
	LoaderData temp_command = deal_settingfile.GetSingleCommandFromDiagnoseSettingFile("DtcInf", QStringLiteral("���ȫ��������"));
	if (temp_command.send_data.frame_id == 0)
	{
		TableWrite("�����ļ�����ʧ�ܣ�û�л���������ݣ�", 5);
	}
	else
	{
		this_command = temp_command;
		if (CAN_open)
		{
			TableWrite("��ʼ��������룡", 1);
			startClearFaultThread();
		}
	}
}

void DealFault::TableWrite(QString message_content, int message_type)//����������Ϣ����
{
	emit display_messageSignal(message_content, message_type);
}

void DealFault::startFaultThread()//�����߳�
{
	boost::function0<void> f1 = boost::bind(&DealFault::deal_send_receive, this);
	fault_thread = boost::thread(f1);
}
void DealFault::startClearFaultThread()//�����߳�
{
	boost::function0<void> f1 = boost::bind(&DealFault::clear_fault, this);
	clearfault_thread = boost::thread(f1);
}

void DealFault::send_fault_result_Slot(FaultTableData faulttable_data)
{
	ui.tableWidget->clear();
	ui.tableWidget->setColumnCount(6);

	QStringList tableheader2;
	tableheader2 << QStringLiteral("���ϴ���") << QStringLiteral("����") << QStringLiteral("����") << QStringLiteral("״̬") << QStringLiteral("����ԭ��") << QStringLiteral("ά��ָ��");
	ui.tableWidget->setHorizontalHeaderLabels(tableheader2);

	ui.pushButton->setVisible(true);
	ui.pushButton_2->setVisible(true);//��������Ĭ�����أ�ֱ����ȡ���Ϻ͹����������
	ui.pushButton_3->setVisible(true);
	ui.pushButton_5->setVisible(false);
	ui.tableWidget->setVisible(true);
	ui.tableView_2->setVisible(false);

	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	if (faulttable_data.faultinfo_vector.size() == 0)
	{
		TableWrite("��ǰû�й����룡", 4);
	}

	current_fault_data = faulttable_data;
	for (int i = 0; i < ui.tableWidget->columnCount(); i++)
	{
		ui.tableWidget->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}

	ui.tableWidget->setRowCount(faulttable_data.faultinfo_vector.size());
	for (int i = 0; i < faulttable_data.faultinfo_vector.size(); i++)
	{
		FaultInfo temp_faultinfo = faulttable_data.faultinfo_vector[i];
		QTableWidgetItem* item1 = new QTableWidgetItem(temp_faultinfo.dtc_code);
		QTableWidgetItem* item2 = new QTableWidgetItem(temp_faultinfo.fault_name);
		QTableWidgetItem* item4 = new QTableWidgetItem(temp_faultinfo.time_state);
		QTableWidgetItem* item5 = new QTableWidgetItem(temp_faultinfo.fault_reasion);
		QTableWidgetItem* item6 = new QTableWidgetItem(temp_faultinfo.repair_info);

		ui.tableWidget->setItem(i, 0, item1);
		ui.tableWidget->setItem(i, 1, item2);

		QPushButton *shotcut_pushbutton = new QPushButton();
		shotcut_pushbutton->setToolTip(QStringLiteral("����"));
		shotcut_pushbutton->setFixedWidth(24);
		shotcut_pushbutton->setFixedHeight(24);
		shotcut_pushbutton->setStyleSheet("\
									QPushButton{\
									background:none;\
									border:none;\
									border-image:url(./image/btn_shoucut_normal.png);\
									}\
									QPushButton::hover{\
									background:none;\
									border:none;\
									border-image:url(./image/btn_shoucut_hover.png);\
									}\
									QPushButton::pressed{\
									border-style: inset;\
									background-color:#f6f5ec;\
									}");

		ui.tableWidget->setCellWidget(i, 2, shotcut_pushbutton);

		ui.tableWidget->setItem(i, 3, item4);
		ui.tableWidget->setItem(i, 4, item5);
		ui.tableWidget->setItem(i, 5, item6);

		connect(shotcut_pushbutton, SIGNAL(clicked()), this, SLOT(shotcut_pushbutton_click_Slot()));// ���ź�
		
	}
}

void DealFault::BackToECUFault()
{
	ui.tableWidget->clear();
	ui.tableWidget->setColumnCount(6);

	QStringList tableheader2;
	tableheader2 << QStringLiteral("���ϴ���") << QStringLiteral("����") << QStringLiteral("����") << QStringLiteral("״̬") << QStringLiteral("����ԭ��") << QStringLiteral("ά��ָ��");
	ui.tableWidget->setHorizontalHeaderLabels(tableheader2);

	ui.pushButton->setVisible(true);
	ui.pushButton_2->setVisible(true);
	ui.pushButton_3->setVisible(true);
	ui.pushButton_5->setVisible(false);
	ui.tableWidget->setVisible(true);
	ui.tableView_2->setVisible(false);


	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	if (current_fault_data.faultinfo_vector.size() == 0)
	{
		TableWrite("��ǰû�й����룡", 4);
	}

	for (int i = 0; i < ui.tableWidget->columnCount(); i++)
	{
		ui.tableWidget->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}

	ui.tableWidget->setRowCount(current_fault_data.faultinfo_vector.size());
	for (int i = 0; i < current_fault_data.faultinfo_vector.size(); i++)
	{
		FaultInfo temp_faultinfo = current_fault_data.faultinfo_vector[i];
		QTableWidgetItem* item1 = new QTableWidgetItem(temp_faultinfo.dtc_code);
		QTableWidgetItem* item2 = new QTableWidgetItem(temp_faultinfo.fault_name);
		QTableWidgetItem* item4 = new QTableWidgetItem(temp_faultinfo.time_state);
		QTableWidgetItem* item5 = new QTableWidgetItem(temp_faultinfo.fault_reasion);
		QTableWidgetItem* item6 = new QTableWidgetItem(temp_faultinfo.repair_info);
		ui.tableWidget->setItem(i, 0, item1);
		ui.tableWidget->setItem(i, 1, item2);

		QPushButton *shotcut_pushbutton = new QPushButton();
		shotcut_pushbutton->setToolTip(QStringLiteral("����"));
		shotcut_pushbutton->setFixedWidth(24);
		shotcut_pushbutton->setFixedHeight(24);
		shotcut_pushbutton->setStyleSheet("\
									QPushButton{\
									background:none;\
									border:none;\
									border-image:url(./image/btn_shoucut_normal.png);\
									}\
									QPushButton::hover{\
									background:none;\
									border:none;\
									border-image:url(./image/btn_shoucut_hover.png);\
									}\
									QPushButton::pressed{\
									border-style: inset;\
									background-color:#f6f5ec;\
									}");
		ui.tableWidget->setCellWidget(i, 2, shotcut_pushbutton);

		ui.tableWidget->setItem(i, 3, item4);
		ui.tableWidget->setItem(i, 4, item5);
		ui.tableWidget->setItem(i, 5, item6);

		connect(shotcut_pushbutton, SIGNAL(clicked()), this, SLOT(shotcut_pushbutton_click_Slot()));// ���ź�

	}
}


void DealFault::send_snapshot_result_Slot(SnapShotInfo snapshottable_data)
{
	table2->clear();
	table2->setColumnCount(2);

	QStringList tableheader2;
	tableheader2 << QStringLiteral("����") << QStringLiteral("ֵ");
	table2->setHorizontalHeaderLabels(tableheader2);

	ui.pushButton->setVisible(false);
	ui.pushButton_2->setVisible(false);
	ui.pushButton_3->setVisible(false);
	ui.pushButton_5->setVisible(true);
	ui.tableWidget->setVisible(false);
	ui.tableView_2->setVisible(true);

	for (int i = 0; i < table2->columnCount(); i++)
	{
		table2->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}

	for (int i = 0; i < snapshottable_data.SnapshotRecord_vector.size(); i++)
	{
		QVector<SnapshotLib> temp_snapshotlib_vector = snapshottable_data.SnapshotRecord_vector[i].SnapshotLib_vector;
		for (int j = 0; j < temp_snapshotlib_vector.size(); j++)
		{
			QMap<QString, QString> temp_map = temp_snapshotlib_vector[j].SnapshotDID_Data_map;
			QMap<QString, QString>::iterator  mi;
			for (mi = temp_map.begin(); mi != temp_map.end(); ++mi)
			{
				QStandardItem* item1 = new QStandardItem(mi.key());
				QStandardItem* item2 = new QStandardItem(mi.value());
				QList<QStandardItem*> row_data;
				row_data << item1 << item2;
				table2->appendRow(row_data);
			}
		}
	}
}

void DealFault::send_faultsort_result_Slot(FaultSortTableData faultsorttable_data)
{
	table2->clear();
	table2->setColumnCount(5);

	QStringList tableheader2;
	tableheader2 << QStringLiteral("���ϴ���") << QStringLiteral("����") << QStringLiteral("״̬") << QStringLiteral("���ϴ���") << QStringLiteral("����˳��");
	table2->setHorizontalHeaderLabels(tableheader2);

	ui.pushButton->setVisible(true);
	ui.pushButton_2->setVisible(true);//��������Ĭ�����أ�ֱ����ȡ���Ϻ͹����������
	ui.pushButton_3->setVisible(true);
	ui.pushButton_5->setVisible(true);
	ui.tableWidget->setVisible(false);
	ui.tableView_2->setVisible(true);

	for (int i = 0; i < table2->columnCount(); i++)
	{
		table2->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	

	if (faultsorttable_data.faultsort_vector.size() == 0)
	{
		TableWrite("��ǰû�й����룡", 4);
	}

	for (int i = 0; i < faultsorttable_data.faultsort_vector.size(); i++)
	{
		for (int j = 0; j < faultsorttable_data.faultsort_vector.size() - i; j++)
		{
			if (j + 1 < faultsorttable_data.faultsort_vector.size())
			{
				if (faultsorttable_data.faultsort_vector[j].fault_order > faultsorttable_data.faultsort_vector[j + 1].fault_order)
				{
					FaultSort temp_faultsort = faultsorttable_data.faultsort_vector[j];
					faultsorttable_data.faultsort_vector[j] = faultsorttable_data.faultsort_vector[j + 1];
					faultsorttable_data.faultsort_vector[j + 1] = temp_faultsort;
				}
			}
		}
	}

	for (int i = 0; i < faultsorttable_data.faultsort_vector.size(); i++)
	{
		FaultSort temp_faultsort = faultsorttable_data.faultsort_vector[i];
		QStandardItem* item1 = new QStandardItem(temp_faultsort.dtc_code);
		QStandardItem* item2 = new QStandardItem(temp_faultsort.fault_name);
		QStandardItem* item3 = new QStandardItem(temp_faultsort.time_state);
		QStandardItem* item4 = new QStandardItem(QString::number(temp_faultsort.fault_times));
		QStandardItem* item5 = new QStandardItem(QString::number(temp_faultsort.fault_order));

		QList<QStandardItem*> row_data;
		row_data << item1 << item2 << item3 << item4 << item5;
		table2->appendRow(row_data);
	}
}

void DealFault::send_clearfault_result_Slot(bool temp_flag)
{
	if (temp_flag)
	{
		TableWrite("���������ɹ���", 4);
	}
	else TableWrite("���������ʧ�ܣ�", 5);
}

void DealFault::deal_send_receive()
{
	bool loading_flag = false;
	int re_send_item = 0;
	bool re_receive_flag = false;
	QMap<QString, QMap<QString, QString> > read_ECU_info_vector;
	LoaderData current_command = this_command;
	SendData current_senddata = current_command.send_data;
	QVector<BYTE> receive_byte_vector;
	int no_receive_count = 0;
	current_thread_order++;
	data_thread_mutex.lock();
	while (CAN_open)
	{
		if (!loading_flag)
		{
			loading_flag = true;
			emit start_loading_Signal();
		}
		
		if (!re_receive_flag)current_senddata = current_command.send_data;
		SendData temp_senddata = current_command.send_data;
		ReceiveDiagloseData temp_receive_settingdata = current_command.receive_data;
		send_command_data(temp_senddata);//����ָ��
		Get_Record();//��ȡ���ͽ�������֡
		if (temp_receive_settingdata.frame_id != 0)//��������֡��Ϊ����ʾ���ղ�Ϊ�գ���Ҫ�����Ӧ����֡������
		{
			QVector<BYTE> temp_receive_byte_vector = receive_command_data(current_senddata, temp_receive_settingdata);//��ý���֡�������ֽڣ�ע���֡���շ�2�ν���
			if (temp_receive_byte_vector.size() == 0)//û�н��յ�����
			{
				if (no_receive_count >= 50)//������50�ζ�û���յ�����˵���������⣬Ӧ��ֹͣˢ��
				{
					TableWrite("���ݽ����쳣���������豸��", 5);
					no_receive_count = 0;
					break;
				}
				if (receive_timeout->elapsed() < 200)//ʱ��С��100ms����������
				{
					current_command.send_data.frame_id = 0;//����֡IDΪ0��������ִ�к��÷��ͣ�ֱ�ӽ���
					re_receive_flag = true;
					continue;
				}
				else if (re_send_item < 2)//��ʱ����100ms��������С��3��ʱ�����ظ�����ִ�д�������
				{
					current_command = this_command;
					re_send_item++;//�ظ�����+1
					no_receive_count++;
					re_receive_flag = false;
					continue;
				}
				else if (re_send_item == 2)//��������ʧ��ʱ��������һ������
				{
					re_send_item = 0;
					no_receive_count++;
					re_receive_flag = false;
					TableWrite("�����η��ͽ���ʧ�ܣ�����ָ��ʧ�ܣ�", 2);
					current_command = this_command;
					continue;
				}
			}
			else
			{
				re_receive_flag = false;
				no_receive_count = 0;
				if ((temp_receive_byte_vector[0] & 0xF0) != 0x10 && (current_command.send_data.hex_data[0] & 0xF0) != 0x30 && temp_receive_byte_vector.size() == 8)//��ͨ��֡����
				{
					receive_byte_vector.clear();
					temp_receive_byte_vector = temp_receive_byte_vector.mid(1, temp_receive_byte_vector[0]);//ֻȡ��Ч���ݣ���һ���ֽڱ�ʾ������Ч�ֽڸ���
					receive_byte_vector.append(temp_receive_byte_vector);
					if (DealReceiveData(temp_receive_settingdata, receive_byte_vector))
					{
						FaultTableData temp_faulttable_data;
						FaultSortTableData temp_faultsorttable_data;
						int result_flag = -1;
						for (int m = 0; m < temp_receive_settingdata.litte_record_vertor.size(); m++)//�������ж��Ƿ��ǹ��������û�й��ϵ�����£�����������ص�֡
						{
							LitteRecord temp_litterecord = temp_receive_settingdata.litte_record_vertor[m];
							if (temp_litterecord.data_content.indexOf("FaultCode") > 0)
							{
								QVector<FaultInfo> temp_faultinfo_vector = AnalyseFaultInfo(receive_byte_vector, temp_litterecord);
								temp_faulttable_data.faultinfo_vector = temp_faultinfo_vector;
								TableWrite("��ȡ������ɹ���", 4);
								emit send_fault_result_Signal(temp_faulttable_data);
								receive_byte_vector.clear();
								result_flag = 1;
								break;
							}
							if (temp_litterecord.data_content.indexOf("FaultSort") > 0)
							{
								QVector<FaultSort> temp_faultsort_vector = AnalyseFaultSort(receive_byte_vector, temp_litterecord);
								temp_faultsorttable_data.faultsort_vector = temp_faultsort_vector;
								TableWrite("����������ɹ���", 4);
								emit send_faultsort_result_Signal(temp_faultsorttable_data);
								receive_byte_vector.clear();
								result_flag = 1;
								break;
							}
						}
						if (result_flag == 1)
						{
							break;
						}
						else
						{
							TableWrite("δ�ҵ��������ݣ�", 5);
							receive_byte_vector.clear();
							break;
						}
					}
					else
					{
						TableWrite("�������ݴ���", 5);
						receive_byte_vector.clear();
						break;
					}
				}
				else if ((current_command.send_data.hex_data[0] & 0xF0) == 0x30)//��֡�У�0x30����֮֡��Ľ�������֡
				{
					receive_byte_vector.append(temp_receive_byte_vector);
					if (DealReceiveData(temp_receive_settingdata, receive_byte_vector))
					{
						FaultTableData temp_faulttable_data;
						SnapShotInfo temp_snapshot_data;
						FaultSortTableData temp_faultsorttable_data;
						int result_flag = -1;
						for (int m = 0; m < temp_receive_settingdata.litte_record_vertor.size(); m++)
						{
							LitteRecord temp_litterecord = temp_receive_settingdata.litte_record_vertor[m];
							if (temp_litterecord.data_content.indexOf("FaultCode") > 0)
							{
								QVector<FaultInfo> temp_faultinfo_vector = AnalyseFaultInfo(receive_byte_vector, temp_litterecord);
								temp_faulttable_data.faultinfo_vector = temp_faultinfo_vector;
								TableWrite("��ȡ������ɹ���", 4);
								emit send_fault_result_Signal(temp_faulttable_data);
								receive_byte_vector.clear();
								result_flag = 1;
								break;
							}
							if (temp_litterecord.data_content.indexOf("Snapshot") > 0)
							{
								QVector<SnapshotRecord> temp_snapshotinfo_vector = AnalyseSnapShotInfo(receive_byte_vector, temp_litterecord);
								temp_snapshot_data.SnapshotRecord_vector = temp_snapshotinfo_vector;
								TableWrite("��ȡ���ճɹ���", 4);
								emit send_snapshot_result_Signal(temp_snapshot_data);
								receive_byte_vector.clear();
								result_flag = 1;
								break;
							}
							if (temp_litterecord.data_content.indexOf("FaultSort") > 0)
							{
								QVector<FaultSort> temp_faultsort_vector = AnalyseFaultSort(receive_byte_vector, temp_litterecord);
								temp_faultsorttable_data.faultsort_vector = temp_faultsort_vector;
								TableWrite("����������ɹ���", 4);
								emit send_faultsort_result_Signal(temp_faultsorttable_data);
								receive_byte_vector.clear();
								result_flag = 1;
								break;
							}
						}
						if (result_flag == 1)
						{
							break;
						}
						else
						{
							TableWrite("��ȡ������/����ʧ�ܣ�", 5);
							break;
						}
					}
					else
					{
						TableWrite("����ָ�ƥ�䣡", 2);
						receive_byte_vector.clear();
						break;
					}
				}
				else if ((temp_receive_byte_vector[0] & 0xF0) == 0x10 && temp_receive_byte_vector.size() == 8)//0x10����֡������֮����뷢��0x30֡
				{
					receive_byte_vector.clear();
					temp_receive_byte_vector.remove(0, 2);//ȡ��6���ֽ�Ϊ��Ч�ֽ�
					receive_byte_vector.append(temp_receive_byte_vector);
					LoaderData tempdata1 = current_command;
					re_receive_flag = false;
					tempdata1.send_data.byte_length = 8;
					tempdata1.send_data.data_analyse_flag = false;
					tempdata1.send_data.data_content = "";
					tempdata1.send_data.single_muti_frame_flag = 0;
					tempdata1.send_data.hex_data.clear();
					tempdata1.send_data.hex_data.push_back(0x30);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					current_command = tempdata1;
					continue;
				}
			}
		}
	}
	current_thread_order--;
	heart_thread_condition.notify_one();
	data_thread_mutex.unlock();
	emit close_loading_Signal();
}


void DealFault::clear_fault()
{
	int re_send_item = 0;
	bool re_receive_flag = false;
	QMap<QString, QMap<QString, QString> > read_ECU_info_vector;
	LoaderData current_command = this_command;
	SendData current_senddata = current_command.send_data;
	QVector<BYTE> receive_byte_vector;
	int no_receive_count = 0;
	current_thread_order++;
	data_thread_mutex.lock();
	while (CAN_open)
	{
		if (!re_receive_flag)current_senddata = current_command.send_data;
		SendData temp_senddata = current_command.send_data;
		ReceiveDiagloseData temp_receive_settingdata = current_command.receive_data;
		send_command_data(temp_senddata);//����ָ��
		Get_Record();//��ȡ���ͽ�������֡
		if (temp_receive_settingdata.frame_id != 0)//��������֡��Ϊ����ʾ���ղ�Ϊ�գ���Ҫ�����Ӧ����֡������
		{
			QVector<BYTE> temp_receive_byte_vector = receive_command_data(current_senddata, temp_receive_settingdata);//��ý���֡�������ֽڣ�ע���֡���շ�2�ν���
			if (temp_receive_byte_vector.size() == 0)//û�н��յ�����
			{
				if (no_receive_count >= 50)//������50�ζ�û���յ�����˵���������⣬Ӧ��ֹͣˢ��
				{
					TableWrite("���ݽ����쳣���������豸��", 5);
					no_receive_count = 0;
					break;
				}
				if (receive_timeout->elapsed() < 200)//ʱ��С��100ms����������
				{
					current_command.send_data.frame_id = 0;//����֡IDΪ0��������ִ�к��÷��ͣ�ֱ�ӽ���
					re_receive_flag = true;
					continue;
				}
				else if (re_send_item < 2)//��ʱ����100ms��������С��3��ʱ�����ظ�����ִ�д�������
				{
					current_command = this_command;
					re_send_item++;//�ظ�����+1
					no_receive_count++;
					re_receive_flag = false;
					continue;
				}
				else if (re_send_item == 2)//��������ʧ��ʱ��������һ������
				{
					re_send_item = 0;
					no_receive_count++;
					re_receive_flag = false;
					TableWrite("�����η��ͽ���ʧ�ܣ�����ָ��ʧ�ܣ�", 2);
					current_command = this_command;
					continue;
				}
			}
			else
			{
				re_receive_flag = false;
				no_receive_count = 0;
				if ((temp_receive_byte_vector[0] & 0xF0) != 0x10 && (current_command.send_data.hex_data[0] & 0xF0) != 0x30 && temp_receive_byte_vector.size() == 8)//��ͨ��֡����
				{
					receive_byte_vector.clear();
					temp_receive_byte_vector = temp_receive_byte_vector.mid(1, temp_receive_byte_vector[0]);//ֻȡ��Ч���ݣ���һ���ֽڱ�ʾ������Ч�ֽڸ���
					receive_byte_vector.append(temp_receive_byte_vector);
					if (DealReceiveData(temp_receive_settingdata, receive_byte_vector))
					{
						emit send_clearfault_result_Signal(true);
						receive_byte_vector.clear();
						break;
					}
					else
					{
						emit send_clearfault_result_Signal(false);
						receive_byte_vector.clear();
						break;
					}
				}
			}
		}
	}
	current_thread_order--;
	heart_thread_condition.notify_one();
	data_thread_mutex.unlock();
}

bool DealFault::DealReceiveData(ReceiveDiagloseData temp_receive_data, QVector<BYTE> receive_byte_vector)//���͹���
{
	int real_same_byte_count = 0;
	int same_byte_count = 0;
	for (int n = 0; n < temp_receive_data.litte_record_vertor.size(); n++)
	{
		LitteRecord temp_little_record = temp_receive_data.litte_record_vertor[n];
		if (!temp_little_record.data_analyse_flag)//��������֡�в���Ҫ����
		{
			for (int s = 0; s < temp_little_record.byte_length; s++)
			{
				if (receive_byte_vector[s + temp_little_record.data_offset] == temp_little_record.hex_data[s])
				{
					same_byte_count++;//����������ļ�����ͬ���ֽ���
				}
			}
			real_same_byte_count += temp_little_record.byte_length;
		}
	}
	if (same_byte_count != real_same_byte_count)//�����������ļ������ֽ����Ƿ���ͬ
	{
		return false;
	}
	else
	{
		QVector<BYTE> temp_byte_data;
		for (int n = 0; n < temp_receive_data.litte_record_vertor.size(); n++)
		{
			LitteRecord temp_little_record = temp_receive_data.litte_record_vertor[n];
			if (temp_little_record.data_analyse_flag)//����֡��Ҫ����
			{
				for (int s = 0; s < temp_little_record.byte_length; s++)
				{

					temp_byte_data.push_back(receive_byte_vector[s + temp_little_record.data_offset]);
				}
				QString analyse_name = temp_little_record.data_content.mid(1, temp_little_record.data_content.length() - 2);
				receive_map_data.insert(analyse_name, temp_byte_data);//������������ݺ�����
			}
		}
		return true;
	}
}

void DealFault::send_command_data(SendData send_data)//���͹���
{
	QVector<VCI_CAN_OBJ> temp_senddata_vector;
	if (send_data.frame_id != 0)//����֡IDΪ0���򲻽��з��ͣ�ֱ����������
	{
		if (send_data.single_muti_frame_flag == 0)//��֡����
		{
			VCI_CAN_OBJ temp_frame;
			temp_frame.ID = send_data.frame_id;
			temp_frame.SendType = 0;// �������� 
			temp_frame.RemoteFlag = 0;// ����֡ 
			if (temp_frame.ID > 0x7ff)//�ж��Ƿ�����չ֡
			{
				temp_frame.ExternFlag = 1;//��չ֡
			}
			else temp_frame.ExternFlag = 0;//����֡

			temp_frame.DataLen = 8;// ���ݳ���1���ֽ� 

			if (!send_data.data_analyse_flag)//֡����Ҫ����
			{
				if (send_data.byte_length == 8)
				{
					for (int j = 0; j < 8; j++)
					{
						temp_frame.Data[j] = send_data.hex_data[j];
					}
				}
			}
			temp_senddata_vector.push_back(temp_frame);
		}
		else//��֡����
		{
			QVector<BYTE> temp_byte_vector = send_data.hex_data;
			if (temp_byte_vector.size() % 8 == 0)
			{
				for (int j = 0; j < temp_byte_vector.size() / 8; j++)
				{
					VCI_CAN_OBJ temp_frame;
					temp_frame.ID = send_data.frame_id;
					temp_frame.SendType = 0;// �������� 
					temp_frame.RemoteFlag = 0;// ����֡ 
					if (temp_frame.ID > 0x7ff)
					{
						temp_frame.ExternFlag = 1;
					}
					else temp_frame.ExternFlag = 0;
					temp_frame.DataLen = 8;// ���ݳ���1���ֽ� 
					temp_frame.Data[0] = temp_byte_vector[j * 8 + 0];
					temp_frame.Data[1] = temp_byte_vector[j * 8 + 1];
					temp_frame.Data[2] = temp_byte_vector[j * 8 + 2];
					temp_frame.Data[3] = temp_byte_vector[j * 8 + 3];
					temp_frame.Data[4] = temp_byte_vector[j * 8 + 4];
					temp_frame.Data[5] = temp_byte_vector[j * 8 + 5];
					temp_frame.Data[6] = temp_byte_vector[j * 8 + 6];
					temp_frame.Data[7] = temp_byte_vector[j * 8 + 7];

					temp_senddata_vector.push_back(temp_frame);
				}
			}
		}
		send_queue_mutex.lock();//���������ݼ��뷢�Ͷ���
		send_can_data_queue.enqueue(temp_senddata_vector);
		send_queue_mutex.unlock();
		receive_timeout->restart();//��ʱ��ʼ
		QThread::msleep(10);
	}
}

QVector<BYTE> DealFault::receive_command_data(SendData send_data, ReceiveDiagloseData temp_receive_data)//���չ���
{
	QVector<BYTE> temp_byte_vector;
	int find_send_frame_index = -1;
	for (int m = 0; m < Fault_Record_Vector.size(); m++)//�ڻ�ȡ�ķ��ͽ�������֡�У�����ɸѡ
	{
		Record temp_can_record = Fault_Record_Vector[m];
		int same_byte_count = 0;
		if (temp_can_record.this_frame.ID == send_data.frame_id && temp_can_record.this_frame.DataLen == send_data.byte_length)//���ҷ���֡�Ƿ����
		{
			for (int n = 0; n < temp_can_record.this_frame.DataLen; n++)//���͵�ÿ���ֽ�Ҫ��ͬ
			{
				if (temp_can_record.this_frame.Data[n] == send_data.hex_data[n])same_byte_count++;
			}
			if (same_byte_count == temp_can_record.this_frame.DataLen)//�ҵ�����֡
			{
				find_send_frame_index = m;
				break;
			}
		}
	}
	if (find_send_frame_index >= 0)//�ҵ�����֡
	{
		int muti_frame_flag = 0;
		for (int m = find_send_frame_index; m < Fault_Record_Vector.size(); m++)//�ӷ���֡λ�ú�ʼ���ҽ���֡
		{
			if (Fault_Record_Vector[m].this_frame.ID == current_page_block.receive_frame_id)//�ڽ�������֡�в���ָ��ID
			{
				if ((Fault_Record_Vector[m].this_frame.Data[0] & 0xF0) == 0x10)//��֡����֡
				{
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(Fault_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_number = Fault_Record_Vector[m].this_frame.Data[1] + (Fault_Record_Vector[m].this_frame.Data[0] & 0x0F) * 256;//��֡���ֽ���
					muti_frame_byte_counter = 6;//��ǰ֡�а���6����֡��Ч�ֽ�
					Fault_Record_Vector.remove(0, m + 1);//��շ��ͽ���֡����
					break;
				}
				else if ((send_data.hex_data[0] & 0xF0) == 0x30 &&
					(Fault_Record_Vector[m].this_frame.Data[0] >= 0x20
					&& Fault_Record_Vector[m].this_frame.Data[0] <= 0x2f))//��֡�ģ�0x30����֡��Ӧ�Ľ���֡
				{
					for (int j = 1; j < 8; j++)
					{
						temp_byte_vector.push_back(Fault_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_counter += 7;//ÿ֡����7����Ч�ֽڣ����ֽڲ���
					muti_frame_flag++;//����0x30����֡��Ӧ�Ľ���֡
					if (muti_frame_byte_counter >= muti_frame_byte_number)//�ж϶�֡�Ƿ������ɣ�������Ч�ֽ��ж�
					{
						Fault_Record_Vector.remove(0, m + 1);//��շ��ͽ���֡����
						break;
					}
				}
				else//��ͨ����֡
				{
					muti_frame_byte_number = 0;
					muti_frame_byte_counter = 0;
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(Fault_Record_Vector[m].this_frame.Data[j]);
					}
					Fault_Record_Vector.remove(0, m + 1);//��շ��ͽ���֡����
					break;
				}
			}
		}
		if (muti_frame_flag > 0)//����0x30����֡��Ӧ�Ľ���֡
		{
			if (muti_frame_byte_counter < muti_frame_byte_number)//����ﵽ��Ч�ֽڵ��������ܷ��أ��������
			{
				muti_frame_byte_counter = 6;
				temp_byte_vector.clear();
			}
			else
			{
				temp_byte_vector.remove(muti_frame_byte_number - 6, temp_byte_vector.size() - muti_frame_byte_number + 6);
			}
		}
		if (temp_byte_vector.size() > 0)//���ؽ����ֽ�
		{
			TableWrite("�������ݽ�����ϣ�����ָ����ɣ�", 3);
		}
	}
	return temp_byte_vector;
}

void DealFault::Get_Record()//��ȡ���ͽ�������֡���������֡ID����
{
	record_mutex.lock();
	for (int i = 0; i < CAN_Record.size(); i++)
	{
		Record temp_record = CAN_Record[i];
		if (current_page_block.send_frame_id != 0 && temp_record.this_frame.ID == current_page_block.send_frame_id && temp_record.frame_index > max_index)//�������֡ID�������������ظ���ȡ
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			Fault_Record_Vector.push_back(temp_record);
		}
		if (current_page_block.receive_frame_id != 0 && temp_record.this_frame.ID == current_page_block.receive_frame_id && temp_record.frame_index > max_index)//�������֡ID�������������ظ���ȡ
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			Fault_Record_Vector.push_back(temp_record);
		}
	}
	record_mutex.unlock();
}

QVector<FaultInfo> DealFault::AnalyseFaultInfo(QVector<BYTE> temp_data, LitteRecord this_record)
{
	QVector<FaultInfo> fault_code_vector;
	QFile file(current_page_block.settingfile);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return fault_code_vector;
	}
	if (this_record.data_content.indexOf("FaultCode") < 0)return fault_code_vector;
	int strat_index = 0;
	int fault_code_length = 0;
	QString analyse_data;
	QString circle_times;
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains("FaultCode"))
			{
				QJsonValue value1 = obj.take("FaultCode");
				QJsonObject object1 = value1.toObject();
				if (object1.contains("StartBYTE_index"))
				{
					QJsonValue value2 = object1.take("StartBYTE_index");
					if (value2.isString())
					{
						if (value2.toString().toInt() != this_record.data_offset)
						{
							return fault_code_vector;
						}
						else
						{
							strat_index = value2.toString().toInt();
						}
					}
					else
					{
						TableWrite("StartBYTE_indexֵ�����ַ�����", 5);
					}
				}
				if (object1.contains("Byte_Length"))
				{
					QJsonValue value3 = object1.take("Byte_Length");
					if (value3.isString())
					{
						if (value3.toString().toInt() > 0)
						{
							fault_code_length = value3.toString().toInt();
						}
					}
					else
					{
						TableWrite("Byte_Lengthֵ�����ַ�����", 5);
					}
				}
				if (object1.contains("Data"))
				{
					QJsonValue value4 = object1.take("Data");
					if (value4.isString())
					{
						analyse_data = value4.toString();
					}
					else
					{
						TableWrite("Dataֵ�����ַ�����", 5);
					}
				}
				if (object1.contains("CircleTimes"))
				{
					QJsonValue value5 = object1.take("CircleTimes");
					if (value5.isString())
					{
						QString temp_times = value5.toString();
						if (temp_times.indexOf('(') >= 0)
						{
							circle_times = temp_times.mid(1, temp_times.length() - 2);
						}
					}
					else
					{
						TableWrite("CircleTimesֵ�����ַ�����", 5);
					}
				}
				if (circle_times == "FaultCodeNum")
				{
					for (int i = strat_index; i < temp_data.size();)
					{
						FaultInfo temp_fault_info;
						QVector<BYTE> tempp_data;
						QRegExp rx1("(0x([0-9A-F]{2}))");
						int pos = 0;
						while (pos < analyse_data.length())
						{
							QRegExp rx2("([0-9]+\\|\\([A-Za-z0-9]+\\))");
							if (rx2.indexIn(analyse_data, pos) != -1)//��Ҫ�����Ĳ���
							{
								QString temp_special_data = rx2.cap(1);
								pos += rx2.matchedLength() + 1;

								QString temp_special_name;
								int temp_name_length = 0;
								QRegExp rx3("(\\([A-Za-z0-9]+\\))");
								if (rx3.indexIn(temp_special_data, 0) != -1)
								{
									temp_special_name = rx3.cap(1);
									temp_special_name = temp_special_name.mid(1, temp_special_name.length() - 2);
								}
								QRegExp rx4("([0-9]+\\|)");
								if (rx4.indexIn(temp_special_data, 0) != -1)
								{
									QString temp_length = rx4.cap(1);
									temp_length = temp_length.mid(0, temp_length.length() - 1);
									if (temp_length.toInt() > 0)
									{
										temp_name_length = temp_length.toInt();
									}
								}
								if (temp_special_name == "FaultCode")
								{
									QString code_value;
									int j = i;
									QVector<BYTE> temp_byte_vector;
									for (; j < i + temp_name_length && j < temp_data.size(); j++)
									{
										code_value += QString("%1").arg(temp_data[j], 2, 16, QLatin1Char('0'));
										temp_byte_vector.push_back(temp_data[j]);
									}
									code_value = code_value.toUpper();
									code_value = "0x" + code_value;

									i = j;
									temp_fault_info = deal_settingfile.GetFaultDetailInfo(code_value);
									temp_fault_info.code_vector = temp_byte_vector;
								}
								if (temp_special_name == "ByteAnalyseData")
								{
									if (temp_name_length == 1)
									{
										temp_fault_info.time_state = deal_settingfile.GetByteAnalyseTimeState(deal_settingfile.GetByteAnalyseID(), temp_data[i]);
										i++;
									}
								}
								if (temp_fault_info.time_state != "")
								{
									fault_code_vector.push_back(temp_fault_info);
								}
							}
						}
					}
				}
			}
			else
			{
				TableWrite("�ļ�������FaultCode���ԣ�", 5);
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return fault_code_vector;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return fault_code_vector;
	}
	return fault_code_vector;
}

QVector<FaultSort> DealFault::AnalyseFaultSort(QVector<BYTE> temp_data, LitteRecord this_record)
{
	QVector<FaultSort> fault_sort_vector;
	QFile file(current_page_block.settingfile);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return fault_sort_vector;
	}
	if (this_record.data_content.indexOf("FaultSort") < 0)return fault_sort_vector;
	int strat_index = 0;
	int fault_sort_length = 0;
	QString analyse_data;
	QString circle_times;
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains("FaultSort"))
			{
				QJsonValue value1 = obj.take("FaultSort");
				QJsonObject object1 = value1.toObject();
				if (object1.contains("StartBYTE_index"))
				{
					QJsonValue value2 = object1.take("StartBYTE_index");
					if (value2.isString())
					{
						if (value2.toString().toInt() != this_record.data_offset)
						{
							return fault_sort_vector;
						}
						else
						{
							strat_index = value2.toString().toInt();
						}
					}
					else
					{
						TableWrite("StartBYTE_indexֵ�����ַ�����", 5);
					}
				}
				if (object1.contains("Byte_Length"))
				{
					QJsonValue value3 = object1.take("Byte_Length");
					if (value3.isString())
					{
						if (value3.toString().toInt() > 0)
						{
							fault_sort_length = value3.toString().toInt();
						}
					}
					else
					{
						TableWrite("Byte_Lengthֵ�����ַ�����", 5);
					}
				}
				if (object1.contains("Data"))
				{
					QJsonValue value4 = object1.take("Data");
					if (value4.isString())
					{
						analyse_data = value4.toString();
					}
					else
					{
						TableWrite("Dataֵ�����ַ�����", 5);
					}
				}
				if (object1.contains("CircleTimes"))
				{
					QJsonValue value5 = object1.take("CircleTimes");
					if (value5.isString())
					{
						QString temp_times = value5.toString();
						if (temp_times.indexOf('(') >= 0)
						{
							circle_times = temp_times.mid(1, temp_times.length() - 2);
						}
					}
					else
					{
						TableWrite("CircleTimesֵ�����ַ�����", 5);
					}
				}
				if (circle_times == "FaultSortNum")
				{
					for (int i = strat_index; i < temp_data.size();)
					{
						FaultSort temp_fault_sort;
						QVector<BYTE> tempp_data;
						QRegExp rx1("(0x([0-9A-F]{2}))");
						int pos = 0;
						while (pos < analyse_data.length())
						{
							QRegExp rx2("([0-9]+\\|\\([A-Za-z0-9]+\\))");
							if (rx2.indexIn(analyse_data, pos) != -1)//��Ҫ�����Ĳ���
							{
								QString temp_special_data = rx2.cap(1);
								pos += rx2.matchedLength() + 1;

								QString temp_special_name;
								int temp_name_length = 0;
								QRegExp rx3("(\\([A-Za-z0-9]+\\))");
								if (rx3.indexIn(temp_special_data, 0) != -1)
								{
									temp_special_name = rx3.cap(1);
									temp_special_name = temp_special_name.mid(1, temp_special_name.length() - 2);
								}
								QRegExp rx4("([0-9]+\\|)");
								if (rx4.indexIn(temp_special_data, 0) != -1)
								{
									QString temp_length = rx4.cap(1);
									temp_length = temp_length.mid(0, temp_length.length() - 1);
									if (temp_length.toInt() > 0)
									{
										temp_name_length = temp_length.toInt();
									}
								}
								if (temp_special_name == "FaultCode")
								{
									QString code_value;
									int j = i;
									QVector<BYTE> temp_byte_vector;
									for (; j < i + temp_name_length && j < temp_data.size(); j++)
									{
										code_value += QString("%1").arg(temp_data[j], 2, 16, QLatin1Char('0'));
										temp_byte_vector.push_back(temp_data[j]);
									}
									code_value = code_value.toUpper();
									code_value = "0x" + code_value;

									i = j;
									FaultInfo temp_fault_info = deal_settingfile.GetFaultDetailInfo(code_value);
									temp_fault_sort.dtc_code = temp_fault_info.dtc_code;
									temp_fault_sort.fault_name = temp_fault_info.fault_name;
								}
								if (temp_special_name == "ByteAnalyseData")
								{
									if (temp_name_length == 1)
									{
										temp_fault_sort.time_state = deal_settingfile.GetByteAnalyseTimeState(deal_settingfile.GetByteAnalyseID(), temp_data[i]);
										i++;
									}
								}
								if (temp_special_name == "FaultTimes")
								{
									if (temp_name_length == 1)
									{
										temp_fault_sort.fault_times = int(temp_data[i]);
										i++;
									}
								}
								if (temp_special_name == "FaultOrder")
								{
									if (temp_name_length == 1)
									{
										temp_fault_sort.fault_order = int(temp_data[i]);
										i++;
									}
								}
								if (temp_fault_sort.fault_order != -1)
								{
									fault_sort_vector.push_back(temp_fault_sort);
								}
							}
						}
					}
				}
			}
			else
			{
				TableWrite("�ļ�������FaultSort���ԣ�", 5);
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return fault_sort_vector;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return fault_sort_vector;
	}
	return fault_sort_vector;
}

QVector<SnapshotRecord> DealFault::AnalyseSnapShotInfo(QVector<BYTE> temp_data, LitteRecord this_record)
{
	QVector<SnapshotRecord> SnapshotRecord_vector;
	QFile file(current_page_block.settingfile);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return SnapshotRecord_vector;
	}
	int strat_index = 0;
	QVector<LitteRecord> LitteRecord_vector;
	QString analyse_data;
	QString circle_times;
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains("Snapshot"))
			{
				QJsonValue value1 = obj.take("Snapshot");
				QJsonObject object1 = value1.toObject();
				if (object1.contains("StartBYTE_index"))
				{
					QJsonValue value2 = object1.take("StartBYTE_index");
					if (value2.isString())
					{
						if (value2.toString().toInt() != this_record.data_offset)
						{
							return SnapshotRecord_vector;
						}
						else
						{
							strat_index = value2.toString().toInt();
						}
					}
					else
					{
						TableWrite("StartBYTE_indexֵ�����ַ�����", 5);
					}
				}
				if (object1.contains("Data"))
				{
					QJsonValue value7 = object1.take("Data");
					if (value7.isArray())
					{
						QJsonArray array2 = value7.toArray();
						for (int j = 0; j < array2.size(); j++)
						{
							LitteRecord temp_little_record;
							QJsonValue value8 = array2.at(j);
							QJsonObject object3 = value8.toObject();
							if (object3.contains("Byte_Offset"))
							{
								QJsonValue value9 = object3.take("Byte_Offset");
								if (value9.isString())
								{
									int temp_offset = value9.toString().toInt();
									//if (temp_offset > 0)temp_little_record.data_offset = temp_offset;
									temp_little_record.data_offset = temp_offset;//�п���Ϊ0������Ϊ�˼�����̩E01�������û�й��ϴ��롢Snapshot_index��DID_num
								}
								else
								{
									TableWrite("Byte_Offsetֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������Byte_Offset���ԣ�", 5);
								break;
							}
							if (object3.contains("Byte_Number"))
							{
								QJsonValue value10 = object3.take("Byte_Number");
								if (value10.isString())
								{
									int temp_length = value10.toString().toInt();
									//if (temp_length > 0)temp_little_record.byte_length = temp_length;
									temp_little_record.byte_length = temp_length;//�п���Ϊ0������Ϊ�˼�����̩E01�������û�й��ϴ��롢Snapshot_index��DID_num
								}
								else
								{
									TableWrite("Byte_Numberֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������Byte_Number���ԣ�", 5);
								break;
							}
							if (object3.contains("Byte"))
							{
								QJsonValue value11 = object3.take("Byte");
								if (value11.isString())
								{
									int pos = value11.toString().indexOf("(");
									if (pos < 0)
									{
										temp_little_record.hex_data = Global::changeHexToByteVector(value11.toString());
									}
									else
									{
										QString temp_data = value11.toString();
										temp_little_record.data_content = temp_data.mid(1, temp_data.length() - 2);
										temp_little_record.data_analyse_flag = true;
									}
								}
								else
								{
									TableWrite("Byteֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������Byte���ԣ�", 5);
								break;
							}
							LitteRecord_vector.push_back(temp_little_record);
						}
					}
					else
					{
						TableWrite("Dataֵ�������飡", 5);
						return SnapshotRecord_vector;
					}
				}
				if (object1.contains("CircleTimes"))
				{
					QJsonValue value5 = object1.take("CircleTimes");
					if (value5.isString())
					{
						QString temp_times = value5.toString();
						if (temp_times.indexOf('(') >= 0)
						{
							circle_times = temp_times.mid(1, temp_times.length() - 2);
						}
					}
					else
					{
						TableWrite("CircleTimesֵ�����ַ�����", 5);
					}
				}
				if (circle_times == "SnapshotNum")
				{
					for (int i = strat_index; i < temp_data.size();)
					{
						SnapshotRecord temp_snapsot;
						for (int j = 0; j < LitteRecord_vector.size(); j++)
						{
							if (LitteRecord_vector[j].data_content == "Snapshot_index")
							{
								QVector<BYTE> temp_byte_vector = temp_data.mid(i + LitteRecord_vector[j].data_offset, LitteRecord_vector[j].byte_length);
								temp_snapsot.SnapshotRecord_index = Global::changeByteVectorToInt(temp_byte_vector);
							}

							if (LitteRecord_vector[j].data_content == "DID_num")
							{
								QVector<BYTE> temp_byte_vector = temp_data.mid(i + LitteRecord_vector[j].data_offset, LitteRecord_vector[j].byte_length);
								temp_snapsot.DID_num = Global::changeByteVectorToInt(temp_byte_vector);
							}
							if (LitteRecord_vector[j].data_content == "SnapshotDID")
							{
								i = i + LitteRecord_vector[j].data_offset;
								/*for (int m = 0; m < temp_snapsot.DID_num && i < temp_data.size(); m++)
								{

									SnapshotLib temp_lib = AnalyseSnapshotLib(temp_data.mid(i, temp_data.size() - i));
									if (temp_lib.SnapshotDID == "")
									{
										break;
									}
									else
									{
										i = i + temp_lib.total_byte_length;
										temp_snapsot.SnapshotLib_vector.push_back(temp_lib);
									}
								}*/
								while(i < temp_data.size())//����Ϊ�˼�����̩E01�������û�й��ϴ��롢Snapshot_index��DID_num��DID_numΪ0
								{

									SnapshotLib temp_lib = AnalyseSnapshotLib(temp_data.mid(i, temp_data.size() - i));
									if (temp_lib.SnapshotDID == "")
									{
										break;
									}
									else
									{
										i = i + temp_lib.total_byte_length;
										temp_snapsot.SnapshotLib_vector.push_back(temp_lib);
									}
								}
							}
						}
						if (temp_snapsot.SnapshotLib_vector.size() == 0)break;
						else SnapshotRecord_vector.push_back(temp_snapsot);
					}
				}
			}
			else
			{
				TableWrite("�ļ�������FaultCode���ԣ�", 5);
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return SnapshotRecord_vector;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return SnapshotRecord_vector;
	}
	return SnapshotRecord_vector;
}

SnapshotLib DealFault::AnalyseSnapshotLib(QVector<BYTE> temp_data)
{
	SnapshotLib temp_SnapshotLib;
	QFile file(current_page_block.settingfile);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return temp_SnapshotLib;
	}
	QVector<LitteRecord> LitteRecord_vector;
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains("SnapshotDID"))
			{
				QJsonValue value1 = obj.take("SnapshotDID");
				QJsonObject object1 = value1.toObject();

				if (object1.contains("Data"))
				{
					QJsonValue value7 = object1.take("Data");
					if (value7.isArray())
					{
						QJsonArray array2 = value7.toArray();
						for (int j = 0; j < array2.size(); j++)
						{
							LitteRecord temp_little_record;
							QJsonValue value8 = array2.at(j);
							QJsonObject object3 = value8.toObject();
							if (object3.contains("Byte_Offset"))
							{
								QJsonValue value9 = object3.take("Byte_Offset");
								if (value9.isString())
								{
									int temp_offset = value9.toString().toInt();
									if (temp_offset > 0)temp_little_record.data_offset = temp_offset;
								}
								else
								{
									TableWrite("Byte_Offsetֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������Byte_Offset���ԣ�", 5);
								break;
							}
							if (object3.contains("Byte_Number"))
							{
								QJsonValue value10 = object3.take("Byte_Number");
								if (value10.isString())
								{
									int temp_length = value10.toString().toInt();
									if (temp_length > 0)temp_little_record.byte_length = temp_length;
								}
								else
								{
									TableWrite("Byte_Numberֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������Byte_Number���ԣ�", 5);
								break;
							}
							if (object3.contains("Byte"))
							{
								QJsonValue value11 = object3.take("Byte");
								if (value11.isString())
								{
									int pos = value11.toString().indexOf("(");
									if (pos < 0)
									{
										temp_little_record.hex_data = Global::changeHexToByteVector(value11.toString());
									}
									else
									{
										QString temp_data = value11.toString();
										temp_little_record.data_content = temp_data.mid(1, temp_data.length() - 2);
										temp_little_record.data_analyse_flag = true;
									}
								}
								else
								{
									TableWrite("Byteֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������Byte���ԣ�", 5);
								break;
							}
							LitteRecord_vector.push_back(temp_little_record);
						}
					}
					else
					{
						TableWrite("Dataֵ�������飡", 5);
						return temp_SnapshotLib;
					}
				}
				for (int j = 0; j < LitteRecord_vector.size(); j++)
				{
					if (LitteRecord_vector[j].data_content == "SnapshotDID")
					{
						QVector<BYTE> temp_byte_vector = temp_data.mid(LitteRecord_vector[j].data_offset, LitteRecord_vector[j].byte_length);
						QString code_value;
						for (int m = 0; m < temp_byte_vector.size(); m++)
						{
							code_value += QString("%1").arg(temp_byte_vector[m], 2, 16, QLatin1Char('0'));
						}
						code_value = code_value.toUpper();
						code_value = "0x" + code_value;

						ReceiveDiagloseData temp_shot = deal_settingfile.AnalyseSnapshotLib("SnapshotLib", code_value);
						if (temp_shot.data_record_vertor.size() == 0)
						{
							TableWrite("��ȡ���������ļ�ʧ�ܣ�", 5);
							break;
						}
						else
						{
							temp_SnapshotLib.SnapshotDID = code_value;
							temp_SnapshotLib.total_byte_length = temp_shot.byte_length + 2;
							temp_SnapshotLib.SnapshotDID_Data_map = GetAnalyseReceiveDataResult(temp_shot, temp_data.mid(2, temp_data.size() -2));
						}
						return temp_SnapshotLib;
					}
				}
			}
			else
			{
				TableWrite("�ļ�������SnapshotDID���ԣ�", 5);
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_SnapshotLib;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_SnapshotLib;
	}
	return temp_SnapshotLib;
}

QMap<QString, QString> DealFault::GetAnalyseReceiveDataResult(ReceiveDiagloseData temp_data_setting, QVector<BYTE> receive_byte_vector)//���ݽ��ղ�������֡���н���
{
	QMap<QString, QString> temp_data_map;
	if (receive_byte_vector.size() == 0)return temp_data_map;

	QBitArray data_group(receive_byte_vector.size() * 8);//����λ����
	for (int m = 0; m < receive_byte_vector.size(); m++)
	{
		BYTE temp_char = receive_byte_vector[m];
		for (int n = 0; n < 8; n++)
		{
			bool temp = temp_char &(1 << (7 - n));
			data_group.setBit(m * 8 + n, temp_char &(1 << (7 - n)));//��λbit����ǰ�棬�±�С
		}
	}
	QString special_data_value;
	for (int i = 0; i < temp_data_setting.data_record_vertor.size(); i++)
	{
		ReceiveAnalyseData temp_analyse = temp_data_setting.data_record_vertor[i];
		if (data_group.size() < temp_analyse.bitlength)//��֤Ϊ���ȴﵽҪ�󣬷�������Ǵ���֡���߽��մ��󣬱������鳬����Χ
		{
			return temp_data_map;
		}
		qlonglong raw_value = 0;
		QBitArray sg_data_group(temp_analyse.bitlength);
		for (int m = 0; m < temp_analyse.bitlength; m++)//��ȡ�źŶ�Ӧ��bit����
		{
			sg_data_group.setBit(m, data_group.at(temp_analyse.startbit + m));
		}
		for (int m = 0; m < sg_data_group.size(); m++)//��ȡʵ��ֵ
		{
			raw_value += (sg_data_group.at(m) << (sg_data_group.size() - m - 1));
		}

		if (temp_analyse.invalid_value != -1)
		{
			if (raw_value == temp_analyse.invalid_value)
			{
				//temp_ecudata.ECUinfo_name = temp_analyse.caption_content;
				//temp_ecudata.ECUinfo_value = "Invalid";
				//temp_ecudata_vector.push_back(temp_ecudata);
				temp_data_map.insert(temp_analyse.caption_content, "Invalid");
				continue;
			}
		}

		switch (temp_analyse.Caculate_Type)//����Caculate_Type�ֱ���н���
		{
		case 0://state��λ��ʾ״̬
		{
			QMap<int, QString>::iterator  mi;
			for (mi = temp_analyse.value_content_map.begin(); mi != temp_analyse.value_content_map.end(); ++mi)
			{
				if (raw_value == mi.key())
				{
					QString temp_content = mi.value();
					temp_data_map.insert(temp_analyse.caption_content, temp_content);
					break;
				}
			}
			break;
		}
		case 1://Numerical����ʾ��ֵ
		{
			QString temp_value = AnalyseFormula(temp_analyse.Formula, (double)raw_value);
			if (temp_analyse.Format == "Date")
			{
				if (temp_value.length() == 1)temp_value = "0" + temp_value;
				special_data_value += temp_value;
			}
			else
			{
				if (temp_analyse.Unit == "")temp_data_map.insert(temp_analyse.caption_content, temp_value);
				else temp_data_map.insert(temp_analyse.caption_content, temp_value + "  " + temp_analyse.Unit);
			}
			break;
		}
		case 2://ASC2��ת����ASC2��
		{
			QString temp_value;
			for (int m = 0; m < receive_byte_vector.size() && m < temp_analyse.bitlength / 8; m++)
			{
				BYTE temp_char = receive_byte_vector[m];
				temp_value.append((uchar)temp_char);
			}
			temp_data_map.insert(temp_analyse.caption_content, temp_value);
			break;
		}
		case 3://16����
		{
			QString temp_value;
			for (int m = 0; m < receive_byte_vector.size() && m < temp_analyse.bitlength / 8; m++)
			{
				BYTE temp_char = receive_byte_vector[m];
				temp_value += QString("%1").arg((unsigned short)temp_char, 2, 16, QLatin1Char('0'));
			}
			temp_data_map.insert(temp_analyse.caption_content, temp_value);
			break;
		}
		default:
			break;
		}
	}
	if (special_data_value != "")//����������
	{
		temp_data_map.insert(temp_data_setting.data_record_vertor[0].caption_content, special_data_value);
	}
	return temp_data_map;
}

QString DealFault::AnalyseFormula(QString temp_formula, double temp_value)//��ʽ������Ҫ��+ - * /����
{
	if (temp_formula == "")return "";
	if (temp_formula.mid(0, 1) != "A")return "";
	QRegExp rx1("([\\+\\-\\*/]+)");
	int pos = 1;
	QString temp_unit1, temp_unit2;
	int pos1 = 0;
	int pos2 = 0;
	QString value1, value2;
	while ((pos = rx1.indexIn(temp_formula, pos)) != -1)
	{
		if (pos1 == 0)pos1 = pos;
		else pos2 = pos;
		if (temp_unit1 == "")temp_unit1 = rx1.cap(1);
		else temp_unit2 = rx1.cap(1);
		if (pos1 != 0 && pos2 != 0)
		{
			value1 = temp_formula.mid(pos1 + 1, pos2 - pos1 - 1);
		}
		pos += rx1.matchedLength();
	}
	int temp_pos = pos1 > pos2 ? pos1 : pos2;
	if (temp_pos + 1 < temp_formula.length())
	{
		if (value1 == "")value1 = temp_formula.mid(temp_pos + 1, temp_formula.length() - temp_pos - 1);
		else value2 = temp_formula.mid(temp_pos + 1, temp_formula.length() - temp_pos - 1);

		if (value1 != "")
		{
			if (temp_unit1 == "+")
			{
				temp_value = temp_value + value1.toDouble();
			}
			if (temp_unit1 == "-")
			{
				temp_value = temp_value - value1.toDouble();
			}
			if (temp_unit1 == "*")
			{
				temp_value = temp_value * value1.toDouble();
			}
			if (temp_unit1 == "/")
			{
				temp_value = temp_value / value1.toDouble();
			}
		}
		if (value2 != "")
		{
			if (temp_unit2 == "+")
			{
				temp_value = temp_value + value2.toDouble();
			}
			if (temp_unit2 == "-")
			{
				temp_value = temp_value - value2.toDouble();
			}
			if (temp_unit2 == "*")
			{
				temp_value = temp_value * value2.toDouble();
			}
			if (temp_unit2 == "/")
			{
				temp_value = temp_value / value2.toDouble();
			}
		}
	}
	if (fabs(temp_value - (int)temp_value) < 0.00000001)//�����ж��Ƿ�������
	{
		return QString::number((qlonglong)temp_value);
	}
	else return QString::number(temp_value, 'f', 2);
}

void DealFault::resizeEvent(QResizeEvent *event)//���ڴ�С�ı䣬table��СҲ���Ÿı�
{
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void DealFault::shotcut_pushbutton_click_Slot()// �󶨵���Ӧ����
{
	QPushButton *senderObj = qobject_cast<QPushButton*>(sender());
	if (senderObj == nullptr)
	{
		return;
	}
	QModelIndex index = ui.tableWidget->indexAt(QPoint(senderObj->frameGeometry().x(), senderObj->frameGeometry().y()));

	QVector<BYTE> temp_code_vector;
	if (current_fault_data.faultinfo_vector.size() > 0)
	{
		temp_code_vector = current_fault_data.faultinfo_vector[index.row()].code_vector;
	}
	QString temp_content;
	for (int i = 0; i < temp_code_vector.size(); i++)
	{
		QString content1 = QString("%1").arg(temp_code_vector[i], 2, 16, QLatin1Char('0'));
		content1 = content1.toUpper();
		content1 = "0x" + content1 + " ";
		temp_content += content1;
	}
	temp_content.chop(1);
	LoaderData temp_command = deal_settingfile.GetSingleCommandFromDiagnoseSettingFile("DtcInf", QStringLiteral("��������Ϣ"));

	if (temp_command.send_data.data_analyse_flag)
	{
		if (temp_command.send_data.data_content.indexOf("3|(FaultCode)") != -1)
		{
			temp_command.send_data.data_content.replace("3|(FaultCode)", temp_content);
			temp_command.send_data.hex_data = Global::changeHexToByteVector(temp_command.send_data.data_content);
			temp_command.send_data.data_analyse_flag = false;
			temp_command.send_data.data_content = "";
		}
	}
	if (temp_command.send_data.frame_id == 0)
	{
		TableWrite("�����ļ�����ʧ�ܣ�û�л���������ݣ�", 5);
	}
	else
	{
		this_command = temp_command;
		if (CAN_open)
		{
			TableWrite("��ʼ��ȡ���գ�", 1);
			startFaultThread();
		}
	}
}

void DealFault::doubleClicked_Slot(QModelIndex index)
{
	int t_w = ui.tableWidget->width();
	int scroll_w = ui.tableWidget->verticalScrollBar()->width();
	if (scroll_w != 100)//����ֱ������û������ʱ��������100�����Ե�������100ʱ���ǹ�����û������
	{
		int this_w = ui.tableWidget->horizontalHeader()->sectionSize(index.column());
		if ((this_w == (t_w - scroll_w) / ui.tableWidget->columnCount() + 1) ||
			(this_w == (t_w - scroll_w) / ui.tableWidget->columnCount() - 1) ||
			(this_w == (t_w - scroll_w) / ui.tableWidget->columnCount()))
		{
			ui.tableWidget->horizontalHeader()->setSectionResizeMode(index.column(), QHeaderView::ResizeToContents);
		}
		else
		{
			ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//������
		}
	}
	else
	{
		int this_w = ui.tableWidget->horizontalHeader()->sectionSize(index.column());
		if ((this_w == t_w / ui.tableWidget->columnCount() + 1) ||
			(this_w == t_w / ui.tableWidget->columnCount() - 1) ||
			(this_w == t_w / ui.tableWidget->columnCount()))
		{
			ui.tableWidget->horizontalHeader()->setSectionResizeMode(index.column(), QHeaderView::ResizeToContents);
		}
		else
		{
			ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//������
		}
	}
}

void DealFault::start_loading_Slot()
{
	thread_wait_timer->start(400);//���ﲻ������̫���ˣ�������ܳ��ֵȴ������Ѿ�������δ��ʾʱ����Ҫ�ر��ˣ������BUG
}

void DealFault::timeoutSlot()
{
	if (thread_wait_timer_count == 2)//�ȴ����ڳ��֣����ȴ�����2*150=300ms����ʾ�ȴ�����
	{
		temp_load_dialog = new LoadDialog;
		this->setEnabled(false);
		temp_load_dialog->show();
	}
	thread_wait_timer_count++;
}

void DealFault::close_loading_Slot()
{
	thread_wait_timer->stop();
	if (thread_wait_timer_count >= 2)//���ȴ����ڳ��ֺ�Ž��йر�
	{
		this->setEnabled(true);
		thread_wait_timer_count = 0;
		temp_load_dialog->close();
	}
}