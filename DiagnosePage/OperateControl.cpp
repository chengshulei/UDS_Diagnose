
#include "OperateControl.h"
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

OperateControl::OperateControl(QWidget *parent)
{
	ui.setupUi(this);

	Global::loadQSS(this, "operatecontrol.qss");

	muti_frame_byte_counter = 0;
	muti_frame_byte_number = 0;
	receive_timeout = new QTime;
	receive_timeout->start();
	max_index = -1;
	stator_temperature = INIT_STATOR_TEMPERATURE;

	ui.listWidget->setFixedWidth(300);

	connect(this, SIGNAL(communicationcontrol_result_Signal(bool, QString)), this, SLOT(communicationcontrol_result_Slot(bool, QString)));

	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(start_operate_Slot()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(end_operate_Slot()));
	connect(ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(ChangeModeSlot(int)));
}

OperateControl::~OperateControl()
{
}

void OperateControl::Init()
{
	if (controldata_vector.size() == 0)
	{
		controldata_vector = deal_settingfile.GetOperateControlData();
		for (int i = 0; i < controldata_vector.size(); i++)
		{
			ui.listWidget->insertItem(i, controldata_vector[i].item_caption);
		}
		if (controldata_vector.size() > 0)
		{
			ui.listWidget->setCurrentRow(0);
		}
	}
}

void OperateControl::startThread()//�����߳�
{
	boost::function0<void> f1 = boost::bind(&OperateControl::deal_send_receive, this);
	communicationcontrol_thread = boost::thread(f1);
}

void OperateControl::ChangeModeSlot(int temp_row)//�������б�������ת
{
	current_select_row = temp_row;
	
	if (controldata_vector.size() > temp_row)
	{
		if (controldata_vector[temp_row].command_vector.size() >= 2)
		{
			ui.pushButton->setText(controldata_vector[temp_row].command_vector[0].Caption_content);
			ui.pushButton_2->setText(controldata_vector[temp_row].command_vector[1].Caption_content);
		}
			
		int input_flag = 0;
		for (int i = 0; i < controldata_vector[temp_row].command_vector.size(); i++)
		{
			if (controldata_vector[temp_row].command_vector[i].send_data.data_analyse_flag)
			{
				input_flag++;
			}
		}
		if (input_flag > 0)
		{
			ui.label->setText(controldata_vector[temp_row].item_caption + ": ");
			
			ui.label->setVisible(true);
			ui.lineEdit->setVisible(true);
		}
		else
		{
			ui.label->setVisible(false);
			ui.lineEdit->setVisible(false);
		}
	}
}

void OperateControl::start_operate_Slot()
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
		TableWrite("û�л���������ݣ�", 5);
	}
	else
	{
		if (temp_command.send_data.data_analyse_flag && temp_command.send_data.data_content.indexOf("TermAdjustment") != -1)
		{
			QString data_content = ui.lineEdit->text();
			QRegExp rx2("(^-?\\d+$)");
			int pos = 0;
			if (rx2.indexIn(data_content, pos) != -1)//��Ҫ�����Ĳ���
			{
				int temp_input_temperature = data_content.toInt();
				if (temp_input_temperature < -40 || temp_input_temperature > 0xFFFF - 40)
				{
					TableWrite("�����¶����뷶Χ����\r\n�Ϸ���Χ��-40~65495 �棡", 5);
					return;
					}
				else
				{
					stator_temperature = temp_input_temperature;
					this_command = temp_command;
					if (CAN_open)
					{
						TableWrite("��ʼ" + temp_content + "!", 1);
						startThread();
					}
				}
			}
			else
			{
				TableWrite("�����¶��������", 5);
				return;
			}
		}
		else
		{
			this_command = temp_command;
			if (CAN_open)
			{
				TableWrite("��ʼ" + temp_content + "!", 1);
				startThread();
			}
		}
	}
}

void OperateControl::end_operate_Slot()
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
		TableWrite("�����ļ�����ʧ�ܣ�û�л���������ݣ�", 5);
	}
	else
	{
		this_command = temp_command;
		if (CAN_open)
		{
			TableWrite("��ʼ��ȡ��������", 1);
			startThread();
		}
	}
}

void OperateControl::TableWrite(QString message_content, int message_type)//����������Ϣ����
{
	emit display_messageSignal(message_content, message_type);
}

