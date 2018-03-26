#include "ReadRealTimeData.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QQueue>
#include <QMutex>
#include <QList>
#include <QTextCodec>
#include <QBitArray>
#include <boost/function/function0.hpp>   
#include <boost/thread/condition.hpp>  
#include <boost/thread/mutex.hpp>  
#include <boost/atomic.hpp>   
#include "DealSettingFile.h"

#define MAIN_SIGNAL_INDEX 10000

extern bool CAN_open;
extern QQueue<QVector<VCI_CAN_OBJ> > send_can_data_queue;
extern QMutex send_queue_mutex;
extern QQueue<Record> CAN_Record;
extern QMutex record_mutex;
extern FunctionBlock current_page_block;
extern boost::mutex data_thread_mutex;
extern boost::condition_variable_any heart_thread_condition;
extern boost::atomic_int current_thread_order;
extern bool realtimedata_is_running;
extern DealSettingFile deal_settingfile;

ReadRealTimeData::ReadRealTimeData(QWidget *parent)
{
	ui.setupUi(this);

	Global::loadQSS(this, "realtimedata.qss");

	receive_timeout = new QTime;
	receive_timeout->start();
	refresh_flag = 0;
	max_index = -1;
	startthread_flag = true;
	muti_frame_byte_counter = 0;
	muti_frame_byte_number = 0;

	ui.pushButton->setText(QStringLiteral("全部取消"));
	ui.pushButton_2->setText(QStringLiteral("开始刷新"));
	ui.pushButton_3->setText(QStringLiteral("读取数据"));
	tree1 = new QStandardItemModel();
	ui.treeView->setModel(tree1);
	table1 = new QStandardItemModel();
	ui.tableView->setModel(table1);

	ui.tableView->verticalHeader()->setVisible(false);
	ui.tableView->horizontalHeader()->setStretchLastSection(true);//最后一列填满表格空白
	//ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分列
	ui.tableView->setShowGrid(false);//无网格线
	ui.treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
	ui.treeView->setItemDelegate(new NoFocusRectangleStyle());
	ui.tableView->setItemDelegate(new NoFocusRectangleStyle());

	connect(this, SIGNAL(send_realtimedata_result_Signal(TreeData)), this, SLOT(send_realtimedata_result_Slot(TreeData)));
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(Check_Signal_Slot()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(Start_Refresh_Slot()));
	connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(ReadData_Slot()));
	connect(this, SIGNAL(stop_refresh_Signal()), this, SLOT(stop_refresh_Slot()));
	connect(tree1, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(treeItemChanged(QStandardItem*)));
	
}

ReadRealTimeData::~ReadRealTimeData()
{
	stopThread();
	delete tree1;
	delete table1;
	delete receive_timeout;
}

void ReadRealTimeData::ReadData_Slot()
{
	QVector<LoaderData> temp_vector = read_realtimedata_command_vector;
	int select_refresh_count = tree1->rowCount();
	for (int i = 0; i < tree1->rowCount(); i++)
	{
		if (tree1->item(i)->checkState() == Qt::Unchecked)
		{
			select_refresh_count--;
			QString temp_signal_name = tree1->item(i, 0)->text();
			for (int n = 0; n < temp_vector.size(); n++)
			{
				if (temp_vector[n].Caption_content == temp_signal_name)
				{
					temp_vector.remove(n);
					break;
				}
			}
		}
	}
	if (select_refresh_count <= 0)//没有选择信号则报错
	{
		TableWrite("没有选择任何信号，无法刷新！请至少选择一个信号！", 5);
		return;
	}
	else
	{
		read_realtimedata_command_vector = temp_vector;
		startReadThread();
	}
	ui.pushButton->setEnabled(false);
	ui.pushButton_2->setEnabled(false);
	ui.pushButton_3->setEnabled(false);
}

void ReadRealTimeData::startReadThread()//启动线程
{
	boost::function0<void> f1 = boost::bind(&ReadRealTimeData::read_data, this);
	readdata_thread = boost::thread(f1);
}
void ReadRealTimeData::startRealTimeDataThread()//启动线程
{
	boost::function0<void> f1 = boost::bind(&ReadRealTimeData::read_realtimedata, this);
	realtimedata_thread = boost::thread(f1);
}

