#include "ReadECU.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QQueue>
#include <QMutex>
#include <QList>
#include <QTextCodec>
#include <QBitArray>
#include "WriteECUPage.h"
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

ReadECU::ReadECU(QWidget *parent)
{
	ui.setupUi(this);

	Global::loadQSS(this, "readecu.qss");

	receive_timeout = new QTime;
	receive_timeout->start();
	
	muti_frame_byte_counter = 0;
	muti_frame_byte_number = 0;
	max_index = -1;
	
	ui.tableWidget->clear();
	ui.tableWidget->setColumnCount(3);
	QStringList tableheader2;
	tableheader2 << QStringLiteral("信号名称") << QStringLiteral("值") << QStringLiteral("编辑");
	ui.tableWidget->setHorizontalHeaderLabels(tableheader2);
	ui.tableWidget->setShowGrid(false);//无网格线
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
	ui.tableWidget->verticalHeader()->setVisible(false);
	ui.tableWidget->setItemDelegate(new NoFocusRectangleStyle());
	ui.tableWidget->horizontalHeader()->setHighlightSections(false);

	ui.pushButton->setText(QStringLiteral("读控制器信息"));
	
	connect(this, SIGNAL(send_ECU_result_Signal(ECUTableData)), this, SLOT(send_ECU_result_Slot(ECUTableData)));
	connect(this, SIGNAL(send_writeECU_result_Signal(bool, QString)), this, SLOT(send_writeECU_result_Slot(bool, QString)));
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(Init()));
}

ReadECU::~ReadECU()
{
}

void ReadECU::startReadECUThread()//启动线程
{
	boost::function0<void> f1 = boost::bind(&ReadECU::read_ecu_data, this);
	readecu_thread = boost::thread(f1);
}
void ReadECU::startWriteECUThread()//启动线程
{
	boost::function0<void> f1 = boost::bind(&ReadECU::write_ecu_data, this);
	writeecu_thread = boost::thread(f1);
}

