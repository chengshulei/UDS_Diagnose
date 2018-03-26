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

	ui.pushButton->setText(QStringLiteral("读取故障"));
	ui.pushButton_2->setText(QStringLiteral("故障排序"));
	ui.pushButton_3->setText(QStringLiteral("清除故障"));
	ui.pushButton_5->setText(QStringLiteral("返回"));

	ui.tableWidget->clear();
	ui.tableWidget->setColumnCount(6);
	ui.tableWidget->setShowGrid(false);//无网格线
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
	ui.tableWidget->verticalHeader()->setVisible(false);
	ui.tableWidget->setItemDelegate(new NoFocusRectangleStyle());
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分列

	QStringList tableheader2;
	tableheader2 << QStringLiteral("故障代码") << QStringLiteral("名称") << QStringLiteral("快照") << QStringLiteral("状态") << QStringLiteral("故障原因") << QStringLiteral("维修指导");
	ui.tableWidget->setHorizontalHeaderLabels(tableheader2);
	ui.pushButton->setVisible(true);
	ui.pushButton_2->setVisible(true);//故障排序，默认隐藏，直到读取故障和故障排序独立
	ui.pushButton_3->setVisible(true);
	ui.pushButton_5->setVisible(false);
	ui.tableWidget->setVisible(true);
	ui.tableView_2->setVisible(false);

	table2 = new QStandardItemModel();
	ui.tableView_2->setModel(table2);
	//ui.tableView_2->horizontalHeader()->setStretchLastSection(true);//最后一列填满表格空白
	ui.tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分列
	ui.tableView_2->setShowGrid(false);//无网格线
	ui.tableView_2->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
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
	LoaderData temp_command = deal_settingfile.GetSingleCommandFromDiagnoseSettingFile("DtcInf", QStringLiteral("读ECU故障"));

	if (temp_command.send_data.frame_id == 0)
	{
		TableWrite("配置文件解析失败！没有获得命令数据！", 5);
	}
	else
	{
		this_command = temp_command;
		if (CAN_open)
		{
			TableWrite("开始读取故障码！", 1);
			startFaultThread();
		}
	}
}

void DealFault::ReadECUFaultSort()
{
	LoaderData temp_command = deal_settingfile.GetSingleCommandFromDiagnoseSettingFile("DtcInf", QStringLiteral("故障排序"));

	if (temp_command.send_data.frame_id == 0)
	{
		TableWrite("配置文件解析失败！没有获得命令数据！", 5);
	}
	else
	{
		this_command = temp_command;
		if (CAN_open)
		{
			TableWrite("开始读取故障排序！", 1);
			startFaultThread();
		}
	}
}

void DealFault::ClearECUFault()
{
	LoaderData temp_command = deal_settingfile.GetSingleCommandFromDiagnoseSettingFile("DtcInf", QStringLiteral("清除全部故障码"));
	if (temp_command.send_data.frame_id == 0)
	{
		TableWrite("配置文件解析失败！没有获得命令数据！", 5);
	}
	else
	{
		this_command = temp_command;
		if (CAN_open)
		{
			TableWrite("开始清除故障码！", 1);
			startClearFaultThread();
		}
	}
}

void DealFault::TableWrite(QString message_content, int message_type)//弹出警告信息窗口
{
	emit display_messageSignal(message_content, message_type);
}

void DealFault::startFaultThread()//启动线程
{
	boost::function0<void> f1 = boost::bind(&DealFault::deal_send_receive, this);
	fault_thread = boost::thread(f1);
}
void DealFault::startClearFaultThread()//启动线程
{
	boost::function0<void> f1 = boost::bind(&DealFault::clear_fault, this);
	clearfault_thread = boost::thread(f1);
}