void ReadRealTimeData::Init()//更新数据
{
	read_realtimedata_command_vector = deal_settingfile.AnalyseDiagnoseSettingFile("ReadDataStream");
	if (read_realtimedata_command_vector.size() == 0)
	{
		TableWrite("配置文件解析失败！没有获得命令数据！", 5);
		return;
	}
	else
	{
		if (CAN_open)
		{
			TableWrite("开始读取实时数据！", 1);

			tree1->clear();//清空控制器信息列表，然后再重新写入
			tree1->setColumnCount(1);

			QStringList tableheader2;
			tableheader2 << QStringLiteral("信号名称");

			tree1->setHorizontalHeaderLabels(tableheader2);
			QHeaderView * temp_header = ui.treeView->header();
			temp_header->setStyleSheet("\
										QHeaderView::section{\
										background-color:#afdfe4;\
										border-bottom:0px solid #f5f5f5;\
										height:30px;\
										border-top-left-radius:5px;\
										font:bold 20px;\
										font-family:System;\
										}");

			int row_index = 0;
			for (int i = 0; i < read_realtimedata_command_vector.size(); i++)
			{
				LoaderData temp_loaderdata = read_realtimedata_command_vector[i];
				QStandardItem *main_item1 = new QStandardItem(temp_loaderdata.Caption_content);
				treeitem_package temp_item;
				temp_item.content = temp_loaderdata.Caption_content;
				temp_item.m_checkstate = Qt::Checked;
				QMap<int, treeitem_package> sub_item_map;
				sub_item_map.insert(MAIN_SIGNAL_INDEX, temp_item);//默认父信号选中
				int sub_item_count = 0;
			
				for (int j = 0; j < temp_loaderdata.receive_data.data_record_vertor.size(); j++)
				{
					QList<QStandardItem *> items3;
					QStandardItem *header_item1 = new QStandardItem(temp_loaderdata.receive_data.data_record_vertor[j].caption_content);//信号名称
					header_item1->setCheckable(true);
					header_item1->setCheckState(Qt::Checked);
					items3.push_back(header_item1);
					main_item1->appendRow(items3);
					treeitem_package temp_item2;
					temp_item2.content = temp_loaderdata.receive_data.data_record_vertor[j].caption_content;
					temp_item2.m_checkstate = Qt::Checked;
					sub_item_map.insert(sub_item_count, temp_item2);
					sub_item_count++;
				}
				main_item1->setCheckable(true);//在主信号前面添加复选框
				main_item1->setTristate(true);//父信号有三种选中状态
				main_item1->setCheckState(Qt::Checked);
				tree1->appendRow(main_item1);
				alltreeitem_map.insert(row_index, sub_item_map);
				row_index++;
			}
			ui.treeView->expandAll();//这里是全部展开
			startReadThread();
		}
	}
}

void ReadRealTimeData::TableWrite(QString message_content, int message_type)//弹出警告信息窗口
{
	emit display_messageSignal(message_content, message_type);
}
void ReadRealTimeData::send_realtimedata_result_Slot(TreeData temp_tree_data)//显示接收的控制器信息，注意这里使用的是Tree view
{
	if (refresh_flag == 0)
	{
		if (temp_tree_data.tree_map.size() == 0)return;

		table1->clear();//清空控制器信息列表，然后再重新写入
		table1->setColumnCount(2);

		QStringList tableheader3;
		tableheader3 << QStringLiteral("信号名称") << QStringLiteral("值");
		table1->setHorizontalHeaderLabels(tableheader3);

		for (int i = 0; i < table1->columnCount(); i++)
		{
			table1->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		}

		UnOrderMap<QString, UnOrderMap<QString, QString> > tree_data = temp_tree_data.tree_map;
		int row_index = 0;
		for (int i = 0; i < tree_data.size(); i++)
		{
			UnOrderMap<QString, QString> temp_map = tree_data[i].second[0];
			QMap<int, QMap<int, treeitem_package> >::iterator it;
			for (it = alltreeitem_map.begin(); it != alltreeitem_map.end(); ++it)
			{
				QMap<int, treeitem_package> tempitem_map = it.value();
				if (tempitem_map[MAIN_SIGNAL_INDEX].content == tree_data[i].first)
				{
					 QMap<int, treeitem_package>::iterator im;
					 for (im = tempitem_map.begin(); im != tempitem_map.end(); ++im)
					{
						if (im.key() != MAIN_SIGNAL_INDEX)
						{
							if (im.value().m_checkstate == Qt::Checked)
							{
								for (int j = 0; j < temp_map.size(); j++)
								{
									if (im.value().content == temp_map[j].first)
									{
										QStandardItem *header_item3 = new QStandardItem(temp_map[j].first);//信号名称
										QStandardItem *header_item4 = new QStandardItem(temp_map[j].second[0]);//值
										QList<QStandardItem *> items4;
										items4.push_back(header_item3);
										items4.push_back(header_item4);
										table1->appendRow(items4);
									}
								}
							}
						}
					}
				}
			}
		}
		if (table1->rowCount() > 0)ui.tableView->resizeColumnToContents(0);
		ui.pushButton->setEnabled(true);
		ui.pushButton_2->setEnabled(true);
		ui.pushButton_3->setEnabled(true);
	}
	else if (refresh_flag == 1)//刷新信号数据
	{
		UnOrderMap<QString, UnOrderMap<QString, QString> > tree_data = temp_tree_data.tree_map;
		for (int i = 0; i < tree_data.size(); i++)
		{
			UnOrderMap<QString, QString> temp_map = tree_data[i].second[0];
			for (int j = 0; j < table1->rowCount(); j++)
			{
				for (int m = 0; m < temp_map.size(); m++)
				{
					if (table1->item(j, 0)->text() == temp_map[m].first)
					{
						if (table1->item(j, 1)->text() != temp_map[m].second[0])
						{
							table1->item(j, 1)->setText(temp_map[m].second[0]);//刷新数据
						}
					}
				}
			}
		}
		if (table1->rowCount() > 0)ui.tableView->resizeColumnToContents(0);
	}
	else return;
}