void OperateControl::communicationcontrolButtontSlot()//���⹦�ܰ�ť
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
				TableWrite("�����ļ�����ʧ�ܣ�û�л���������ݣ�", 5);
				break;
			}
			else
			{
				this_command = temp_command;
				if (CAN_open)
				{
					TableWrite("��ʼ���⹦���̣߳�", 1);
					startThread();//ִ�����ݷ��ͽ��չ���
				}
			}
		}
	}
}

void OperateControl::communicationcontrol_result_Slot(bool flag, QString function_name)//���⹦�ܲ������
{
	if (!flag)
	{
		TableWrite(function_name + " ����ʧ�ܣ�", 5);
	}
	else
	{
		TableWrite(function_name + " �����ɹ���", 4);
	}
}

void OperateControl::deal_send_receive()
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

		send_realtime_data(temp_senddata);//����ָ��
		Get_Record();//��ȡ���ͽ�������֡
		if (temp_receive_settingdata.frame_id != 0)//��������֡��Ϊ����ʾ���ղ�Ϊ�գ���Ҫ�����Ӧ����֡������
		{
			QVector<BYTE> temp_receive_byte_vector = receive_realtime_data(this_command.send_data, temp_receive_settingdata);//��ý���֡�������ֽڣ�ע���֡���շ�2�ν���
			if (temp_receive_byte_vector.size() == 0)//û�н��յ�����
			{
				if (no_receive_count >= 50)//������50�ζ�û���յ�����˵���������⣬Ӧ��ֹͣˢ��
				{
					TableWrite("���ݽ����쳣���������豸��", 5);
					no_receive_count = 0;
					break;
				}
				if (receive_timeout->elapsed() < 500)//ʱ��С��100ms����������
				{
					current_command.send_data.frame_id = 0;//����֡IDΪ0��������ִ�к��÷��ͣ�ֱ�ӽ���
					re_receive_flag = true;
					continue;
				}
				else if (re_send_item < 2)//��ʱ����100ms��������С��3��ʱ�����ظ�����ִ�д�������
				{
					current_command = this_command;
					re_send_item++;//�ظ�����+1
					re_receive_flag = false;
					no_receive_count++;
					continue;
				}
				else if (re_send_item == 2)//��������ʧ��ʱ��������һ������
				{
					re_send_item = 0;
					re_receive_flag = false;
					TableWrite("�����η��ͽ���ʧ�ܣ�����ָ��ʧ�ܣ�", 2);
					no_receive_count++;
					current_command = this_command;
					continue;
				}
			}
			else
			{
				re_receive_flag = false;
				no_receive_count = 0;
				if (temp_receive_byte_vector[0] != 0x10 && temp_receive_byte_vector.size() == 8)//��ͨ��֡����
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
				else if (current_command.send_data.hex_data[0] == 0x30)//��֡�У�0x30����֮֡��Ľ�������֡
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
				else if (temp_receive_byte_vector[0] == 0x10 && temp_receive_byte_vector.size() == 8)//0x10����֡������֮����뷢��0x30֡
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
}

bool OperateControl::DealReceiveData(ReceiveDiagloseData temp_receive_data, QVector<BYTE> receive_byte_vector)//���͹���
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
		return true;
	}
}