void DealFault::send_fault_result_Slot(FaultTableData faulttable_data)
{
	ui.tableWidget->clear();
	ui.tableWidget->setColumnCount(6);

	QStringList tableheader2;
	tableheader2 << QStringLiteral("故障代码") << QStringLiteral("名称") << QStringLiteral("快照") << QStringLiteral("状态") << QStringLiteral("故障原因") << QStringLiteral("维修指导");
	ui.tableWidget->setHorizontalHeaderLabels(tableheader2);

	ui.pushButton->setVisible(true);
	ui.pushButton_2->setVisible(true);//故障排序，默认隐藏，直到读取故障和故障排序独立
	ui.pushButton_3->setVisible(true);
	ui.pushButton_5->setVisible(false);
	ui.tableWidget->setVisible(true);
	ui.tableView_2->setVisible(false);

	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	if (faulttable_data.faultinfo_vector.size() == 0)
	{
		TableWrite("当前没有故障码！", 4);
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
		shotcut_pushbutton->setToolTip(QStringLiteral("快照"));
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

		connect(shotcut_pushbutton, SIGNAL(clicked()), this, SLOT(shotcut_pushbutton_click_Slot()));// 绑定信号
		
	}
}

void DealFault::BackToECUFault()
{
	ui.tableWidget->clear();
	ui.tableWidget->setColumnCount(6);

	QStringList tableheader2;
	tableheader2 << QStringLiteral("故障代码") << QStringLiteral("名称") << QStringLiteral("快照") << QStringLiteral("状态") << QStringLiteral("故障原因") << QStringLiteral("维修指导");
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
		TableWrite("当前没有故障码！", 4);
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
		shotcut_pushbutton->setToolTip(QStringLiteral("快照"));
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

		connect(shotcut_pushbutton, SIGNAL(clicked()), this, SLOT(shotcut_pushbutton_click_Slot()));// 绑定信号

	}
}