void ReadRealTimeData::Check_Signal_Slot()//全部选择，取消全选
{
	TableWrite(ui.pushButton->text(), 3);
	if (ui.pushButton->text() == QStringLiteral("全部选择"))
	{
		for (int i = 0; i < tree1->rowCount(); i++)
		{
			if (alltreeitem_map[i][MAIN_SIGNAL_INDEX].m_checkstate == Qt::Unchecked)
			{
				tree1->item(i)->setCheckState(Qt::Checked);
				alltreeitem_map[i][MAIN_SIGNAL_INDEX].m_checkstate = Qt::Checked;
				for (int j = 0; j < alltreeitem_map[i].size() - 1; j++)
				{
					alltreeitem_map[i][j].m_checkstate = Qt::Checked;
				}
			}
		}
		ui.pushButton->setText(QStringLiteral("全部取消"));
	}
	else if (ui.pushButton->text() == QStringLiteral("全部取消"))
	{
		for (int i = 0; i < tree1->rowCount(); i++)
		{
			if (alltreeitem_map[i][MAIN_SIGNAL_INDEX].m_checkstate == Qt::Checked)
			{
				tree1->item(i)->setCheckState(Qt::Unchecked);
				alltreeitem_map[i][MAIN_SIGNAL_INDEX].m_checkstate = Qt::Unchecked;
				for (int j = 0; j < alltreeitem_map[i].size() - 1; j++)
				{
					alltreeitem_map[i][j].m_checkstate = Qt::Unchecked;
				}
			}
		}
		ui.pushButton->setText(QStringLiteral("全部选择"));
	}
}

void ReadRealTimeData::Start_Refresh_Slot()//开始刷新、停止刷新
{
	if (ui.pushButton_2->text() == QStringLiteral("开始刷新"))//开始刷新
	{
		QVector<LoaderData> temp_vector = read_realtimedata_command_vector;
		int select_refresh_count = tree1->rowCount();
		for (int i = 0; i < tree1->rowCount(); i++)
		{
			if (tree1->item(i)->checkState() == Qt::Unchecked)
			{
				select_refresh_count--;
				QString temp_signal_name = tree1->item(i, 0)->text();
				for (int n = 0; n < temp_vector.size(); n++)
				{
					if (temp_vector[n].Caption_content == temp_signal_name)
					{
						temp_vector.remove(n);
						break;
					}
				}
			}
		}
		if (select_refresh_count <= 0)//没有选择信号则报错
		{
			TableWrite("没有选择任何信号，无法刷新！请至少选择一个信号！", 5);
			return;
		}
		else
		{
			TableWrite("停止心跳线程！开始刷新实时数据！", 1);
			refresh_flag = 1;
			startthread_flag = true;
			startRealTimeDataThread();
		}
		ui.pushButton_2->setText(QStringLiteral("停止刷新"));
		ui.pushButton->setEnabled(false);
		ui.pushButton_3->setEnabled(false);
	}
	else if (ui.pushButton_2->text() == QStringLiteral("停止刷新"))//停止刷新
	{
		TableWrite("启动心跳线程！停止刷新实时数据！", 1);
		emit stop_refresh_Signal();
	}
}

void ReadRealTimeData::stop_refresh_Slot()//停止刷新，不启动心跳线程，主要是因为实时数据接收失败，说明设备已经死机，心跳命令也会失败
{
	startthread_flag = false;
	ui.pushButton_2->setText(QStringLiteral("开始刷新"));
	ui.pushButton->setEnabled(true);
	ui.pushButton_3->setEnabled(true);
}


