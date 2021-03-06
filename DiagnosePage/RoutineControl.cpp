
#include "RoutineControl.h"
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
#include <QRegExp>
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

#define INIT_STATOR_TEMPERATURE -200

RoutineControl::RoutineControl(QWidget *parent)
{
	ui.setupUi(this);

	muti_frame_byte_counter = 0;
	muti_frame_byte_number = 0;
	receive_timeout = new QTime;
	receive_timeout->start();
	max_index = -1;
	stator_temperature = INIT_STATOR_TEMPERATURE;

	connect(this, SIGNAL(communicationcontrol_result_Signal(bool, QString)), this, SLOT(communicationcontrol_result_Slot(bool, QString)));

	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(start_operate_Slot()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(end_operate_Slot()));
	connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(start_learn_Slot()));
	connect(ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(ChangeModeSlot(int)));
}

RoutineControl::~RoutineControl()
{
}

void RoutineControl::Init()
{
	controldata_vector = deal_settingfile.GetRoutineControlData();
	for (int i = 0; i < controldata_vector.size(); i++)
	{
		ui.listWidget->insertItem(i, controldata_vector[i].item_caption);
	}
	if (controldata_vector.size() > 0)
	{
		ui.listWidget->setCurrentRow(0);
	}
}

void RoutineControl::startThread()//启动线程
{
	boost::function0<void> f1 = boost::bind(&RoutineControl::deal_send_receive, this);
	communicationcontrol_thread = boost::thread(f1);
}

void RoutineControl::ChangeModeSlot(int temp_row)//点击左侧列表，界面跳转
{
	current_select_row = temp_row;
	
	if (controldata_vector.size() > temp_row)
	{		
		if (controldata_vector[temp_row].item_id == "ID_00")
		{
			ui.widget->setVisible(true);
			ui.widget_2->setVisible(false);
			ui.label->setText(controldata_vector[temp_row].item_caption);
			if (controldata_vector[temp_row].command_vector.size() >= 2)
			{
				ui.pushButton->setText(controldata_vector[temp_row].command_vector[0].Caption_content);
				ui.pushButton_2->setText(controldata_vector[temp_row].command_vector[1].Caption_content);
			}
		}
		else if(controldata_vector[temp_row].item_id == "ID_01")
		{
			ui.widget->setVisible(false);
			ui.widget_2->setVisible(true);
			if (controldata_vector[temp_row].command_vector.size() > 0)
			{
				ui.pushButton_3->setText(controldata_vector[temp_row].command_vector[0].Caption_content);
				ui.label_2->setText("");

			}
		}
	}
}

void RoutineControl::start_operate_Slot()
{
	LoaderData temp_command;
	QString temp_content;
	QString current_item = ui.pushButton->text();
	for (int i = 0; i < controldata_vector[current_select_row].command_vector.size(); i++)
	{
		if (controldata_vector[current_select_row].command_vector[i].Caption_content == current_item)
		{
			temp_command = controldata_vector[current_select_row].command_vector[i];
			temp_content = controldata_vector[current_select_row].item_caption + " " + controldata_vector[current_select_row].command_vector[i].Caption_content;
			break;
		}
	}
	

	if (temp_command.send_data.frame_id == 0)
	{
		TableWrite("没有获得命令数据！", 5);
	}
	else
	{
		if (temp_command.send_data.data_analyse_flag && temp_command.send_data.data_content.indexOf("TermAdjustment") != -1)
		{
			QString data_content = ui.lineEdit->text();
			QRegExp rx2("(^-?\\d+$)");
			int pos = 0;
			if (rx2.indexIn(data_content, pos) != -1)//需要解析的部分
			{
				int temp_input_temperature = data_content.toInt();
				if (temp_input_temperature < -40 || temp_input_temperature > 0xFFFF - 40)
				{
					TableWrite("定子温度输入范围错误！\r\n合法范围：-40~65495 ℃！", 5);
					return;
					}
				else
				{
					stator_temperature = temp_input_temperature;
					this_command = temp_command;
					if (CAN_open)
					{
						TableWrite("开始" + temp_content + "!", 1);
						startThread();
					}
				}
			}
			else
			{
				TableWrite("定子温度输入错误！", 5);
				return;
			}
		}
		else
		{
			this_command = temp_command;
			if (CAN_open)
			{
				TableWrite("开始" + temp_content + "!", 1);
				startThread();
			}
		}
	}
}

void RoutineControl::end_operate_Slot()
{
	LoaderData temp_command;
	QString current_item = ui.pushButton_2->text();
	for (int i = 0; i < controldata_vector[current_select_row].command_vector.size(); i++)
	{
		if (controldata_vector[current_select_row].command_vector[i].Caption_content == current_item)
		{
			temp_command = controldata_vector[current_select_row].command_vector[i];
			break;
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
			TableWrite("开始读取故障排序！", 1);
			startThread();
		}
	}
}

void RoutineControl::start_learn_Slot()
{

}

void RoutineControl::TableWrite(QString message_content, int message_type)//弹出警告信息窗口
{
	emit display_messageSignal(message_content, message_type);
}