void DealFault::send_snapshot_result_Slot(SnapShotInfo snapshottable_data)
{
	table2->clear();
	table2->setColumnCount(2);

	QStringList tableheader2;
	tableheader2 << QStringLiteral("名称") << QStringLiteral("值");
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
	tableheader2 << QStringLiteral("故障代码") << QStringLiteral("名称") << QStringLiteral("状态") << QStringLiteral("故障次数") << QStringLiteral("故障顺序");
	table2->setHorizontalHeaderLabels(tableheader2);

	ui.pushButton->setVisible(true);
	ui.pushButton_2->setVisible(true);//故障排序，默认隐藏，直到读取故障和故障排序独立
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
		TableWrite("当前没有故障码！", 4);
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
		TableWrite("清除错误码成功！", 4);
	}
	else TableWrite("清除错误码失败！", 5);
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
		send_command_data(temp_senddata);//发送指令
		Get_Record();//获取发送接收数据帧
		if (temp_receive_settingdata.frame_id != 0)//接收数据帧不为，表示接收不为空，需要获得相应数据帧并解析
		{
			QVector<BYTE> temp_receive_byte_vector = receive_command_data(current_senddata, temp_receive_settingdata);//获得接收帧中所有字节，注意多帧接收分2次进行
			if (temp_receive_byte_vector.size() == 0)//没有接收到数据
			{
				if (no_receive_count >= 50)//当连续50次都没有收到数据说明出了问题，应该停止刷新
				{
					TableWrite("数据接收异常！请重启设备！", 5);
					no_receive_count = 0;
					break;
				}
				if (receive_timeout->elapsed() < 200)//时间小于100ms，继续接收
				{
					current_command.send_data.frame_id = 0;//发送帧ID为0，即重新执行后不用发送，直接接收
					re_receive_flag = true;
					continue;
				}
				else if (re_send_item < 2)//计时超过100ms，当次数小于3次时进行重复重新执行此条命令
				{
					current_command = this_command;
					re_send_item++;//重复次数+1
					no_receive_count++;
					re_receive_flag = false;
					continue;
				}
				else if (re_send_item == 2)//当第三次失败时，进行下一条命令
				{
					re_send_item = 0;
					no_receive_count++;
					re_receive_flag = false;
					TableWrite("第三次发送接收失败，本条指令失败！", 2);
					current_command = this_command;
					continue;
				}
			}
			else
			{
				re_receive_flag = false;
				no_receive_count = 0;
				if ((temp_receive_byte_vector[0] & 0xF0) != 0x10 && (current_command.send_data.hex_data[0] & 0xF0) != 0x30 && temp_receive_byte_vector.size() == 8)//普通单帧接收
				{
					receive_byte_vector.clear();
					temp_receive_byte_vector = temp_receive_byte_vector.mid(1, temp_receive_byte_vector[0]);//只取有效数据，以一个字节表示后面有效字节个数
					receive_byte_vector.append(temp_receive_byte_vector);
					if (DealReceiveData(temp_receive_settingdata, receive_byte_vector))
					{
						FaultTableData temp_faulttable_data;
						FaultSortTableData temp_faultsorttable_data;
						int result_flag = -1;
						for (int m = 0; m < temp_receive_settingdata.litte_record_vertor.size(); m++)//这里是判断是否是故障码命令，没有故障的情况下，故障码命令返回单帧
						{
							LitteRecord temp_litterecord = temp_receive_settingdata.litte_record_vertor[m];
							if (temp_litterecord.data_content.indexOf("FaultCode") > 0)
							{
								QVector<FaultInfo> temp_faultinfo_vector = AnalyseFaultInfo(receive_byte_vector, temp_litterecord);
								temp_faulttable_data.faultinfo_vector = temp_faultinfo_vector;
								TableWrite("读取故障码成功！", 4);
								emit send_fault_result_Signal(temp_faulttable_data);
								receive_byte_vector.clear();
								result_flag = 1;
								break;
							}
							if (temp_litterecord.data_content.indexOf("FaultSort") > 0)
							{
								QVector<FaultSort> temp_faultsort_vector = AnalyseFaultSort(receive_byte_vector, temp_litterecord);
								temp_faultsorttable_data.faultsort_vector = temp_faultsort_vector;
								TableWrite("读故障排序成功！", 4);
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
							TableWrite("未找到接收数据！", 5);
							receive_byte_vector.clear();
							break;
						}
					}
					else
					{
						TableWrite("接收数据错误！", 5);
						receive_byte_vector.clear();
						break;
					}
				}
				else if ((current_command.send_data.hex_data[0] & 0xF0) == 0x30)//多帧中，0x30发送帧之后的接收数据帧
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
								TableWrite("读取故障码成功！", 4);
								emit send_fault_result_Signal(temp_faulttable_data);
								receive_byte_vector.clear();
								result_flag = 1;
								break;
							}
							if (temp_litterecord.data_content.indexOf("Snapshot") > 0)
							{
								QVector<SnapshotRecord> temp_snapshotinfo_vector = AnalyseSnapShotInfo(receive_byte_vector, temp_litterecord);
								temp_snapshot_data.SnapshotRecord_vector = temp_snapshotinfo_vector;
								TableWrite("读取快照成功！", 4);
								emit send_snapshot_result_Signal(temp_snapshot_data);
								receive_byte_vector.clear();
								result_flag = 1;
								break;
							}
							if (temp_litterecord.data_content.indexOf("FaultSort") > 0)
							{
								QVector<FaultSort> temp_faultsort_vector = AnalyseFaultSort(receive_byte_vector, temp_litterecord);
								temp_faultsorttable_data.faultsort_vector = temp_faultsort_vector;
								TableWrite("读故障排序成功！", 4);
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
							TableWrite("读取故障码/快照失败！", 5);
							break;
						}
					}
					else
					{
						TableWrite("接收指令不匹配！", 2);
						receive_byte_vector.clear();
						break;
					}
				}
				else if ((temp_receive_byte_vector[0] & 0xF0) == 0x10 && temp_receive_byte_vector.size() == 8)//0x10接收帧，接收之后必须发送0x30帧
				{
					receive_byte_vector.clear();
					temp_receive_byte_vector.remove(0, 2);//取后6个字节为有效字节
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
		send_command_data(temp_senddata);//发送指令
		Get_Record();//获取发送接收数据帧
		if (temp_receive_settingdata.frame_id != 0)//接收数据帧不为，表示接收不为空，需要获得相应数据帧并解析
		{
			QVector<BYTE> temp_receive_byte_vector = receive_command_data(current_senddata, temp_receive_settingdata);//获得接收帧中所有字节，注意多帧接收分2次进行
			if (temp_receive_byte_vector.size() == 0)//没有接收到数据
			{
				if (no_receive_count >= 50)//当连续50次都没有收到数据说明出了问题，应该停止刷新
				{
					TableWrite("数据接收异常！请重启设备！", 5);
					no_receive_count = 0;
					break;
				}
				if (receive_timeout->elapsed() < 200)//时间小于100ms，继续接收
				{
					current_command.send_data.frame_id = 0;//发送帧ID为0，即重新执行后不用发送，直接接收
					re_receive_flag = true;
					continue;
				}
				else if (re_send_item < 2)//计时超过100ms，当次数小于3次时进行重复重新执行此条命令
				{
					current_command = this_command;
					re_send_item++;//重复次数+1
					no_receive_count++;
					re_receive_flag = false;
					continue;
				}
				else if (re_send_item == 2)//当第三次失败时，进行下一条命令
				{
					re_send_item = 0;
					no_receive_count++;
					re_receive_flag = false;
					TableWrite("第三次发送接收失败，本条指令失败！", 2);
					current_command = this_command;
					continue;
				}
			}
			else
			{
				re_receive_flag = false;
				no_receive_count = 0;
				if ((temp_receive_byte_vector[0] & 0xF0) != 0x10 && (current_command.send_data.hex_data[0] & 0xF0) != 0x30 && temp_receive_byte_vector.size() == 8)//普通单帧接收
				{
					receive_byte_vector.clear();
					temp_receive_byte_vector = temp_receive_byte_vector.mid(1, temp_receive_byte_vector[0]);//只取有效数据，以一个字节表示后面有效字节个数
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

bool DealFault::DealReceiveData(ReceiveDiagloseData temp_receive_data, QVector<BYTE> receive_byte_vector)//发送过程
{
	int real_same_byte_count = 0;
	int same_byte_count = 0;
	for (int n = 0; n < temp_receive_data.litte_record_vertor.size(); n++)
	{
		LitteRecord temp_little_record = temp_receive_data.litte_record_vertor[n];
		if (!temp_little_record.data_analyse_flag)//接收数据帧中不需要解析
		{
			for (int s = 0; s < temp_little_record.byte_length; s++)
			{
				if (receive_byte_vector[s + temp_little_record.data_offset] == temp_little_record.hex_data[s])
				{
					same_byte_count++;//计算和配置文件中相同的字节数
				}
			}
			real_same_byte_count += temp_little_record.byte_length;
		}
	}
	if (same_byte_count != real_same_byte_count)//接收与配置文件中总字节数是否相同
	{
		return false;
	}
	else
	{
		QVector<BYTE> temp_byte_data;
		for (int n = 0; n < temp_receive_data.litte_record_vertor.size(); n++)
		{
			LitteRecord temp_little_record = temp_receive_data.litte_record_vertor[n];
			if (temp_little_record.data_analyse_flag)//接收帧需要解析
			{
				for (int s = 0; s < temp_little_record.byte_length; s++)
				{

					temp_byte_data.push_back(receive_byte_vector[s + temp_little_record.data_offset]);
				}
				QString analyse_name = temp_little_record.data_content.mid(1, temp_little_record.data_content.length() - 2);
				receive_map_data.insert(analyse_name, temp_byte_data);//保存解析的数据和名称
			}
		}
		return true;
	}
}

void DealFault::send_command_data(SendData send_data)//发送过程
{
	QVector<VCI_CAN_OBJ> temp_senddata_vector;
	if (send_data.frame_id != 0)//发送帧ID为0，则不进行发送，直接跳到接收
	{
		if (send_data.single_muti_frame_flag == 0)//单帧发送
		{
			VCI_CAN_OBJ temp_frame;
			temp_frame.ID = send_data.frame_id;
			temp_frame.SendType = 0;// 正常发送 
			temp_frame.RemoteFlag = 0;// 数据帧 
			if (temp_frame.ID > 0x7ff)//判断是否是扩展帧
			{
				temp_frame.ExternFlag = 1;//扩展帧
			}
			else temp_frame.ExternFlag = 0;//正常帧

			temp_frame.DataLen = 8;// 数据长度1个字节 

			if (!send_data.data_analyse_flag)//帧不需要解析
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
		else//多帧发送
		{
			QVector<BYTE> temp_byte_vector = send_data.hex_data;
			if (temp_byte_vector.size() % 8 == 0)
			{
				for (int j = 0; j < temp_byte_vector.size() / 8; j++)
				{
					VCI_CAN_OBJ temp_frame;
					temp_frame.ID = send_data.frame_id;
					temp_frame.SendType = 0;// 正常发送 
					temp_frame.RemoteFlag = 0;// 数据帧 
					if (temp_frame.ID > 0x7ff)
					{
						temp_frame.ExternFlag = 1;
					}
					else temp_frame.ExternFlag = 0;
					temp_frame.DataLen = 8;// 数据长度1个字节 
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
		send_queue_mutex.lock();//将发送数据加入发送队列
		send_can_data_queue.enqueue(temp_senddata_vector);
		send_queue_mutex.unlock();
		receive_timeout->restart();//计时开始
		QThread::msleep(10);
	}
}

QVector<BYTE> DealFault::receive_command_data(SendData send_data, ReceiveDiagloseData temp_receive_data)//接收过程
{
	QVector<BYTE> temp_byte_vector;
	int find_send_frame_index = -1;
	for (int m = 0; m < Fault_Record_Vector.size(); m++)//在获取的发送接收数据帧中，进行筛选
	{
		Record temp_can_record = Fault_Record_Vector[m];
		int same_byte_count = 0;
		if (temp_can_record.this_frame.ID == send_data.frame_id && temp_can_record.this_frame.DataLen == send_data.byte_length)//查找发送帧是否存在
		{
			for (int n = 0; n < temp_can_record.this_frame.DataLen; n++)//发送的每个字节要相同
			{
				if (temp_can_record.this_frame.Data[n] == send_data.hex_data[n])same_byte_count++;
			}
			if (same_byte_count == temp_can_record.this_frame.DataLen)//找到发送帧
			{
				find_send_frame_index = m;
				break;
			}
		}
	}
	if (find_send_frame_index >= 0)//找到发送帧
	{
		int muti_frame_flag = 0;
		for (int m = find_send_frame_index; m < Fault_Record_Vector.size(); m++)//从发送帧位置后开始查找接收帧
		{
			if (Fault_Record_Vector[m].this_frame.ID == current_page_block.receive_frame_id)//在接收数据帧中查找指定ID
			{
				if ((Fault_Record_Vector[m].this_frame.Data[0] & 0xF0) == 0x10)//多帧的首帧
				{
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(Fault_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_number = Fault_Record_Vector[m].this_frame.Data[1] + (Fault_Record_Vector[m].this_frame.Data[0] & 0x0F) * 256;//多帧总字节数
					muti_frame_byte_counter = 6;//当前帧中包含6个多帧有效字节
					Fault_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
					break;
				}
				else if ((send_data.hex_data[0] & 0xF0) == 0x30 &&
					(Fault_Record_Vector[m].this_frame.Data[0] >= 0x20
					&& Fault_Record_Vector[m].this_frame.Data[0] <= 0x2f))//多帧的，0x30发送帧对应的接收帧
				{
					for (int j = 1; j < 8; j++)
					{
						temp_byte_vector.push_back(Fault_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_counter += 7;//每帧包含7个有效字节，首字节不算
					muti_frame_flag++;//发现0x30发送帧对应的接收帧
					if (muti_frame_byte_counter >= muti_frame_byte_number)//判断多帧是否接收完成，根据有效字节判断
					{
						Fault_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
						break;
					}
				}
				else//普通接收帧
				{
					muti_frame_byte_number = 0;
					muti_frame_byte_counter = 0;
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(Fault_Record_Vector[m].this_frame.Data[j]);
					}
					Fault_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
					break;
				}
			}
		}
		if (muti_frame_flag > 0)//发现0x30发送帧对应的接收帧
		{
			if (muti_frame_byte_counter < muti_frame_byte_number)//必须达到有效字节的数量才能返回，否则清空
			{
				muti_frame_byte_counter = 6;
				temp_byte_vector.clear();
			}
			else
			{
				temp_byte_vector.remove(muti_frame_byte_number - 6, temp_byte_vector.size() - muti_frame_byte_number + 6);
			}
		}
		if (temp_byte_vector.size() > 0)//返回接收字节
		{
			TableWrite("返回数据接收完毕，本条指令完成！", 3);
		}
	}
	return temp_byte_vector;
}

void DealFault::Get_Record()//获取发送接收数据帧，这里进行帧ID过滤
{
	record_mutex.lock();
	for (int i = 0; i < CAN_Record.size(); i++)
	{
		Record temp_record = CAN_Record[i];
		if (current_page_block.send_frame_id != 0 && temp_record.this_frame.ID == current_page_block.send_frame_id && temp_record.frame_index > max_index)//这里根据帧ID和索引，避免重复获取
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			Fault_Record_Vector.push_back(temp_record);
		}
		if (current_page_block.receive_frame_id != 0 && temp_record.this_frame.ID == current_page_block.receive_frame_id && temp_record.frame_index > max_index)//这里根据帧ID和索引，避免重复获取
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
	QFile file(current_page_block.settingfile);//打开并读取配置文件
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("打开配置文件失败！", 5);
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
						TableWrite("StartBYTE_index值不是字符串！", 5);
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
						TableWrite("Byte_Length值不是字符串！", 5);
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
						TableWrite("Data值不是字符串！", 5);
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
						TableWrite("CircleTimes值不是字符串！", 5);
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
							if (rx2.indexIn(analyse_data, pos) != -1)//需要解析的部分
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
				TableWrite("文件不包含FaultCode属性！", 5);
			}
		}
		else
		{
			TableWrite("Json格式不正确！", 5);
			return fault_code_vector;
		}
	}
	else
	{
		TableWrite("Json格式不正确！错误位置： " + QString::number(json_error.offset), 5);
		return fault_code_vector;
	}
	return fault_code_vector;
}

QVector<FaultSort> DealFault::AnalyseFaultSort(QVector<BYTE> temp_data, LitteRecord this_record)
{
	QVector<FaultSort> fault_sort_vector;
	QFile file(current_page_block.settingfile);//打开并读取配置文件
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("打开配置文件失败！", 5);
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
						TableWrite("StartBYTE_index值不是字符串！", 5);
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
						TableWrite("Byte_Length值不是字符串！", 5);
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
						TableWrite("Data值不是字符串！", 5);
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
						TableWrite("CircleTimes值不是字符串！", 5);
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
							if (rx2.indexIn(analyse_data, pos) != -1)//需要解析的部分
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
				TableWrite("文件不包含FaultSort属性！", 5);
			}
		}
		else
		{
			TableWrite("Json格式不正确！", 5);
			return fault_sort_vector;
		}
	}
	else
	{
		TableWrite("Json格式不正确！错误位置： " + QString::number(json_error.offset), 5);
		return fault_sort_vector;
	}
	return fault_sort_vector;
}

QVector<SnapshotRecord> DealFault::AnalyseSnapShotInfo(QVector<BYTE> temp_data, LitteRecord this_record)
{
	QVector<SnapshotRecord> SnapshotRecord_vector;
	QFile file(current_page_block.settingfile);//打开并读取配置文件
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("打开配置文件失败！", 5);
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
						TableWrite("StartBYTE_index值不是字符串！", 5);
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
									temp_little_record.data_offset = temp_offset;//有可能为0，这里为了兼容众泰E01，其快照没有故障代码、Snapshot_index、DID_num
								}
								else
								{
									TableWrite("Byte_Offset值不是字符串！", 5);
									break;
								}
							}
							else
							{
								TableWrite("文件不包含Byte_Offset属性！", 5);
								break;
							}
							if (object3.contains("Byte_Number"))
							{
								QJsonValue value10 = object3.take("Byte_Number");
								if (value10.isString())
								{
									int temp_length = value10.toString().toInt();
									//if (temp_length > 0)temp_little_record.byte_length = temp_length;
									temp_little_record.byte_length = temp_length;//有可能为0，这里为了兼容众泰E01，其快照没有故障代码、Snapshot_index、DID_num
								}
								else
								{
									TableWrite("Byte_Number值不是字符串！", 5);
									break;
								}
							}
							else
							{
								TableWrite("文件不包含Byte_Number属性！", 5);
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
									TableWrite("Byte值不是字符串！", 5);
									break;
								}
							}
							else
							{
								TableWrite("文件不包含Byte属性！", 5);
								break;
							}
							LitteRecord_vector.push_back(temp_little_record);
						}
					}
					else
					{
						TableWrite("Data值不是数组！", 5);
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
						TableWrite("CircleTimes值不是字符串！", 5);
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
								while(i < temp_data.size())//这里为了兼容众泰E01，其快照没有故障代码、Snapshot_index、DID_num，DID_num为0
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
				TableWrite("文件不包含FaultCode属性！", 5);
			}
		}
		else
		{
			TableWrite("Json格式不正确！", 5);
			return SnapshotRecord_vector;
		}
	}
	else
	{
		TableWrite("Json格式不正确！错误位置： " + QString::number(json_error.offset), 5);
		return SnapshotRecord_vector;
	}
	return SnapshotRecord_vector;
}