void ReadRealTimeData::treeItemChanged(QStandardItem * item)//修改选中状态
{
	if (item == NULL)return;
	if (refresh_flag == 1)//当刷新时，不可修改选中状态，当修改后要进行还原
	{
		if (item->rowCount() > 0)//有自item，说明不是子信号，是父信号
		{
			for (int i = 0; i < tree1->rowCount(); i++)
			{
				if (tree1->item(i)->text() == item->text())
				{
					QMap<int, treeitem_package> temp_map = alltreeitem_map[i];
					item->setCheckState(temp_map[MAIN_SIGNAL_INDEX].m_checkstate);//父信号的选中状态也放在map中，KEY是MAIN_SIGNAL_INDEX
					break;
				}
			}
		}
		else  if (item->rowCount() == 0)//子信号
		{
			QStandardItem * temp_parent = item->parent();//获得父信号
			for (int i = 0; i < tree1->rowCount(); i++)
			{
				if (tree1->item(i)->text() == temp_parent->text())//查找父信号的位置
				{
					QMap<int, treeitem_package> temp_map = alltreeitem_map[i];
					for (int j = 0; j < temp_parent->rowCount(); j++)
					{
						if (temp_parent->child(j)->text() == item->text())//子信号的索引
						{
							item->setCheckState(temp_map[j].m_checkstate);
							break;
						}
					}
					break;
				}
			}
		}
	}
	else//不是刷新状态，可以任意修改选中状态
	{
		if (item->rowCount() > 0)
		{
			for (int i = 0; i < tree1->rowCount(); i++)
			{
				if (tree1->item(i)->text() == item->text())
				{
					int main_item_index = item->rowCount();
					alltreeitem_map[i][main_item_index].m_checkstate = item->checkState();

					if (item->checkState() == Qt::Checked)//父信号选中，子信号全部选中
					{
						for (int j = 0; j < item->rowCount(); j++)
						{
							item->child(j)->setCheckState(Qt::Checked);
							alltreeitem_map[i][j].m_checkstate = Qt::Checked;
						}
					}
					else if (item->checkState() == Qt::Unchecked)//父信号取消选中，子信号全部取消选中
					{
						for (int j = 0; j < item->rowCount(); j++)
						{
							item->child(j)->setCheckState(Qt::Unchecked);
							alltreeitem_map[i][j].m_checkstate = Qt::Unchecked;
						}
					}
					break;
				}
			}
		}
		else  if (item->rowCount() == 0)//子信号
		{
			if (item->checkState() == Qt::Checked)
			{
				QStandardItem * temp_parent = item->parent();
				for (int i = 0; i < tree1->rowCount(); i++)
				{
					if (tree1->item(i)->text() == temp_parent->text())
					{
						for (int j = 0; j < temp_parent->rowCount(); j++)
						{
							if (temp_parent->child(j)->text() == item->text())
							{
								alltreeitem_map[i][j].m_checkstate = Qt::Checked;
								break;
							}
						}
						break;
					}
				}
				int child_check_count = 0;
				for (int i = 0; i < temp_parent->rowCount(); i++)//注意，若子信号全部选中，则父信号也为选中状态
				{
					if (temp_parent->child(i)->checkState() == Qt::Checked)
					{
						child_check_count++;
					}
				}
				if (child_check_count == temp_parent->rowCount() && child_check_count > 0)
				{
					temp_parent->setCheckState(Qt::Checked);

					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::Checked;
							break;
						}
					}
				}
				else if(child_check_count < temp_parent->rowCount() && child_check_count > 0)//子信号部分选中，则父信号为第三态
				{
					temp_parent->setCheckState(Qt::PartiallyChecked);
					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::PartiallyChecked;
							break;
						}
					}
				}
				else if (child_check_count == 0)
				{
					temp_parent->setCheckState(Qt::Unchecked);
					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::Unchecked;
							break;
						}
					}
				}
			}
			else if (item->checkState() == Qt::Unchecked)
			{
				QStandardItem * temp_parent = item->parent();
				for (int i = 0; i < tree1->rowCount(); i++)
				{
					if (tree1->item(i)->text() == temp_parent->text())
					{
						for (int j = 0; j < temp_parent->rowCount(); j++)
						{
							if (temp_parent->child(j)->text() == item->text())
							{
								alltreeitem_map[i][j].m_checkstate = Qt::Unchecked;
								break;
							}
						}
						break;
					}
				}
				int child_uncheck_count = 0;
				for (int i = 0; i < temp_parent->rowCount(); i++)
				{
					if (temp_parent->child(i)->checkState() == Qt::Unchecked)
					{
						child_uncheck_count++;
					}
				}
				if (child_uncheck_count == temp_parent->rowCount() && child_uncheck_count > 0)//注意，若子信号全部取消选中，则父信号也为取消选中状态
				{
					temp_parent->setCheckState(Qt::Unchecked);
					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::Unchecked;
							break;
						}
					}
				}
				else if (child_uncheck_count < temp_parent->rowCount() && child_uncheck_count > 0)//否则父信号为第三态
				{
					temp_parent->setCheckState(Qt::PartiallyChecked);
					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::PartiallyChecked;
							break;
						}
					}
				}
				else if (child_uncheck_count == 0)
				{
					temp_parent->setCheckState(Qt::Checked);
					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::Checked;
							break;
						}
					}
				}
			}
		}
		refresh_table();
	}
}
void ReadRealTimeData::refresh_table()//当左边树形列表选择状态有变化之后，右边树形列表要进行刷新
{
	table1->clear();//清空控制器信息列表，然后再重新写入
	table1->setColumnCount(2);
	QStringList tableheader3;
	tableheader3 << QStringLiteral("信号名称") << QStringLiteral("值");
	table1->setHorizontalHeaderLabels(tableheader3);
	for (int i = 0; i < tree1->rowCount(); i++)
	{
		QMap<int, treeitem_package> temp_map = alltreeitem_map[i];
		for (int j = 0; j < tree1->item(i)->rowCount(); j++)
		{
			if (temp_map[j].m_checkstate == Qt::Checked)
			{
				QStandardItem *header_item3 = new QStandardItem(tree1->item(i)->child(j)->text());//信号名称
				QStandardItem *header_item4 = new QStandardItem("");//值
				QList<QStandardItem *> items4;
				items4.push_back(header_item3);
				items4.push_back(header_item4);
				table1->appendRow(items4);
			}
		}
	}
	if (table1->rowCount() > 0)
	{
		ui.tableView->resizeColumnToContents(0);
		ui.tableView->horizontalHeader()->setStretchLastSection(true);//最后一列填满表格空白
	}
	
}