void RoutineControl::communicationcontrolButtontSlot()//特殊功能按钮
{
	QWidget* current_widget = this->focusWidget();
	QString function_name = ((QPushButton*)current_widget)->text();
	for (int i = 0; i < communicationcontrol_command_vector.size(); i++)
	{
		if (communicationcontrol_command_vector[i].Caption_content == function_name)
		{
			LoaderData temp_command = communicationcontrol_command_vector[i];
			if (temp_command.send_data.frame_id == 0)
			{
				TableWrite("配置文件解析失败！没有获得命令数据！", 5);
				break;
			}
			else
			{
				this_command = temp_command;
				if (CAN_open)
				{
					TableWrite("开始特殊功能线程！", 1);
					startThread();//执行数据发送接收过程
				}
			}
		}
	}
}

void RoutineControl::communicationcontrol_result_Slot(bool flag, QString function_name)//特殊功能操作结果
{
	if (!flag)
	{
		TableWrite(function_name + " 操作失败！", 5);
	}
	else
	{
		TableWrite(function_name + " 操作成功！", 4);
	}
}

void RoutineControl::deal_send_receive()
{
	int re_send_item = 0;
	bool re_receive_flag = false;
	QMap<QString, QMap<QString, QString> > read_ECU_info_vector;
	LoaderData current_command = this_command;
	QVector<BYTE> receive_byte_vector;
	int no_receive_count = 0;
	current_thread_order++;
	data_thread_mutex.lock();
	while (CAN_open)
	{
		SendData temp_senddata = current_command.send_data;
		ReceiveDiagloseData temp_receive_settingdata = current_command.receive_data;

		send_realtime_data(temp_senddata);//发送指令
		Get_Record();//获取发送接收数据帧
		if (temp_receive_settingdata.frame_id != 0)//接收数据帧不为，表示接收不为空，需要获得相应数据帧并解析
		{
			QVector<BYTE> temp_receive_byte_vector = receive_realtime_data(this_command.send_data, temp_receive_settingdata);//获得接收帧中所有字节，注意多帧接收分2次进行
			if (temp_receive_byte_vector.size() == 0)//没有接收到数据
			{
				if (no_receive_count >= 50)//当连续50次都没有收到数据说明出了问题，应该停止刷新
				{
					TableWrite("数据接收异常！请重启设备！", 5);
					no_receive_count = 0;
					break;
				}
				if (receive_timeout->elapsed() < 500)//时间小于100ms，继续接收
				{
					current_command.send_data.frame_id = 0;//发送帧ID为0，即重新执行后不用发送，直接接收
					re_receive_flag = true;
					continue;
				}
				else if (re_send_item < 2)//计时超过100ms，当次数小于3次时进行重复重新执行此条命令
				{
					current_command = this_command;
					re_send_item++;//重复次数+1
					re_receive_flag = false;
					no_receive_count++;
					continue;
				}
				else if (re_send_item == 2)//当第三次失败时，进行下一条命令
				{
					re_send_item = 0;
					re_receive_flag = false;
					TableWrite("第三次发送接收失败，本条指令失败！", 2);
					no_receive_count++;
					current_command = this_command;
					continue;
				}
			}
			else
			{
				re_receive_flag = false;
				no_receive_count = 0;
				if (temp_receive_byte_vector[0] != 0x10 && temp_receive_byte_vector.size() == 8)//普通单帧接收
				{
					receive_byte_vector.clear();
					receive_byte_vector.append(temp_receive_byte_vector);
					if (DealReceiveData(temp_receive_settingdata, receive_byte_vector))
					{
						communicationcontrol_result_Signal(true, this_command.Caption_content);
						receive_byte_vector.clear();
						break;
					}
					else
					{
						communicationcontrol_result_Signal(false, this_command.Caption_content);
						receive_byte_vector.clear();
						break;
					}
				}
				else if (current_command.send_data.hex_data[0] == 0x30)//多帧中，0x30发送帧之后的接收数据帧
				{
					receive_byte_vector.append(temp_receive_byte_vector);
					if (DealReceiveData(temp_receive_settingdata, receive_byte_vector))
					{
						emit communicationcontrol_result_Signal(true, this_command.Caption_content);
						receive_byte_vector.clear();
						break;
					}
					else
					{
						emit communicationcontrol_result_Signal(false, this_command.Caption_content);
						receive_byte_vector.clear();
						break;
					}
				}
				else if (temp_receive_byte_vector[0] == 0x10 && temp_receive_byte_vector.size() == 8)//0x10接收帧，接收之后必须发送0x30帧
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
}

bool RoutineControl::DealReceiveData(ReceiveDiagloseData temp_receive_data, QVector<BYTE> receive_byte_vector)//发送过程
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

void RoutineControl::send_realtime_data(SendData send_data)//发送过程
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
			else//帧需要解析
			{
				if (send_data.byte_length == 8)
				{
					QVector<BYTE> temp_data = analyse_send_singleframe(send_data.data_content);//单帧解析
					for (int j = 0; j < 8; j++)
					{
						temp_frame.Data[j] = temp_data[j];
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

QVector<BYTE> RoutineControl::receive_realtime_data(SendData send_data, ReceiveDiagloseData temp_receive_data)//接收过程
{
	QVector<BYTE> temp_byte_vector;
	int find_send_frame_index = -1;
	for (int m = 0; m < communicationcontrol_Record_Vector.size(); m++)//在获取的发送接收数据帧中，进行筛选
	{
		Record temp_can_record = communicationcontrol_Record_Vector[m];
		int same_byte_count = 0;
		if (temp_can_record.this_frame.ID == send_data.frame_id && temp_can_record.this_frame.DataLen == send_data.byte_length)//查找发送帧是否存在
		{
			if (send_data.data_analyse_flag && send_data.data_content != "")//这里考虑到有seed、key需要解析，所有这里必须加上
			{
				send_data.hex_data = analyse_send_singleframe(send_data.data_content);
			}
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
		for (int m = find_send_frame_index; m < communicationcontrol_Record_Vector.size(); m++)//从发送帧位置后开始查找接收帧
		{
			if (communicationcontrol_Record_Vector[m].this_frame.ID == current_page_block.receive_frame_id)//在接收数据帧中查找指定ID
			{
				if (communicationcontrol_Record_Vector[m].this_frame.Data[0] == 0x10)//多帧的首帧
				{
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(communicationcontrol_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_number = communicationcontrol_Record_Vector[m].this_frame.Data[1];//多帧总字节数
					muti_frame_byte_counter = 6;//当前帧中包含6个多帧有效字节
					communicationcontrol_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
					break;
				}
				else if (send_data.hex_data[0] == 0x30 &&
					(communicationcontrol_Record_Vector[m].this_frame.Data[0] >= 0x20
					&& communicationcontrol_Record_Vector[m].this_frame.Data[0] <= 0x2f))//多帧的，0x30发送帧对应的接收帧
				{
					for (int j = 1; j < 8; j++)
					{
						temp_byte_vector.push_back(communicationcontrol_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_counter += 7;//每帧包含7个有效字节，首字节不算
					muti_frame_flag++;//发现0x30发送帧对应的接收帧
					if (muti_frame_byte_counter >= muti_frame_byte_number)//判断多帧是否接收完成，根据有效字节判断
					{
						communicationcontrol_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
						break;
					}
				}
				else//普通接收帧
				{
					muti_frame_byte_number = 0;
					muti_frame_byte_counter = 0;
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(communicationcontrol_Record_Vector[m].this_frame.Data[j]);
					}
					communicationcontrol_Record_Vector.remove(0, m + 1);//清空发送接收帧容器
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

void RoutineControl::Get_Record()//获取发送接收数据帧，这里进行帧ID过滤
{
	record_mutex.lock();
	for (int i = 0; i < CAN_Record.size(); i++)
	{
		Record temp_record = CAN_Record[i];
		if (current_page_block.send_frame_id != 0 && temp_record.this_frame.ID == current_page_block.send_frame_id && temp_record.frame_index > max_index)//这里根据帧ID和索引，避免重复获取
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			communicationcontrol_Record_Vector.push_back(temp_record);
		}
		if (current_page_block.receive_frame_id != 0 && temp_record.this_frame.ID == current_page_block.receive_frame_id && temp_record.frame_index > max_index)//这里根据帧ID和索引，避免重复获取
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			communicationcontrol_Record_Vector.push_back(temp_record);
		}
	}
	record_mutex.unlock();
}

QVector<BYTE> RoutineControl::analyse_send_singleframe(QString data_content)//解析单帧
{
	QVector<BYTE> tempp_data;
	QRegExp rx1("(0x([0-9A-F]{2}))");
	int pos = 0;
	while (pos < data_content.length())
	{
		while (rx1.indexIn(data_content, pos) == pos || rx1.indexIn(data_content, pos) == pos + 1)//不需要解析的部分
		{
			BYTE temp_byte = Global::changeHexToUINT(rx1.cap(1));
			tempp_data.push_back(temp_byte);
			pos += rx1.matchedLength() + 1;
		}
		QRegExp rx2("([0-9]+\\|\\([A-Za-z0-9]+\\))");
		if (rx2.indexIn(data_content, pos) != -1)//需要解析的部分
		{
			QString temp_special_data = rx2.cap(1);
			pos += rx2.matchedLength() + 1;
			QRegExp rx3("(\\([A-Za-z0-9]+\\))");
			if (rx3.indexIn(temp_special_data, 0) != -1)
			{
				QString temp_special_name = rx3.cap(1);
				temp_special_name = temp_special_name.mid(1, temp_special_name.length() - 2);
				if (temp_special_name == "TermAdjustment")//key，根据seed计算
				{
					QVector<BYTE> temp_input;
					if (stator_temperature != INIT_STATOR_TEMPERATURE)
					{
						int temp_tem = stator_temperature + 40;//这里定子温度+40，然后转换成2个字节
						if (temp_tem > 0 && temp_tem < 0xFFFF)
						{
							temp_input.append(BYTE(temp_tem / 0xFF));
							temp_input.append(BYTE(temp_tem % 0xFF));
							tempp_data.append(temp_input);
						}
					}
				}
			}
		}
	}
	return tempp_data;
}