SnapshotLib DealFault::AnalyseSnapshotLib(QVector<BYTE> temp_data)
{
	SnapshotLib temp_SnapshotLib;
	QFile file(current_page_block.settingfile);//打开并读取配置文件
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("打开配置文件失败！", 5);
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
									TableWrite("Byte_Offset值不是字符串！", 5);
									break;
								}
							}
							else
							{
								TableWrite("文件不包含Byte_Offset属性！", 5);
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
									TableWrite("Byte_Number值不是字符串！", 5);
									break;
								}
							}
							else
							{
								TableWrite("文件不包含Byte_Number属性！", 5);
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
									TableWrite("Byte值不是字符串！", 5);
									break;
								}
							}
							else
							{
								TableWrite("文件不包含Byte属性！", 5);
								break;
							}
							LitteRecord_vector.push_back(temp_little_record);
						}
					}
					else
					{
						TableWrite("Data值不是数组！", 5);
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
							TableWrite("读取快照配置文件失败！", 5);
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
				TableWrite("文件不包含SnapshotDID属性！", 5);
			}
		}
		else
		{
			TableWrite("Json格式不正确！", 5);
			return temp_SnapshotLib;
		}
	}
	else
	{
		TableWrite("Json格式不正确！错误位置： " + QString::number(json_error.offset), 5);
		return temp_SnapshotLib;
	}
	return temp_SnapshotLib;
}