void ReadRealTimeData::read_data()
{
	int re_send_item = 0;
	bool re_receive_flag = false;
	UnOrderMap<QString, UnOrderMap<QString, QString> > read_ECU_info_vector;
	LoaderData current_command;
	int total_signal_number = 0;
	int no_receive_count = 0;
	int i = -1;
	QVector<LoaderData> command_data_vector = read_realtimedata_command_vector;
	if (total_signal_number == 0)total_signal_number = command_data_vector.size();//总信号数量
	QVector<BYTE> receive_byte_vector;
	current_thread_order++;
	data_thread_mutex.lock();
	while (CAN_open && ++i < command_data_vector.size())//命令列表一条条执行
	{
		boost::this_thread::interruption_point();//这里加入中断点，中断才会起作用
		UnOrderMap<QString, QString> one_DID_data_map;
		
		if (!re_receive_flag)current_command = command_data_vector[i];//没有重新接收时，保存指令，因为重新接收后指令会发生变化
		SendData temp_senddata = command_data_vector[i].send_data;
		ReceiveDiagloseData temp_receive_settingdata = command_data_vector[i].receive_data;
		send_realtime_data(temp_senddata);//发送指令
		Get_Record();//获取发送接收数据帧
		if (temp_receive_settingdata.frame_id != 0)//接收数据帧不为，表示接收不为空，需要获得相应数据帧并解析
		{
			QVector<BYTE> temp_receive_byte_vector = receive_realtime_data(current_command.send_data, temp_receive_settingdata);//获得接收帧中所有字节，注意多帧接收分2次进行
			if (no_receive_count >= 50)//当连续50次都没有收到数据说明出了问题，应该停止刷新
			{
				TableWrite("实时数据接收异常！中断刷新！请重启设备！", 5);
				no_receive_count = 0;
				break;
			}
			if (temp_receive_byte_vector.size() == 0)//没有接收到数据
			{
				if (receive_timeout->elapsed() < 10)//时间小于10ms，继续接收
				{
					LoaderData tempdata1 = command_data_vector[i];
					tempdata1.send_data.frame_id = 0;//发送帧ID为0，即重新执行后不用发送，直接接收
					command_data_vector.replace(i, tempdata1);//替换当前命令
					i = i - 1;//下次循环重新执行当前命令，不发送只接收
					re_receive_flag = true;
					continue;
				}
				else if (re_send_item < 2)//计时超过100ms，当次数小于3次时进行重复重新执行此条命令
				{
					if (current_command.send_data.hex_data[0] == 0x30)//判断当前帧是否为0x30帧，否则退回到前一个发送帧重新发送，因为0x30帧重发无效
					{
						command_data_vector.remove(i);
						i = i - 2;
					}
					else//其他发送帧接收失败，则重新发送
					{
						command_data_vector.replace(i, current_command);
						i = i - 1;
					}
					no_receive_count++;
					re_send_item++;//重复次数+1
					re_receive_flag = false;
					continue;
				}
				else if (re_send_item == 2)//当第三次失败时，进行下一条命令
				{
					re_send_item = 0;
					TableWrite("第三次发送接收失败，本条指令失败！", 2);
					if (current_command.send_data.hex_data[0] == 0x30)//判断当前帧是否为0x30帧，否则退回到前一个发送帧重新发送，因为0x30帧重发无效
					{
						command_data_vector.remove(i);
						i = i - 2;
					}
					else//其他发送帧接收失败，则重新发送
					{
						command_data_vector.replace(i, current_command);
						i = i - 1;
					}
					re_receive_flag = false;
					no_receive_count++;
					continue;
				}
			}
			else
			{
				re_receive_flag = false;
				no_receive_count = 0;
				if (temp_receive_byte_vector[0] != 0x10 && current_command.send_data.hex_data[0] != 0x30 && temp_receive_byte_vector.size() == 8)//普通单帧接收
				{
					temp_receive_byte_vector.remove(0, 4);//取后4个字节为有效字节
					receive_byte_vector.append(temp_receive_byte_vector);
					one_DID_data_map = GetAnalyseReceiveDataResult(temp_receive_settingdata, receive_byte_vector);
					receive_byte_vector.clear();
				}
				else if (current_command.send_data.hex_data[0] == 0x30 && temp_receive_byte_vector[0] >= 0x20 && temp_receive_byte_vector[0] <= 0x2f)//多帧中，0x30发送帧之后的接收数据帧
				{
					QVector<BYTE> temp_vector;
					for (int m = 0; m < temp_receive_byte_vector.size(); m++)
					{
						if (m % 8 != 0)temp_vector.push_back(temp_receive_byte_vector[m]);//去掉每帧的首字节
					}
					receive_byte_vector.append(temp_vector);
					one_DID_data_map = GetAnalyseReceiveDataResult(temp_receive_settingdata, receive_byte_vector);
					receive_byte_vector.clear();
				}
				else if (temp_receive_byte_vector[0] == 0x10 && temp_receive_byte_vector.size() == 8)//0x10接收帧，接收之后必须发送0x30帧
				{
					temp_receive_byte_vector.remove(0, 5);//取后3个字节为有效字节
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
					command_data_vector.insert(i + 1, tempdata1);
					continue;
				}
			}
		}
		if (one_DID_data_map.size() == 0)//解析失败
		{
			if (current_command.send_data.hex_data[0] == 0x30)//判断当前帧是否为0x30帧，否则退回到前一个发送帧重新发送，因为0x30帧重发无效
			{
				command_data_vector.remove(i);
				i = i - 2;
			}
			else//其他发送帧接收失败，则重新发送
			{
				command_data_vector.replace(i, current_command);
				i = i - 1;
			}
		}
		else
		{
			read_ECU_info_vector.insert(command_data_vector[i].Caption_content, one_DID_data_map);//保存解析结果
		}
	}
	current_thread_order--;
	heart_thread_condition.notify_one();
	data_thread_mutex.unlock();
	TreeData temp_treedata;
	refresh_flag = 0;
	if (read_ECU_info_vector.size() == total_signal_number)//当解析信号数量达到总信号数量后，将解析结果传递给主线程并显示
	{
		temp_treedata.tree_map = read_ECU_info_vector;
		TableWrite("读取实时数据成功！", 4);
		emit send_realtimedata_result_Signal(temp_treedata);
		read_ECU_info_vector.clear();
	}
	else
	{
		TableWrite("读取实时数据失败！", 5);
		emit send_realtimedata_result_Signal(temp_treedata);
		read_ECU_info_vector.clear();
	}
}