void OperateControl::send_realtime_data(SendData send_data)//���͹���
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
			else//֡��Ҫ����
			{
				if (send_data.byte_length == 8)
				{
					QVector<BYTE> temp_data = analyse_send_singleframe(send_data.data_content);//��֡����
					for (int j = 0; j < 8; j++)
					{
						temp_frame.Data[j] = temp_data[j];
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

QVector<BYTE> OperateControl::receive_realtime_data(SendData send_data, ReceiveDiagloseData temp_receive_data)//���չ���
{
	QVector<BYTE> temp_byte_vector;
	int find_send_frame_index = -1;
	for (int m = 0; m < communicationcontrol_Record_Vector.size(); m++)//�ڻ�ȡ�ķ��ͽ�������֡�У�����ɸѡ
	{
		Record temp_can_record = communicationcontrol_Record_Vector[m];
		int same_byte_count = 0;
		if (temp_can_record.this_frame.ID == send_data.frame_id && temp_can_record.this_frame.DataLen == send_data.byte_length)//���ҷ���֡�Ƿ����
		{
			if (send_data.data_analyse_flag && send_data.data_content != "")//���￼�ǵ���seed��key��Ҫ��������������������
			{
				send_data.hex_data = analyse_send_singleframe(send_data.data_content);
			}
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
		for (int m = find_send_frame_index; m < communicationcontrol_Record_Vector.size(); m++)//�ӷ���֡λ�ú�ʼ���ҽ���֡
		{
			if (communicationcontrol_Record_Vector[m].this_frame.ID == current_page_block.receive_frame_id)//�ڽ�������֡�в���ָ��ID
			{
				if (communicationcontrol_Record_Vector[m].this_frame.Data[0] == 0x10)//��֡����֡
				{
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(communicationcontrol_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_number = communicationcontrol_Record_Vector[m].this_frame.Data[1];//��֡���ֽ���
					muti_frame_byte_counter = 6;//��ǰ֡�а���6����֡��Ч�ֽ�
					communicationcontrol_Record_Vector.remove(0, m + 1);//��շ��ͽ���֡����
					break;
				}
				else if (send_data.hex_data[0] == 0x30 &&
					(communicationcontrol_Record_Vector[m].this_frame.Data[0] >= 0x20
					&& communicationcontrol_Record_Vector[m].this_frame.Data[0] <= 0x2f))//��֡�ģ�0x30����֡��Ӧ�Ľ���֡
				{
					for (int j = 1; j < 8; j++)
					{
						temp_byte_vector.push_back(communicationcontrol_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_counter += 7;//ÿ֡����7����Ч�ֽڣ����ֽڲ���
					muti_frame_flag++;//����0x30����֡��Ӧ�Ľ���֡
					if (muti_frame_byte_counter >= muti_frame_byte_number)//�ж϶�֡�Ƿ������ɣ�������Ч�ֽ��ж�
					{
						communicationcontrol_Record_Vector.remove(0, m + 1);//��շ��ͽ���֡����
						break;
					}
				}
				else//��ͨ����֡
				{
					muti_frame_byte_number = 0;
					muti_frame_byte_counter = 0;
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(communicationcontrol_Record_Vector[m].this_frame.Data[j]);
					}
					communicationcontrol_Record_Vector.remove(0, m + 1);//��շ��ͽ���֡����
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

void OperateControl::Get_Record()//��ȡ���ͽ�������֡���������֡ID����
{
	record_mutex.lock();
	for (int i = 0; i < CAN_Record.size(); i++)
	{
		Record temp_record = CAN_Record[i];
		if (current_page_block.send_frame_id != 0 && temp_record.this_frame.ID == current_page_block.send_frame_id && temp_record.frame_index > max_index)//�������֡ID�������������ظ���ȡ
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			communicationcontrol_Record_Vector.push_back(temp_record);
		}
		if (current_page_block.receive_frame_id != 0 && temp_record.this_frame.ID == current_page_block.receive_frame_id && temp_record.frame_index > max_index)//�������֡ID�������������ظ���ȡ
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			communicationcontrol_Record_Vector.push_back(temp_record);
		}
	}
	record_mutex.unlock();
}

QVector<BYTE> OperateControl::analyse_send_singleframe(QString data_content)//������֡
{
	QVector<BYTE> tempp_data;
	QRegExp rx1("(0x([0-9A-F]{2}))");
	int pos = 0;
	while (pos < data_content.length())
	{
		while (rx1.indexIn(data_content, pos) == pos || rx1.indexIn(data_content, pos) == pos + 1)//����Ҫ�����Ĳ���
		{
			BYTE temp_byte = Global::changeHexToUINT(rx1.cap(1));
			tempp_data.push_back(temp_byte);
			pos += rx1.matchedLength() + 1;
		}
		QRegExp rx2("([0-9]+\\|\\([A-Za-z0-9]+\\))");
		if (rx2.indexIn(data_content, pos) != -1)//��Ҫ�����Ĳ���
		{
			QString temp_special_data = rx2.cap(1);
			pos += rx2.matchedLength() + 1;
			QRegExp rx3("(\\([A-Za-z0-9]+\\))");
			if (rx3.indexIn(temp_special_data, 0) != -1)
			{
				QString temp_special_name = rx3.cap(1);
				temp_special_name = temp_special_name.mid(1, temp_special_name.length() - 2);
				if (temp_special_name == "TermAdjustment")//key������seed����
				{
					QVector<BYTE> temp_input;
					if (stator_temperature != INIT_STATOR_TEMPERATURE)
					{
						int temp_tem = stator_temperature + 40;//���ﶨ���¶�+40��Ȼ��ת����2���ֽ�
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