void ReadECU::read_ecu_data()
{
	int re_send_item = 0;
	bool re_receive_flag = false;
	QVector<ECUData> read_ECU_info_vector;
	ECUInfoData current_command;
	int i = -1;
	int no_receive_count = 0;
	QVector<BYTE> receive_byte_vector;
	current_thread_order++;
	data_thread_mutex.lock();
	while (CAN_open && ++i < readecu_analyse_vector.size())//命令列表一条条执行
	{
		QVector<ECUData> temp_ecudata_vector;
		if (!re_receive_flag)current_command = readecu_analyse_vector[i];//没有重新接收时，保存指令，因为重新接收后指令会发生变化
		SendData temp_senddata = readecu_analyse_vector[i].send_data;
		ReceiveDiagloseData temp_receive_settingdata = readecu_analyse_vector[i].receive_data;
		send_command(temp_senddata);//发送指令
		Get_Record();//获取发送接收数据帧
		if (temp_receive_settingdata.frame_id != 0)//接收数据帧不为，表示接收不为空，需要获得相应数据帧并解析
		{
			QVector<BYTE> temp_receive_byte_vector = receive_command(current_command.send_data, temp_receive_settingdata);//获得接收帧中所有字节，注意多帧接收分2次进行
			if (temp_receive_byte_vector.size() == 0)//没有接收到数据
			{
				if (no_receive_count >= 20)//当连续20次都没有收到数据说明出了问题，应该停止刷新
				{
					emit display_messageSignal("ECU据接收异常！请重启设备！", 5);
					no_receive_count = 0;
					break;
				}
				if (receive_timeout->elapsed() < 100)//时间小于100ms，继续接收
				{
					ECUInfoData tempdata1 = readecu_analyse_vector[i];
					tempdata1.send_data.frame_id = 0;//发送帧ID为0，即重新执行后不用发送，直接接收
					readecu_analyse_vector.replace(i, tempdata1);//替换当前命令;
					i = i - 1;//下次循环重新执行当前命令，不发送只接收
					re_receive_flag = true;
					continue;
				}
				else if (re_send_item < 2)//计时超过100ms，当次数小于3次时进行重复重新执行此条命令
				{
					if (current_command.send_data.hex_data[0] == 0x30)//判断当前帧是否为0x30帧，否则退回到前一个发送帧重新发送，因为0x30帧重发无效
					{
						readecu_analyse_vector.remove(i);
						i = i - 2;
					}
					else//其他发送帧接收失败，则重新发送
					{
						readecu_analyse_vector.replace(i, current_command);
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
						readecu_analyse_vector.remove(i);
						i = i - 2;
					}
					else//其他发送帧接收失败，则重新发送
					{
						readecu_analyse_vector.replace(i, current_command);
						i = i - 1;
					}
					no_receive_count++;
					re_receive_flag = false;
					continue;
				}
			}
			else
			{
				no_receive_count = 0;
				re_receive_flag = false;
				if (temp_receive_byte_vector[0] != 0x10 && current_command.send_data.hex_data[0] != 0x30 && temp_receive_byte_vector.size() == 8)//普通单帧接收
				{
					temp_receive_byte_vector.remove(0, 4);//取后4个字节为有效字节
					receive_byte_vector.append(temp_receive_byte_vector);
					temp_ecudata_vector = GetAnalyseReceiveDataResult(temp_receive_settingdata, receive_byte_vector);
					for (int j = 0; j < temp_ecudata_vector.size(); j++)
					{
						temp_ecudata_vector[j].writeECU_ID = readecu_analyse_vector[i].writeECU_ID;
					}
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
					temp_ecudata_vector = GetAnalyseReceiveDataResult(temp_receive_settingdata, receive_byte_vector);
					for (int j = 0; j < temp_ecudata_vector.size(); j++)
					{
						temp_ecudata_vector[j].writeECU_ID = readecu_analyse_vector[i].writeECU_ID;
					}
					receive_byte_vector.clear();
				}
				else if (temp_receive_byte_vector[0] == 0x10 && temp_receive_byte_vector.size() == 8)//0x10接收帧，接收之后必须发送0x30帧
				{
					temp_receive_byte_vector.remove(0, 5);//取后3个字节为有效字节
					receive_byte_vector.append(temp_receive_byte_vector);
					ECUInfoData tempdata1 = current_command;
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
					readecu_analyse_vector.insert(i + 1, tempdata1);
					continue;
				}
			}
		}
		if (temp_ecudata_vector.size() == 0)
		{
			if (current_command.send_data.hex_data[0] == 0x30)//判断当前帧是否为0x30帧，否则退回到前一个发送帧重新发送，因为0x30帧重发无效
			{
				readecu_analyse_vector.remove(i);
				i = i - 2;
			}
			else//其他发送帧接收失败，则重新发送
			{
				readecu_analyse_vector.replace(i, current_command);
				i = i - 1;
			}
		}
		else
		{
			read_ECU_info_vector.append(temp_ecudata_vector);
		}
	}
	current_thread_order--;
	heart_thread_condition.notify_one();
	data_thread_mutex.unlock();
	ECUTableData temp_ecutabledata;
	temp_ecutabledata.ecuinfo_vector = read_ECU_info_vector;
	readecu_analyse_vector.clear();//清空命令列表，该线程休眠
	read_ECU_info_vector.clear();
	TableWrite("读取ECU信息成功！", 4);
	emit send_ECU_result_Signal(temp_ecutabledata);
}