void ReadRealTimeData::read_realtimedata()
{
	int re_send_item = 0;
	bool re_receive_flag = false;
	UnOrderMap<QString, UnOrderMap<QString, QString> > read_ECU_info_vector;
	LoaderData current_command;
	int total_signal_number = 0;
	int no_receive_count = 0;
	current_thread_order++;
	data_thread_mutex.lock();
	realtimedata_is_running = true;
	while (CAN_open && startthread_flag)
	{
		boost::this_thread::interruption_point();//这里加入中断点，中断才会起作用
		int i = -1;
		QVector<LoaderData> command_data_vector = read_realtimedata_command_vector;
		if (total_signal_number == 0)total_signal_number = command_data_vector.size();//总信号数量
		QVector<BYTE> receive_byte_vector;
		while (CAN_open && ++i < command_data_vector.size() && startthread_flag)//命令列表一条条执行
		{
			UnOrderMap<QString, QString> one_DID_data_map;
			
			if (!re_receive_flag)current_command = command_data_vector[i];//没有重新接收时，保存指令，因为重新接收后指令会发生变化
			SendData temp_senddata = command_data_vector[i].send_data;
			ReceiveDiagloseData temp_receive_settingdata = command_data_vector[i].receive_data;

			send_realtime_data(temp_senddata);//发送指令
			Get_Record();//获取发送接收数据帧
			if (temp_receive_settingdata.frame_id != 0)//接收数据帧不为，表示接收不为空，需要获得相应数据帧并解析
			{
				QVector<BYTE> temp_receive_byte_vector = receive_realtime_data(current_command.send_data, temp_receive_settingdata);//获得接收帧中所有字节，注意多帧接收分2次进行
				if (temp_receive_byte_vector.size() == 0)//没有接收到数据
				{
					if (no_receive_count >= 50)//当连续50次都没有收到数据说明出了问题，应该停止刷新
					{
						TableWrite("数据接收异常！请重启设备！", 5);
						no_receive_count = 0;
						emit stop_refresh_Signal();
						break;
					}
					if (receive_timeout->elapsed() < 10)//时间小于10ms，继续接收
					{
						LoaderData tempdata1 = command_data_vector[i];
						tempdata1.send_data.frame_id = 0;//发送帧ID为0，即重新执行后不用发送，直接接收
						command_data_vector.replace(i, tempdata1);//替换当前命令
						i = i - 1;//下次循环重新执行当前命令，不发送只接收
						re_receive_flag = true;
						continue;
					}
					else if (re_send_item < 2)//计时超过100ms，当次数小于3次时进行重复重新执行此条命令
					{
						if (current_command.send_data.hex_data[0] == 0x30)//判断当前帧是否为0x30帧，否则退回到前一个发送帧重新发送，因为0x30帧重发无效
						{
							command_data_vector.remove(i);
							i = i - 2;

						}
						else//其他发送帧接收失败，则重新发送
						{
							command_data_vector.replace(i, current_command);
							i = i - 1;
						}
						no_receive_count++;
						re_send_item++;//重复次数+1
						re_receive_flag = false;
						continue;
					}
					else if (re_send_item == 2)//当第三次失败时，进行下一条命令
					{
						re_send_item = 0;
						TableWrite("第三次发送接收失败，本条指令失败！", 2);
						re_receive_flag = false;
						no_receive_count++;
						continue;
					}
				}
				else
				{
					re_receive_flag = false;
					no_receive_count = 0;
					if (temp_receive_byte_vector[0] != 0x10 && current_command.send_data.hex_data[0] != 0x30 && temp_receive_byte_vector.size() == 8)//普通单帧接收
					{
						temp_receive_byte_vector.remove(0, 4);//取后4个字节为有效字节
						receive_byte_vector.append(temp_receive_byte_vector);
						one_DID_data_map = GetAnalyseReceiveDataResult(temp_receive_settingdata, receive_byte_vector);
						receive_byte_vector.clear();
					}
					else if (current_command.send_data.hex_data[0] == 0x30 && temp_receive_byte_vector[0] >= 0x20 && temp_receive_byte_vector[0] <= 0x2f)//多帧中，0x30发送帧之后的接收数据帧
					{
						QVector<BYTE> temp_vector;
						for (int m = 0; m < temp_receive_byte_vector.size(); m++)
						{
							if (m % 8 != 0)temp_vector.push_back(temp_receive_byte_vector[m]);//去掉每帧的首字节
						}
						
						receive_byte_vector.append(temp_vector);
						one_DID_data_map = GetAnalyseReceiveDataResult(temp_receive_settingdata, receive_byte_vector);
						receive_byte_vector.clear();
					}
					else if (temp_receive_byte_vector[0] == 0x10 && temp_receive_byte_vector.size() == 8)//0x10接收帧，接收之后必须发送0x30帧
					{
						temp_receive_byte_vector.remove(0, 5);//取后3个字节为有效字节
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
						command_data_vector.insert(i + 1, tempdata1);
						continue;
					}
				}
			}
			if (one_DID_data_map.size() == 0)//解析失败
			{
				if (current_command.send_data.hex_data[0] == 0x30)//判断当前帧是否为0x30帧，否则退回到前一个发送帧重新发送，因为0x30帧重发无效
				{
					command_data_vector.remove(i);
					i = i - 2;
				}
				else//其他发送帧接收失败，则重新发送
				{
					command_data_vector.replace(i, current_command);
					i = i - 1;
				}
			}
			else read_ECU_info_vector.insert(command_data_vector[i].Caption_content, one_DID_data_map);//保存解析结果
		}
		if (read_ECU_info_vector.size() == total_signal_number)//当解析信号数量达到总信号数量后，将解析结果传递给主线程并显示
		{
			TreeData temp_treedata;
			refresh_flag = 1;
			temp_treedata.tree_map = read_ECU_info_vector;
			emit send_realtimedata_result_Signal(temp_treedata);
			read_ECU_info_vector.clear();
			QThread::msleep(30);
		}
		else
		{
			read_ECU_info_vector.clear();
			continue;
		}
	}
	current_thread_order--;
	heart_thread_condition.notify_one();
	data_thread_mutex.unlock();
	refresh_flag = 0;
	realtimedata_is_running = false;
}