QMap<QString, QString> DealFault::GetAnalyseReceiveDataResult(ReceiveDiagloseData temp_data_setting, QVector<BYTE> receive_byte_vector)//根据接收参数，对帧进行解析
{
	QMap<QString, QString> temp_data_map;
	if (receive_byte_vector.size() == 0)return temp_data_map;

	QBitArray data_group(receive_byte_vector.size() * 8);//按照位进行
	for (int m = 0; m < receive_byte_vector.size(); m++)
	{
		BYTE temp_char = receive_byte_vector[m];
		for (int n = 0; n < 8; n++)
		{
			bool temp = temp_char &(1 << (7 - n));
			data_group.setBit(m * 8 + n, temp_char &(1 << (7 - n)));//高位bit排在前面，下标小
		}
	}
	QString special_data_value;
	for (int i = 0; i < temp_data_setting.data_record_vertor.size(); i++)
	{
		ReceiveAnalyseData temp_analyse = temp_data_setting.data_record_vertor[i];
		if (data_group.size() < temp_analyse.bitlength)//保证为长度达到要求，否则可能是错误帧或者接收错误，避免数组超出范围
		{
			return temp_data_map;
		}
		qlonglong raw_value = 0;
		QBitArray sg_data_group(temp_analyse.bitlength);
		for (int m = 0; m < temp_analyse.bitlength; m++)//获取信号对应的bit数据
		{
			sg_data_group.setBit(m, data_group.at(temp_analyse.startbit + m));
		}
		for (int m = 0; m < sg_data_group.size(); m++)//获取实际值
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

		switch (temp_analyse.Caculate_Type)//根据Caculate_Type分别进行解析
		{
		case 0://state，位表示状态
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
		case 1://Numerical，表示数值
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
		case 2://ASC2，转换成ASC2码
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
		case 3://16进制
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
	if (special_data_value != "")//保存解析结果
	{
		temp_data_map.insert(temp_data_setting.data_record_vertor[0].caption_content, special_data_value);
	}
	return temp_data_map;
}

QString DealFault::AnalyseFormula(QString temp_formula, double temp_value)//公式处理，主要是+ - * /运算
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
	if (fabs(temp_value - (int)temp_value) < 0.00000001)//这里判断是否是整数
	{
		return QString::number((qlonglong)temp_value);
	}
	else return QString::number(temp_value, 'f', 2);
}

void DealFault::resizeEvent(QResizeEvent *event)//窗口大小改变，table大小也跟着改变
{
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void DealFault::shotcut_pushbutton_click_Slot()// 绑定的响应函数
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
	LoaderData temp_command = deal_settingfile.GetSingleCommandFromDiagnoseSettingFile("DtcInf", QStringLiteral("读快照信息"));

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
		TableWrite("配置文件解析失败！没有获得命令数据！", 5);
	}
	else
	{
		this_command = temp_command;
		if (CAN_open)
		{
			TableWrite("开始读取快照！", 1);
			startFaultThread();
		}
	}
}

void DealFault::doubleClicked_Slot(QModelIndex index)
{
	int t_w = ui.tableWidget->width();
	int scroll_w = ui.tableWidget->verticalScrollBar()->width();
	if (scroll_w != 100)//当竖直滚动条没有启用时，其宽度是100，所以当不等于100时即是滚动条没有启用
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
			ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分列
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
			ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分列
		}
	}
}

void DealFault::start_loading_Slot()
{
	thread_wait_timer->start(400);//这里不能设置太快了，否则可能出现等待窗口已经启动还未显示时，就要关闭了，会出现BUG
}

void DealFault::timeoutSlot()
{
	if (thread_wait_timer_count == 2)//等待窗口出现，即等待超过2*150=300ms才显示等待窗口
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
	if (thread_wait_timer_count >= 2)//当等待窗口出现后才进行关闭
	{
		this->setEnabled(true);
		thread_wait_timer_count = 0;
		temp_load_dialog->close();
	}
}