void ReadECU::write_ecu_data()
{
	int re_send_item = 0;
	bool re_receive_flag = false;
	bool writeecu_receive_flag = false;
	ECUInfoData current_command;
	int i = -1;
	int no_receive_count = 0;
	QVector<BYTE> receive_byte_vector;
	current_thread_order++;
	data_thread_mutex.lock();
	while (CAN_open && ++i < writeecu_analyse_vector.size())//命令列表一条条执行
	{
		ECUData temp_ecudata;
		if (!re_receive_flag)current_command = writeecu_analyse_vector[i];//没有重新接收时，保存指令，因为重新接收后指令会发生变化
		SendData temp_senddata = writeecu_analyse_vector[i].send_data;
		ReceiveDiagloseData temp_receive_settingdata = writeecu_analyse_vector[i].receive_data;
		send_command(temp_senddata);//发送指令
		Get_Record();//获取发送接收数据帧
		if (temp_receive_settingdata.frame_id != 0)//接收数据帧不为，表示接收不为空，需要获得相应数据帧并解析
		{
			QVector<BYTE> temp_receive_byte_vector = receive_command(current_command.send_data, temp_receive_settingdata);//获得接收帧中所有字节，注意多帧接收分2次进行
			if (temp_receive_byte_vector.size() == 0)//没有接收到数据
			{
				if (no_receive_count >= 20)//当连续20次都没有收到数据说明出了问题，应该停止刷新
				{
					emit display_messageSignal("ECU据接收异常！请重启设备！", 5);
					no_receive_count = 0;
					break;
				}
				if (receive_timeout->elapsed() < 100)//时间小于100ms，继续接收
				{
					ECUInfoData tempdata1 = writeecu_analyse_vector[i];
					tempdata1.send_data.frame_id = 0;//发送帧ID为0，即重新执行后不用发送，直接接收
					writeecu_analyse_vector.replace(i, tempdata1);//替换当前命令
					i = i - 1;//下次循环重新执行当前命令，不发送只接收
					re_receive_flag = true;
					continue;
				}
				else if (re_send_item < 2)//计时超过100ms，当次数小于3次时进行重复重新执行此条命令
				{
					if (current_command.send_data.hex_data[0] == 0x30)//判断当前帧是否为0x30帧，否则退回到前一个发送帧重新发送，因为0x30帧重发无效
					{
						writeecu_analyse_vector.remove(i);
						i = i - 2;
					}
					else//其他发送帧接收失败，则重新发送
					{
						writeecu_analyse_vector.replace(i, current_command);
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
					no_receive_count++;
					TableWrite("第三次发送接收失败，本条指令失败！", 2);
					if (current_command.send_data.hex_data[0] == 0x30)//判断当前帧是否为0x30帧，否则退回到前一个发送帧重新发送，因为0x30帧重发无效
					{
						writeecu_analyse_vector.remove(i);
						i = i - 2;
					}
					else//其他发送帧接收失败，则重新发送
					{
						writeecu_analyse_vector.replace(i, current_command);
						i = i - 1;
					}
					re_receive_flag = false;
					break;
				}
			}
			else
			{
				no_receive_count = 0;
				re_receive_flag = false;
				if (temp_receive_byte_vector[0] != 0x10 && current_command.send_data.hex_data[0] != 0x30 && temp_receive_byte_vector.size() == 8)//普通单帧接收
				{
					receive_byte_vector.append(temp_receive_byte_vector);
					writeecu_receive_flag = DealReceiveData(temp_receive_settingdata, receive_byte_vector);
					if (!writeecu_receive_flag)break;
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
					writeecu_receive_flag = DealReceiveData(temp_receive_settingdata, receive_byte_vector);
					if (!writeecu_receive_flag)break;	
					receive_byte_vector.clear();
				}
				else if (temp_receive_byte_vector[0] == 0x10 && temp_receive_byte_vector.size() == 8)//0x10接收帧，接收之后必须发送0x30帧
				{
					temp_receive_byte_vector.remove(0, 5);//取后3个字节为有效字节
					receive_byte_vector.append(temp_receive_byte_vector);
					ECUInfoData tempdata1 = current_command;
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
					writeecu_analyse_vector.insert(i + 1, tempdata1);
					continue;
				}
			}
		}
	}
	current_thread_order--;
	heart_thread_condition.notify_one();
	data_thread_mutex.unlock();
	emit send_writeECU_result_Signal(writeecu_receive_flag, writeecu_analyse_vector[0].Caption);
	writeecu_analyse_vector.clear();
}

void ReadECU::send_command(SendData send_data)//发送过程
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

QVector<BYTE> ReadECU::receive_command(SendData send_data, ReceiveDiagloseData temp_receive_data)//接收过程
{
	QVector<BYTE> temp_byte_vector;
	int find_send_frame_index = -1;
	for (int m = 0; m < Read_Record_Vector.size(); m++)//在获取的发送接收数据帧中，进行筛选
	{
		Record temp_can_record = Read_Record_Vector[m];
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
		for (int m = find_send_frame_index; m < Read_Record_Vector.size(); m++)//从发送帧位置后开始查找接收帧
		{
			if (Read_Record_Vector[m].this_frame.ID == current_page_block.receive_frame_id)//在接收数据帧中查找指定ID
			{
				if (Read_Record_Vector[m].this_frame.Data[0] == 0x10)//多帧的首帧
				{
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(Read_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_number = Read_Record_Vector[m].this_frame.Data[1];//多帧总字节数
					muti_frame_byte_counter = 6;//当前帧中包含6个多帧有效字节
					Read_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
					break;
				}
				else if (send_data.hex_data[0] == 0x30 &&
					(Read_Record_Vector[m].this_frame.Data[0] >= 0x20
					&& Read_Record_Vector[m].this_frame.Data[0] <= 0x2f))//多帧的，0x30发送帧对应的接收帧
				{
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(Read_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_counter += 7;//每帧包含7个有效字节，首字节不算
					muti_frame_flag++;//发现0x30发送帧对应的接收帧
					if (muti_frame_byte_counter >= muti_frame_byte_number)//判断多帧是否接收完成，根据有效字节判断
					{
						Read_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
						break;
					}
				}
				else//普通接收帧
				{
					muti_frame_byte_number = 0;
					muti_frame_byte_counter = 0;
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(Read_Record_Vector[m].this_frame.Data[j]);
					}
					Read_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
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

void ReadECU::Get_Record()//获取发送接收数据帧，这里进行帧ID过滤
{
	record_mutex.lock();
	for (int i = 0; i < CAN_Record.size(); i++)
	{
		Record temp_record = CAN_Record[i];
		if (current_page_block.send_frame_id != 0 && temp_record.this_frame.ID == current_page_block.send_frame_id && temp_record.frame_index > max_index)//这里根据帧ID和索引，避免重复获取
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			Read_Record_Vector.push_back(temp_record);
		}
		if (current_page_block.receive_frame_id != 0 && temp_record.this_frame.ID == current_page_block.receive_frame_id && temp_record.frame_index > max_index)//这里根据帧ID和索引，避免重复获取
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			Read_Record_Vector.push_back(temp_record);
		}
	}
	record_mutex.unlock();
}

bool ReadECU::DealReceiveData(ReceiveDiagloseData temp_receive_data, QVector<BYTE> receive_byte_vector)//发送过程
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
		return true;
	}
}

QVector<ECUData> ReadECU::GetAnalyseReceiveDataResult(ReceiveDiagloseData temp_data_setting, QVector<BYTE> receive_byte_vector)//根据接收参数，对帧进行解析
{
	QVector<ECUData> temp_ecudata_vector;
	if (receive_byte_vector.size() == 0)return temp_ecudata_vector;
	if (temp_data_setting.frame_id == 0)return temp_ecudata_vector;

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
	QString special_item_name;
	for (int i = 0; i < temp_data_setting.data_record_vertor.size(); i++)
	{
		QVector<BYTE> item_receive_byte_vector;
		ReceiveAnalyseData temp_analyse = temp_data_setting.data_record_vertor[i];
		if (data_group.size() < temp_analyse.bitlength)//保证为长度达到要求，否则可能是错误帧或者接收错误，避免数组超出范围
		{
			return temp_ecudata_vector;
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

		ECUData temp_ecudata;
		temp_ecudata.store_name = temp_analyse.store_name;
		if (temp_analyse.invalid_value != -1)
		{
			if (raw_value == temp_analyse.invalid_value)
			{
				temp_ecudata.ECUinfo_name = temp_analyse.caption_content;
				temp_ecudata.ECUinfo_value = "Invalid";
				temp_ecudata_vector.push_back(temp_ecudata);
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
					temp_ecudata.ECUinfo_name = temp_analyse.caption_content;
					temp_ecudata.ECUinfo_value = temp_content;
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
				special_item_name = temp_analyse.caption_content;
			}
			else
			{
				if (special_data_value != "")//保存解析结果
				{
					ECUData temp_ecudata;
					temp_ecudata.ECUinfo_name = special_item_name;
					temp_ecudata.ECUinfo_value = special_data_value;
					temp_ecudata_vector.push_back(temp_ecudata);
					special_data_value = "";
					special_item_name = "";
				}
				if (temp_analyse.Unit == "")
				{
					temp_ecudata.ECUinfo_name = temp_analyse.caption_content;
					temp_ecudata.ECUinfo_value = temp_value;
				}
				else
				{
					temp_ecudata.ECUinfo_name = temp_analyse.caption_content;
					temp_ecudata.ECUinfo_value = temp_value + "  " + temp_analyse.Unit;
				}
			}
			break;
		}
		case 2://ASC2，转换成ASC2码
		{
			QString temp_value;
			for (int m = temp_analyse.startbit / 8; m < receive_byte_vector.size() && m < (temp_analyse.startbit + temp_analyse.bitlength) / 8; m++)
			{
				BYTE temp_char = receive_byte_vector[m];
				temp_value.append((uchar)temp_char);

				item_receive_byte_vector.push_back(temp_char);
			}
			temp_ecudata.ECUinfo_name = temp_analyse.caption_content;
			temp_ecudata.ECUinfo_value = temp_value;
			break;
		}
		case 3://16进制
		{
			if (special_data_value != "")//保存解析结果
			{
				ECUData temp_ecudata;
				temp_ecudata.ECUinfo_name = special_item_name;
				temp_ecudata.ECUinfo_value = special_data_value;
				temp_ecudata_vector.push_back(temp_ecudata);
				special_data_value = "";
				special_item_name = "";
			}
			QString temp_value;
			for (int m = temp_analyse.startbit / 8; m < receive_byte_vector.size() && m < (temp_analyse.startbit + temp_analyse.bitlength) / 8; m++)
			{
				BYTE temp_char = receive_byte_vector[m];
				temp_value += QString("%1").arg((unsigned short)temp_char, 2, 16, QLatin1Char('0'));
			}
			temp_ecudata.ECUinfo_name = temp_analyse.caption_content;
			temp_ecudata.ECUinfo_value = temp_value;
			break;
		}
		default:
			break;
		}
		if (special_data_value == "")temp_ecudata_vector.push_back(temp_ecudata);

		if (temp_analyse.store_name != "")
		{
			receivecontent_map.insert(temp_analyse.store_name, item_receive_byte_vector);
		}
	}
	if (special_data_value != "")//保存解析结果
	{
		ECUData temp_ecudata;
		temp_ecudata.ECUinfo_name = temp_data_setting.data_record_vertor[0].caption_content;
		temp_ecudata.ECUinfo_value = special_data_value;
		temp_ecudata.store_name = temp_data_setting.data_record_vertor[0].store_name;
		temp_ecudata_vector.push_back(temp_ecudata);
	}
	return temp_ecudata_vector;
}

QString ReadECU::AnalyseFormula(QString temp_formula, double temp_value)//公式处理，主要是+ - * /运算
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


void ReadECU::Init()//更新数据
{
	readecu_analyse_vector = deal_settingfile.GetReadECUData();
	if (readecu_analyse_vector.size() == 0)
	{
		TableWrite("配置文件解析失败！没有获得命令数据！", 5);
		return;
	}
	else
	{
		if (CAN_open)
		{
			TableWrite("开始读取ECU信息！", 1);
			startReadECUThread();//执行数据发送接收过程
		}
	}
}

void ReadECU::TableWrite(QString message_content, int message_type)//弹出警告信息窗口
{
	emit display_messageSignal(message_content, message_type);
}
void ReadECU::send_ECU_result_Slot(ECUTableData temp_ecutable_data)//显示接收的控制器信息，注意这里使用的是Tree view
{
	ui.tableWidget->clear();//清空控制器信息列表，然后再重新写入
	ui.tableWidget->setColumnCount(3);
	QStringList tableheader2;
	tableheader2 << QStringLiteral("信号名称") << QStringLiteral("值") << QStringLiteral("编辑");
	ui.tableWidget->setHorizontalHeaderLabels(tableheader2);

	for (int i = 0; i < ui.tableWidget->columnCount(); i++)
	{
		ui.tableWidget->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}

	current_ecutable_data = temp_ecutable_data;
	QVector<ECUData> ecudata_vector = temp_ecutable_data.ecuinfo_vector;
	ui.tableWidget->setRowCount(ecudata_vector.size());
	for (int i = 0; i < ecudata_vector.size();i++)
	{
		QTableWidgetItem *header_item1 = new QTableWidgetItem(ecudata_vector[i].ECUinfo_name);//信号名称
		QTableWidgetItem *header_item2 = new QTableWidgetItem(ecudata_vector[i].ECUinfo_value);//值
		QTableWidgetItem *header_item3 = NULL;

		ui.tableWidget->setItem(i, 0, header_item1);
		ui.tableWidget->setItem(i, 1, header_item2);
		if (ecudata_vector[i].writeECU_ID == "")ui.tableWidget->setItem(i, 2, header_item3);
		else
		{
			QPushButton *edit_toolbutton = new QPushButton();
			edit_toolbutton->setToolTip(QStringLiteral("编辑"));
			edit_toolbutton->setFixedWidth(24);
			edit_toolbutton->setFixedHeight(24);
			edit_toolbutton->setStyleSheet("\
										QPushButton{\
										background:none;\
										border:none;\
										border-image:url(./image/btn_edit_normal.png);\
										}\
										QPushButton::hover{\
										background:none;\
										border:none;\
										border-image:url(./image/btn_edit_hover.png);\
										}\
										QPushButton::pressed{\
										border-style: inset;\
										background-color:#f6f5ec;\
										}");

			connect(edit_toolbutton, SIGNAL(clicked()), this, SLOT(edit_pushbutton_click_Slot()));// 绑定信号
			ui.tableWidget->setCellWidget(i, 2, edit_toolbutton);
		}
	}
}

void ReadECU::input_resultSlot(InputWriteECUData input_writeecudata)
{
	QVector<ECUInfoData> temp_writeecu_vector;
	for (int i = 0; i < input_writeecudata.input_writedata_vector.size(); i++)
	{
		ECUInfoData temp_write_data;
		if (input_writeecudata.input_writedata_vector[i].send_data.hex_data.size() != 8)
		{
			Global::Display_prompt(0, QStringLiteral("控制信息输入错误！"));
			return;
		}
		temp_write_data.Caption = input_writeecudata.input_writedata_vector[i].Caption;
		temp_write_data.receive_data = input_writeecudata.input_writedata_vector[i].receive_data;
		temp_write_data.send_data = input_writeecudata.input_writedata_vector[i].send_data;
		temp_write_data.writeECU_ID = input_writeecudata.input_writedata_vector[i].writeECU_ID;
		temp_writeecu_vector.push_back(temp_write_data);
	}
	if (temp_writeecu_vector.size() == 0)
	{
		Global::Display_prompt(0, QStringLiteral("控制信息输入失败！"));
		return;
	}
	else
	{
		writeecu_analyse_vector = temp_writeecu_vector;
		startWriteECUThread();
	}
}

void ReadECU::send_writeECU_result_Slot(bool write_flag, QString writeitem_name)
{
	if (!write_flag)
	{
		TableWrite("写 " + writeitem_name + " 失败！", 5);
	}
	else
	{
		TableWrite("写 " + writeitem_name + " 成功！", 4);
	}
}

void ReadECU::resizeEvent(QResizeEvent *event)//窗口大小改变，table大小也跟着改变
{
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void ReadECU::edit_pushbutton_click_Slot()// 绑定的响应函数
{
	QPushButton *senderObj = qobject_cast<QPushButton*>(sender());
	if (senderObj == nullptr)
	{
		return;
	}
	QModelIndex index = ui.tableWidget->indexAt(QPoint(senderObj->frameGeometry().x(), senderObj->frameGeometry().y()));

	ECUData temp_ecudata;
	if (current_ecutable_data.ecuinfo_vector.size() > 0)
	{
		temp_ecudata = current_ecutable_data.ecuinfo_vector[index.row()];
	}
	QVector<WriteECUData> total_wirteecudata_vector = deal_settingfile.GetWriteECUData();
	QVector<WriteECUData> select_writeecudata_vector;
	for (int i = 0; i < total_wirteecudata_vector.size(); i++)
	{
		if (total_wirteecudata_vector[i].writeECU_ID == temp_ecudata.writeECU_ID)
		{
			select_writeecudata_vector.push_back(total_wirteecudata_vector[i]);
			select_writeecudata_vector[select_writeecudata_vector.size() - 1].lasttime_readvalue = temp_ecudata.ECUinfo_value;
		}
	}

	if (select_writeecudata_vector.size() > 0)
	{
		WriteECUPage* temp_writeecu_data = new WriteECUPage(select_writeecudata_vector, receivecontent_map, temp_ecudata);
		connect(temp_writeecu_data, SIGNAL(input_resultSignal(InputWriteECUData)), this, SLOT(input_resultSlot(InputWriteECUData)));
		temp_writeecu_data->show();
	}
}
	