void ReadRealTimeData::send_realtime_data(SendData send_data)//发送过程
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
		QThread::msleep(5);
	}
}

QVector<BYTE> ReadRealTimeData::receive_realtime_data(SendData send_data, ReceiveDiagloseData temp_receive_data)//接收过程
{
	QVector<BYTE> temp_byte_vector;
	int find_send_frame_index = -1;
	for (int m = 0; m < ReadRealTime_Record_Vector.size(); m++)//在获取的发送接收数据帧中，进行筛选
	{
		Record temp_can_record = ReadRealTime_Record_Vector[m];
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
		for (int m = find_send_frame_index; m < ReadRealTime_Record_Vector.size(); m++)//从发送帧位置后开始查找接收帧
		{
			if (ReadRealTime_Record_Vector[m].this_frame.ID == current_page_block.receive_frame_id)//在接收数据帧中查找指定ID
			{
				if (ReadRealTime_Record_Vector[m].this_frame.Data[0] == 0x10)//多帧的首帧
				{
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(ReadRealTime_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_number = ReadRealTime_Record_Vector[m].this_frame.Data[1];//多帧总字节数
					muti_frame_byte_counter = 6;//当前帧中包含6个多帧有效字节
					ReadRealTime_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
					break;
				}
				else if (send_data.hex_data[0] == 0x30 &&
					(ReadRealTime_Record_Vector[m].this_frame.Data[0] >= 0x20
					&& ReadRealTime_Record_Vector[m].this_frame.Data[0] <= 0x2f))//多帧的，0x30发送帧对应的接收帧
				{
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(ReadRealTime_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_counter += 7;//每帧包含7个有效字节，首字节不算
					muti_frame_flag++;//发现0x30发送帧对应的接收帧
					if (muti_frame_byte_counter >= muti_frame_byte_number)//判断多帧是否接收完成，根据有效字节判断
					{
						ReadRealTime_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
						break;
					}
				}
				else//普通接收帧
				{
					muti_frame_byte_number = 0;
					muti_frame_byte_counter = 0;
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(ReadRealTime_Record_Vector[m].this_frame.Data[j]);
					}
					ReadRealTime_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
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
		}
		if (temp_byte_vector.size() > 0)//返回接收字节
		{
			TableWrite("返回数据接收完毕，本条指令完成！", 3);
		}
	}
	return temp_byte_vector;
}

void ReadRealTimeData::Get_Record()//获取发送接收数据帧，这里进行帧ID过滤
{
	record_mutex.lock();
	for (int i = 0; i < CAN_Record.size(); i++)
	{
		Record temp_record = CAN_Record[i];
		if (current_page_block.send_frame_id != 0 && temp_record.this_frame.ID == current_page_block.send_frame_id && temp_record.frame_index > max_index)//这里根据帧ID和索引，避免重复获取
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			ReadRealTime_Record_Vector.push_back(temp_record);
		}
		if (current_page_block.receive_frame_id != 0 && temp_record.this_frame.ID == current_page_block.receive_frame_id && temp_record.frame_index > max_index)//这里根据帧ID和索引，避免重复获取
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			ReadRealTime_Record_Vector.push_back(temp_record);
		}
	}
	record_mutex.unlock();
}

UnOrderMap<QString, QString> ReadRealTimeData::GetAnalyseReceiveDataResult(ReceiveDiagloseData temp_data_setting, QVector<BYTE> receive_byte_vector)//根据接收参数，对帧进行解析
{
	UnOrderMap<QString, QString> temp_data_map;
	if (receive_byte_vector.size() == 0)return temp_data_map;
	if (temp_data_setting.frame_id == 0)return temp_data_map;

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
		if (data_group.size() < temp_analyse.startbit + temp_analyse.bitlength)//保证为长度达到要求，否则可能是错误帧或者接收错误，避免数组超出范围
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
				if (temp_analyse.Unit == "")
				{
					temp_data_map.insert(temp_analyse.caption_content, temp_value);
				}
				else
				{
					temp_data_map.insert(temp_analyse.caption_content, temp_value + "  " + temp_analyse.Unit);
				}
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

QString ReadRealTimeData::AnalyseFormula(QString temp_formula, double temp_value)//公式处理，主要是+ - * /运算
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

void ReadRealTimeData::stopThread()//停止实时数据线程
{
	startthread_flag = false;
	realtimedata_thread.interrupt();
	realtimedata_thread.join();
}
