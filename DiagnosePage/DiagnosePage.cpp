#include "DiagnosePage.h"
#include <QtCore/qmath.h>
#include "ControlCAN.H"
#include "CommunicationPage.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQueue>
#include <QMutex>
#include "Prompt.h"
#include <QDesktopWidget>
#include <boost\shared_array.hpp>
#include <boost/function/function0.hpp>   
#include <boost/thread/condition.hpp>  
#include <boost/thread/mutex.hpp>  
#include <boost/atomic.hpp>   
#include "DealSettingFile.h"

#pragma comment(lib,"ControlCAN.lib")
#define  MAX_RECORD_SIZE 1000

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


DiagnosePage::DiagnosePage(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	Global::loadQSS(this, "style.qss");

	this->setAttribute(Qt::WA_TranslucentBackground);//���ô��ڱ���͸��
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);//ȥ���߿�

	ui.close_pushButton->setToolTip(QStringLiteral("�ر�"));//��ť��ʾ
	ui.min_pushButton->setToolTip(QStringLiteral("��С��"));//��ť��ʾ

	startthread_flag = true;
	temp_index = -1;
	select_receive_frame_id = 0;
	select_send_frame_id = 0;
	thread_index = -1;
	max_index = -1;
	last_select_list_index = -1;

	ui.listWidget->insertItem(0, QStringLiteral("ѡ����"));
	ui.listWidget->setCurrentRow(0);
	ui.widget->setVisible(false);

	setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);//�������
	setFixedSize(960, 720);//�̶����ڸ߶ȺͿ��
	

	QString temp_version;//��ȡ��ǰ�汾����version.ini�л�ȡ
	QString version_path = QCoreApplication::applicationDirPath() + "/" + "version.ini";
	QFile file(version_path);
	if (file.open(QIODevice::ReadOnly))
	{
		QTextStream stream(&file);
		QString line;
		while (!stream.atEnd())
		{
			line = stream.readLine(); // ��������/n����һ���ı�
			if (line.length() >= 21)
			{
				if (line.mid(0, 8) == "version=")
				{
					temp_version = line.mid(8, 13);
					break;
				}
			}
		}
		file.close();
	}
	else
	{
		temp_version = "";
	}
	QString temp_title = "GLB_Diagnose " + temp_version;
	setWindowTitle(temp_title);//�ı�����汾��
	ui.title_label->setText(temp_title);

	TableWrite("���������", 1);

	selectmodelpage = new SelectModelPage;
	readecupage = NULL;
	realtimedata_page = NULL;
	dealfault_page = NULL;
	communicationcontrol_page = NULL;
	operatecontrol_page = NULL;
	specialfunction_page = NULL;

	while (ui.stackedWidget->count() > 0)//�������widget��ע�⣺һ��Ҫ�������Ϊ��2��Ĭ�ϵ�widget
	{
		ui.stackedWidget->removeWidget(ui.stackedWidget->currentWidget());
	}

	ui.stackedWidget->addWidget(selectmodelpage);
	ui.stackedWidget->setCurrentIndex(0);

	receivetime_count = new QTime;
	receivetime_count->start();


	connect(ui.close_pushButton, SIGNAL(clicked()), this, SLOT(close_windowSlot()));//��ťʵ��
	connect(ui.min_pushButton, SIGNAL(clicked()), this, SLOT(min_windowSlot()));//��ťʵ��
	connect(selectmodelpage, SIGNAL(selectmodelsignal(ModelInfo)), this, SLOT(ReceiveSelectModelSlot(ModelInfo)));
	connect(this, SIGNAL(DiagnoseEnterSiganl()), this, SLOT(DiagnoseEnterSlot()));
	connect(ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(ChangeModeSlot(int)));
	connect(this, SIGNAL(display_messageSignal(QString, int)), this, SLOT(TableWrite(QString, int)));
	connect(&deal_settingfile, SIGNAL(display_messageSignal(QString, int)), this, SLOT(TableWrite(QString, int)));
	connect(selectmodelpage, SIGNAL(display_messageSignal(QString, int)), this, SLOT(TableWrite(QString, int)));

	
}
DiagnosePage::~DiagnosePage()
{
	if (CAN_open)
	{
		VCI_CloseDevice(useful_device.device_type_value, useful_device.device_index);
		CAN_open = false;
		TableWrite("CAN�豸�Ѿ��򿪣����йرգ�", 1);
	}

	if (selectmodelpage != NULL)delete selectmodelpage;
	if (readecupage != NULL)delete readecupage;
	delete receivetime_count;
	if (realtimedata_page != NULL)delete realtimedata_page;
	if (dealfault_page != NULL)delete dealfault_page;
	if (specialfunction_page != NULL)delete specialfunction_page;
}

bool DiagnosePage::CANDeviceInit(int m_devtype, int index, int cannum, QString baud_setting)//��ʼ��CAN�豸�������򿪡���ʼ����������
{
	int baud = 0;
	VCI_INIT_CONFIG init_config;
	if (m_devtype == -1)return false;

	baud_setting = baud_setting.toUpper();

	QVector<QString> baud_vector;
	baud_vector.push_back("1000K");
	baud_vector.push_back("800K");
	baud_vector.push_back("500K");
	baud_vector.push_back("250K");
	baud_vector.push_back("125K");
	baud_vector.push_back("100K");
	baud_vector.push_back("50K");
	baud_vector.push_back("20K");
	baud_vector.push_back("10K");
	baud_vector.push_back("5K");

	init_config.Mode = 0;
	init_config.AccCode = 0x00000000;
	init_config.AccMask = 0xFFFFFFFF;
	init_config.Filter = 0;


	switch (baud_vector.indexOf(baud_setting))
	{
	case 0://1000K
		baud = 0x060003;
		init_config.Timing0 = (UCHAR)0x00;
		init_config.Timing1 = (UCHAR)0x14;

		break;
	case 1://800K
		baud = 0x060004;
		init_config.Timing0 = (UCHAR)0x00;
		init_config.Timing1 = (UCHAR)0x16;
		break;
	case 2://500K
		baud = 0x060007;
		init_config.Timing0 = (UCHAR)0x00;
		init_config.Timing1 = (UCHAR)0x1C;
		break;
	case 3://250K
		baud = 0x1C0008;
		init_config.Timing0 = (UCHAR)0x01;
		init_config.Timing1 = (UCHAR)0x1C;
		break;
	case 4://125K
		baud = 0x1C0011;
		init_config.Timing0 = (UCHAR)0x03;
		init_config.Timing1 = (UCHAR)0x1C;
		break;
	case 5://100K
		baud = 0x160023;
		init_config.Timing0 = (UCHAR)0x04;
		init_config.Timing1 = (UCHAR)0x1C;
		break;
	case 6://50K
		baud = 0x1C002C;
		init_config.Timing0 = (UCHAR)0x09;
		init_config.Timing1 = (UCHAR)0x1C;
		break;
	case 7://20K
		baud = 0x1600B3;
		init_config.Timing0 = (UCHAR)0x18;
		init_config.Timing1 = (UCHAR)0x1C;
		break;
	case 8://10K
		baud = 0x1C00E0;
		init_config.Timing0 = (UCHAR)0x31;
		init_config.Timing1 = (UCHAR)0x1C;
		break;
	case 9://5K
		baud = 0x1C01C1;
		init_config.Timing0 = (UCHAR)0xBF;
		init_config.Timing1 = (UCHAR)0xFF;
		break;
	default:return false;
		//break;
	}

	VCI_BOARD_INFO board_info;
	VCI_CAN_STATUS can_status;

	if (CAN_open)
	{
		VCI_CloseDevice(m_devtype, index);
		CAN_open = false;
		TableWrite("CAN�豸�Ѿ��򿪣����йرգ�", 1);
	}
	if (!CAN_open)
	{
		if (VCI_OpenDevice(m_devtype, index, cannum) != STATUS_OK)
		{
			TableWrite("���豸ʧ�ܣ�", 5);
			return false;
		}
		QThread::msleep(10);
		if (VCI_ReadBoardInfo(m_devtype, index, &board_info) != STATUS_OK)
		{
			VCI_CloseDevice(m_devtype, index);
			TableWrite("��ȡboard��Ϣʧ�ܣ�", 5);
			return false;
		}
		QThread::msleep(10);
		if (VCI_InitCAN(m_devtype, index, cannum, &init_config) != STATUS_OK)
		{
			VCI_CloseDevice(m_devtype, index);
			TableWrite("��ʼ��CANʧ�ܣ���������CAN�豸�����ӣ�", 5);
			return false;
		}
		QThread::msleep(10);
		if (VCI_ReadBoardInfo(m_devtype, index, &board_info) != STATUS_OK)
		{
			VCI_CloseDevice(m_devtype, index);
			TableWrite("��ȡboard��Ϣʧ�ܣ�", 5);
			return false;
		}
		QThread::msleep(10);
		if (VCI_ClearBuffer(m_devtype, index, cannum) != STATUS_OK)
		{
			VCI_CloseDevice(m_devtype, index);
			TableWrite("����CAN�豸flashʧ�ܣ�", 5);
			return false;
		}
		QThread::msleep(10);
		if (VCI_StartCAN(m_devtype, index, cannum) != STATUS_OK)
		{
			VCI_CloseDevice(m_devtype, index);
			TableWrite("����CAN�豸��Ϣʧ�ܣ�", 5);
			return false;
		}
		CAN_open = true;
	}
	return true;
}
void DiagnosePage::startThread()//�����߳�
{
	TableWrite("���������շ��̣߳�", 1);
	boost::function0<void> f1 = boost::bind(&DiagnosePage::send_receive, this);
	send_receive_thread = boost::thread(f1);
}

void DiagnosePage::showEvent(QShowEvent *event)//�����ڴ�С�ı�ʱҲ��ִ������¼�����󻯣�
{
	int n_wildth = this->width();
	int temp1 = qLn(n_wildth) / qLn(10) * 80;//��������10Ϊ�׵Ķ��� * 100,������������Ϊ����̫�����úܴ�ͬʱ�ڴ��ڽ�խʱҲ������խ
	ui.listWidget->setMaximumWidth(temp1);

	this->move((QApplication::desktop()->width() - this->width()) / 2, (QApplication::desktop()->height() - this->height()) / 2 - 20);//�ô����������Ͼ�����ʾ
}

void DiagnosePage::ReceiveSelectModelSlot(ModelInfo selected_modelinfo)
{
	if (selected_modelinfo.settingfile_path != "")
	{
		QThread::msleep(200);
		select_model = selected_modelinfo;
		TableWrite("ѡ�������ͺţ� " + select_model.model_name, 1);
		init_analyse_vector = deal_settingfile.AnalyseDiagnoseVehicleFile(selected_modelinfo.DiagnosisBased);
		if (init_analyse_vector.size() == 0)
		{
			TableWrite("�����ļ�����ʧ�ܣ�û�л���������ݣ�", 5);
			return;
		}
		else
		{
			select_send_frame_id = init_analyse_vector[0].send_data.frame_id;
			select_receive_frame_id = init_analyse_vector[0].receive_data.frame_id;
			CommunicationPage* temp_communicationpage = new CommunicationPage(selected_modelinfo);
			connect(temp_communicationpage, SIGNAL(communication_resultSignal(Exist_Device)), this, SLOT(communication_resultSlot(Exist_Device)));
			temp_communicationpage->show();
		}
	}
}

void DiagnosePage::TableWrite(QString message_content, int message_type)//������Ϣ
{
	if (message_type == 0)//������Ϣ����ɫΪ��ɫ
	{
		LOG_E << message_content;
	}
	else if (message_type == 1)//������Ϣ����ɫΪ��ɫ
	{
		LOG << message_content;
	}
	else if (message_type == 2)//������Ϣ�����ڽ�����Ϣ������ʾ��ֻ��log�ļ��м�¼
	{
		LOG_E << message_content;
	}
	else if (message_type == 3)//������Ϣ�����ڽ�����Ϣ������ʾ��ֻ��log�ļ��м�¼
	{
		LOG << message_content;
	}
	else if (message_type == 4)//������Ϣ�������ʾ+Log��¼+������ʾ
	{
		LOG << message_content;
		Global::Display_prompt(1, message_content);
	}
	else if (message_type == 5)//������Ϣ�������ʾ+Log��¼+������ʾ
	{
		LOG_E << message_content;
		Global::Display_prompt(0, message_content);
	}
}

void DiagnosePage::DiagnoseEnterSlot()//��������ִ����ϣ��Զ�������һ��ҳ��
{
	QString select_settingfile = QCoreApplication::applicationDirPath() + "/" + select_model.settingfile_path + "/" + "Diag_Set.txt";
	TableWrite("ѡ�г��Ͷ�Ӧ�����ļ���" + select_settingfile, 3);
	deal_settingfile.settingfile_path = select_settingfile;
	QFile file(select_settingfile);//�򿪲���ȡ�����ļ�
	QMap<int, QString> pagelist_map = deal_settingfile.GetPageList();

	if (pagelist_map.size() == 0)
	{
		TableWrite("�����ļ�����ʧ�ܣ�û��ҳ���б�", 5);
		return;
	}
	else
	{
		QVector<QString> total_listitem_vector;
		total_listitem_vector.append(QStringLiteral("��������Ϣ"));
		total_listitem_vector.append(QStringLiteral("ʵʱ����"));
		total_listitem_vector.append(QStringLiteral("��������"));
		total_listitem_vector.append(QStringLiteral("ͨ�ſ���"));
		total_listitem_vector.append(QStringLiteral("��������"));
		total_listitem_vector.append(QStringLiteral("���̿���"));
		total_listitem_vector.append(QStringLiteral("���⹦��"));

		function_map.clear();
		current_page_block.send_frame_id = select_send_frame_id;
		current_page_block.receive_frame_id = select_receive_frame_id;
		current_page_block.model_name = select_model.model_name;
		current_page_block.settingfile = select_settingfile;
		QMap<int, QString>::iterator  mi;
		for (mi = pagelist_map.begin(); mi != pagelist_map.end(); ++mi)
		{
			PageInit * temp_page = NULL;
			int temp_index = total_listitem_vector.indexOf(mi.value());
			switch (temp_index)
			{
			case 0:
			{
				ui.listWidget->insertItem(mi.key() + 1, mi.value());
				readecupage = new ReadECU;
				ui.stackedWidget->addWidget(readecupage);
				connect(readecupage, SIGNAL(display_messageSignal(QString, int)), this, SLOT(TableWrite(QString, int)));
				temp_page = (PageInit*)readecupage;
				break;
			}
			case 1:
			{
				ui.listWidget->insertItem(mi.key() + 1, mi.value());
				realtimedata_page = new ReadRealTimeData;
				ui.stackedWidget->addWidget(realtimedata_page);
				connect(this, SIGNAL(stop_realtimedata_Siganl()), realtimedata_page, SLOT(stop_refresh_Slot()));
				connect(realtimedata_page, SIGNAL(display_messageSignal(QString, int)), this, SLOT(TableWrite(QString, int)));
				temp_page = (PageInit*)realtimedata_page;
				break;
			}
			case 2:
			{
				ui.listWidget->insertItem(mi.key() + 1, mi.value());
				dealfault_page = new DealFault;
				ui.stackedWidget->addWidget(dealfault_page);
				connect(dealfault_page, SIGNAL(display_messageSignal(QString, int)), this, SLOT(TableWrite(QString, int)));
				temp_page = (PageInit*)dealfault_page;
				break;
			}
			case 3:
			{
				ui.listWidget->insertItem(mi.key() + 1, mi.value());
				communicationcontrol_page = new CommunicationControl;
				ui.stackedWidget->addWidget(communicationcontrol_page);
				connect(communicationcontrol_page, SIGNAL(display_messageSignal(QString, int)), this, SLOT(TableWrite(QString, int)));
				temp_page = (PageInit*)communicationcontrol_page;
				break;
			}
			case 4:
			{
				ui.listWidget->insertItem(mi.key() + 1, mi.value());
				operatecontrol_page = new OperateControl;
				ui.stackedWidget->addWidget(operatecontrol_page);
				connect(operatecontrol_page, SIGNAL(display_messageSignal(QString, int)), this, SLOT(TableWrite(QString, int)));
				temp_page = (PageInit*)operatecontrol_page;
				break;
			}
			case 5:
			{
				break;
			}
			case 6:
			{
				ui.listWidget->insertItem(mi.key() + 1, mi.value());
				specialfunction_page = new SpecialFunction;
				ui.stackedWidget->addWidget(specialfunction_page);
				connect(specialfunction_page, SIGNAL(display_messageSignal(QString, int)), this, SLOT(TableWrite(QString, int)));
				temp_page = (PageInit*)specialfunction_page;
				break;
			}
			default:
				break;
			}
			function_map.insert(mi.key() + 1, temp_page);
		}

		for (int i = 0; i < ui.listWidget->count(); i++)
		{
			ui.listWidget->item(i)->setTextAlignment(Qt::AlignCenter);
		}
		ui.listWidget->setCurrentRow(1);
		ui.widget->setVisible(true);
	}
}

void DiagnosePage::ChangeModeSlot(int temp_row)//�������б�������ת
{
	if (last_select_list_index == -1)last_select_list_index = temp_row;
	else
	{
		if (last_select_list_index == temp_row)return;
		else
		{
			last_select_list_index = temp_row;
		}
	}
	ui.stackedWidget->setCurrentIndex(temp_row);
	if (temp_row != 0)
	{
		PageInit* temp_page = NULL;
		QMap<int, PageInit*>::iterator  mi;
		for (mi = function_map.begin(); mi != function_map.end(); ++mi)
		{
			if (mi.key() == temp_row)
			{
				temp_page = mi.value();
				break;
			}
		}
		if (temp_page != realtimedata_page)
		{
			emit stop_realtimedata_Siganl();
		}
		temp_page->Init();//��̬Ӧ�ã����̳���Init()����
	}
}

void DiagnosePage::startDataThread()//��ʼ���н��������߳�
{
	boost::function0<void> f1 = boost::bind(&DiagnosePage::dealwith_sendreceive, this);
	data_thread = boost::thread(f1);
}

void DiagnosePage::dealwith_sendreceive()//���������̺߳���
{
	int re_send_item = 0;
	LoaderData current_command;
	bool re_receive_flag = false;
	bool DiagnoseEnter_flag = false;
	int no_receive_count = 0;
	data_thread_mutex.lock();
	while (CAN_open && ++thread_index < init_analyse_vector.size() && startthread_flag)//�����б�һ����ִ�У�CAN�豸�򿪣����߳�û�з���ֹͣ�ź�
	{
		if (!re_receive_flag)current_command = init_analyse_vector[thread_index];
		send_EnterAndHeart_data(init_analyse_vector[thread_index].send_data);
		Get_Record();
		if (!receive_EnterAndHeart_data(current_command.send_data, init_analyse_vector[thread_index].receive_data))
		{
			if (no_receive_count >= 20)//������20�ζ�û���յ�����˵���������⣬Ӧ��ֹͣˢ��
			{
				emit display_messageSignal("���ݽ����쳣���������豸��", 5);
				no_receive_count = 0;
				break;
			}
			if (receivetime_count->elapsed() < 100)//ʱ��С��100ms����������
			{
				LoaderData tempdata1 = init_analyse_vector[thread_index];
				tempdata1.send_data.frame_id = 0;
				init_analyse_vector.replace(thread_index, tempdata1);
				thread_index--;
				re_receive_flag = true;	
			}
			else if (re_send_item < 2)//��ʱ����200ms��������С��3��ʱ�����ظ�����ִ�д�������
			{
				init_analyse_vector.replace(thread_index, current_command);
				thread_index--;
				re_send_item++;//�ظ�����+1
				re_receive_flag = false;
				no_receive_count++;
			}
			else if (re_send_item == 2)//��������ʧ��ʱ��������һ������
			{
				emit display_messageSignal(current_command.Caption_content + " �������ʧ�ܣ�", 2);
				no_receive_count++;
				init_analyse_vector.replace(thread_index, current_command);
				re_send_item = 0;
				re_receive_flag = false;
				thread_index--;
				continue;
			}
		}
		else
		{
			no_receive_count = 0;
			if (!DiagnoseEnter_flag)
			{
				if (thread_index == init_analyse_vector.size() - 1)
				{
					DiagnoseEnter_flag = true;
					emit DiagnoseEnterSiganl();
				}
			}
			if (init_analyse_vector[thread_index].Dely_Time > 500)
			{
				int temp_times = init_analyse_vector[thread_index].Dely_Time / 100;
				for (int i = 0; i < temp_times; i++)
				{
					QThread::msleep(100);
					if (current_thread_order != 0)heart_thread_condition.wait(data_thread_mutex);//����������������һ�����������߳̽��������շ������̻߳�������
				}
			}
			else
			{
				QThread::msleep(init_analyse_vector[thread_index].Dely_Time);
			}
			if (init_analyse_vector[thread_index].circle_times == -1)
			{
				init_analyse_vector.replace(thread_index, current_command);
				thread_index--;
			}
			re_receive_flag = false;
		}
	}
	thread_index--;
	data_thread_mutex.unlock();
	
}

void DiagnosePage::send_EnterAndHeart_data(SendData send_data)//���ͽ�����������
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
		receivetime_count->restart();//��ʱ��ʼ
		QThread::msleep(5);
	}
}

QVector<BYTE> DiagnosePage::analyse_send_singleframe(QString data_content)//������֡
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
				if (temp_special_name == "Key")//key������seed����
				{
					QVector<BYTE> seed1;
					QVector<BYTE> key1;
					QMap<QString, QVector<BYTE>>::iterator  mi;
					for (mi = receive_map_data.begin(); mi != receive_map_data.end(); ++mi)
					{
						if ("Seed" == mi.key())
						{
							seed1 = mi.value();
							break;
						}
					}
					if (seed1.size() > 0)
					{
						key1 = ComputeKey(seed1);
					}
					for (int i = 0; i < key1.size(); i++)
					{
						tempp_data.push_back(key1[i]);
					}
				}
			}
		}
	}
	return tempp_data;
}

QVector<BYTE> DiagnosePage::ComputeKey(QVector<BYTE> seed)//����key
{
	QVector<BYTE> temp_data;
	if (select_model.model_name == "���ͼ�")
	{
		if (seed.size() != 4) return temp_data;
		BYTE Xor_Level1[4] = { 0x6A, 0xBB, 0x18, 0x23 };
		BYTE Cal[4] = { 0 };
		BYTE Key[4] = { 0 };

		for (int i = 0; i < 4; i++)
		{
			Cal[i] = seed[i] ^ Xor_Level1[i];
		}

		Key[0] = ((Cal[3] & 0x0F) << 4) | (Cal[2] & 0xF0);
		Key[1] = ((Cal[2] & 0x3F) << 2) | ((Cal[1] & 0xFC) >> 2);
		Key[2] = ((Cal[1] & 0xFC) >> 2) | (Cal[0] & 0xC0);
		Key[3] = ((Cal[0] & 0x0F) << 4) | (Cal[3] & 0x0F);


		for (int i = 0; i < 4; i++)
		{
			temp_data.push_back(Key[i]);
		}
		return temp_data;
	}
	else if (select_model.model_name == "��ʱ��" || select_model.model_name == "��ʱ��8849")
	{
		uint32_t result = 0;
		uint32_t tmp = 0;
		uint32_t temp_seed = 0;

		for (int i = 0; i < 4; i++)
		{
			temp_seed += seed[i] << ((3 - i) * 8);
		}

		result = (temp_seed >> 9) | (temp_seed << 22);/* Rotate right 9 bits */
		result *= 3;/* Multiply with 3 */
		tmp = result ^ 0x33333333;/* XOR with constant */
		result = (tmp << 14) | (tmp >> 17);/* Rotate left 14 bits */

		for (int i = 0; i < 4; i++)
		{
			temp_data.push_back(result >> ((3 - i) * 8));
		}
		return temp_data;
	}
	else if (select_model.model_name.mid(0,2) == "��̩")
	{
		uint32_t result = 0;
		uint32_t temp_seed = 0;

		for (int i = 0; i < 4; i++)
		{
			temp_seed += seed[i] << ((3 - i) * 8);
		}

		if (!((temp_seed == 0x00000000) || (temp_seed == 0xFFFFFFFF)))
		{
			for (unsigned int i = 0; i < 35; i++)
			{
				if (temp_seed & 0x80000000)
				{
					temp_seed = temp_seed << 1;
					temp_seed = temp_seed ^ 0x45303131;
				}
				else
				{
					temp_seed = temp_seed << 1;
				}
			}

			result = temp_seed;
		}
		for (int i = 0; i < 4; i++)
		{
			temp_data.push_back(result >> ((3 - i) * 8));
		}
		return temp_data;
	}
	else if (select_model.model_name.mid(0, 2) == "����")
	{
		BYTE xorArray[4] = { 0x75, 0x58, 0x97, 0x81 };
		BYTE Cal[4] = { 0 };
		BYTE Key[4] = { 0 };

		for (int i = 0; i < 4; i++)
		{
			Cal[i] = seed[i] ^ xorArray[i];
		}

		Key[0] = ((Cal[3] & 0x0F) << 4) | (Cal[3] & 0xF0);
		Key[1] = ((Cal[1] & 0x0F) << 4) | ((Cal[0] & 0xF0) >> 4);
		Key[2] = (Cal[1] & 0xF0) | ((Cal[2] & 0xF0) >> 4);
		Key[3] = ((Cal[0] & 0x0F) << 4) | (Cal[2] & 0x0F);


		for (int i = 0; i < 4; i++)
		{
			temp_data.push_back(Key[i]);
		}
		return temp_data;
	}
	else return temp_data;
}

bool DiagnosePage::receive_EnterAndHeart_data(SendData send_data, ReceiveDiagloseData temp_receive_data)//���ս�����������
{
	if (temp_receive_data.frame_id == 0)
	{
		return true;
	}
	else
	{
		int find_send_frame_index = -1;
		for (int m = 0; m < EnterAndHeart_Record_Vector.size(); m++)
		{
			Record temp_can_record = EnterAndHeart_Record_Vector[m];
			int same_byte_count = 0;
			if (temp_can_record.this_frame.ID == send_data.frame_id && temp_can_record.this_frame.DataLen == send_data.byte_length)
			{
				if (send_data.data_analyse_flag && send_data.data_content != "")//���￼�ǵ���seed��key��Ҫ��������������������
				{
					send_data.hex_data = analyse_send_singleframe(send_data.data_content);
				}
				for (int n = 0; n < temp_can_record.this_frame.DataLen; n++)
				{
					if (temp_can_record.this_frame.Data[n] == send_data.hex_data[n])same_byte_count++;
				}
				if (same_byte_count == temp_can_record.this_frame.DataLen)
				{
					find_send_frame_index = m;
					break;
				}
			}
		}
		if (find_send_frame_index >= 0)
		{
			BYTE first_byte = 0;
			int same_byte_count = 0;
			int real_same_byte_count = 0;
			for (int m = find_send_frame_index; m < EnterAndHeart_Record_Vector.size(); m++)
			{
				if (EnterAndHeart_Record_Vector[m].this_frame.ID == select_receive_frame_id)//�ڽ�������֡�в���ָ��ID
				{
					if (temp_receive_data.byte_length == 8)
					{
						for (int n = 0; n < temp_receive_data.litte_record_vertor.size(); n++)
						{
							LitteRecord temp_little_record = temp_receive_data.litte_record_vertor[n];
							if (!temp_little_record.data_analyse_flag)//��������֡�в���Ҫ����
							{
								for (int s = 0; s < temp_little_record.byte_length; s++)
								{
									if (EnterAndHeart_Record_Vector[m].this_frame.Data[s + temp_little_record.data_offset] == temp_little_record.hex_data[s])
									{
										same_byte_count++;//����������ļ�����ͬ���ֽ���
									}
								}
								real_same_byte_count += temp_little_record.byte_length;
							}
						}
					}
					if (same_byte_count != real_same_byte_count)//�����������ļ������ֽ����Ƿ���ͬ
					{
						EnterAndHeart_Record_Vector.remove(0, m);
						emit display_messageSignal("��Ӧ���յ�����֡��ֵ�������ļ���ƥ�䣡", 2);
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

									temp_byte_data.push_back(EnterAndHeart_Record_Vector[m].this_frame.Data[s + temp_little_record.data_offset]);
								}
								QString analyse_name = temp_little_record.data_content.mid(1, temp_little_record.data_content.length() - 2);
								receive_map_data.insert(analyse_name, temp_byte_data);//������������ݺ�����
							}
						}
						EnterAndHeart_Record_Vector.remove(0, m);
						emit display_messageSignal("��Ӧ���յ�����֡��ֵ�������ļ���ȫһ�£�����ָ����ɣ�", 3);
						return true;
					}
				}
			}
		}
		return false;
	}
}

void DiagnosePage::Get_Record()//ɸѡ����������Ӧ�ķ��ͽ�������
{
	record_mutex.lock();
	for (int i = 0; i < CAN_Record.size(); i++)
	{
		Record temp_record = CAN_Record[i];
		if (select_send_frame_id != 0 && temp_record.this_frame.ID == select_send_frame_id && temp_record.frame_index > max_index)
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			EnterAndHeart_Record_Vector.push_back(temp_record);
		}
		if (select_receive_frame_id != 0 && temp_record.this_frame.ID == select_receive_frame_id && temp_record.frame_index > max_index)
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			EnterAndHeart_Record_Vector.push_back(temp_record);
		}
	}
	record_mutex.unlock();
}

void DiagnosePage::send_receive()
{
	while (CAN_open)
	{
		QThread::msleep(1);
		pull_coro_t source1(boost::bind(&DiagnosePage::Send_Data, this));
		pull_coro_t source2(boost::bind(&DiagnosePage::Receive_Data, this));
	}
}

void DiagnosePage::Send_Data()
{
	QVector<VCI_CAN_OBJ> temp_senddata;
	send_queue_mutex.lock();
	if (send_can_data_queue.size() > 0)
	{
		temp_senddata = send_can_data_queue.dequeue();
	}
	send_queue_mutex.unlock();
	for (int i = 0; i < temp_senddata.size(); i++)
	{
		VCI_CAN_OBJ frameinfo = temp_senddata[i];
		unsigned long len = VCI_Transmit(useful_device.device_type_value, useful_device.device_index, useful_device.channel_number, &frameinfo, 1);//��������
		if (len != 1)
		{
			TableWrite("����֡����δ�ɹ���", 2);
			break;
		}
		else
		{
			if (select_send_frame_id != 0 && frameinfo.ID == select_send_frame_id)LOG << Global::GetFramePrintContent(frameinfo, 0);
			TakeDataToRecord(frameinfo);
		}
	}
}

void DiagnosePage::Receive_Data()
{
	VCI_ERR_INFO errinfo;
	int len = -1;
	unsigned long receive_length = VCI_GetReceiveNum(useful_device.device_type_value, useful_device.device_index, useful_device.channel_number);
	if(receive_length != 0)
	{
		boost::shared_array<VCI_CAN_OBJ> frameinfo(new VCI_CAN_OBJ[receive_length]);
		try//��Ϊ���ֹ���������֡Ϊ255���ֽڵ�����������ؽ����쳣������ֹ�������
		{
			len = VCI_Receive(useful_device.device_type_value, useful_device.device_index, useful_device.channel_number, frameinfo.get(), receive_length, 200);//��ȡ��bufffer�ĳ��Ȳ��ܳ���VCI_CAN_OBJ����Ĵ�С
		}
		catch (exception &e)
		{
			LOG_E << e.what();
			LOG_E << typeid(e).name();
			len = -1;
		}
		if (len <= 0)
		{
			VCI_ReadErrInfo(useful_device.device_type_value, useful_device.device_index, useful_device.channel_number, &errinfo);
		}
		else
		{
			for (int i = 0; i < len; i++)
			{
				if (select_receive_frame_id != 0 && frameinfo[i].ID == select_receive_frame_id)LOG << Global::GetFramePrintContent(frameinfo[i], 1);
				TakeDataToRecord(frameinfo[i]);
			}
		}
	}
}

void DiagnosePage::TakeDataToRecord(VCI_CAN_OBJ temp_frame)
{
	Record this_record;
	this_record.frame_index = ++temp_index;
	this_record.this_frame = temp_frame;
	record_mutex.lock();
	if (CAN_Record.size() == MAX_RECORD_SIZE)
	{
		CAN_Record.dequeue();
		CAN_Record.enqueue(this_record);
	}
	else
	{
		CAN_Record.enqueue(this_record);
	}
	record_mutex.unlock();
}

void DiagnosePage::communication_resultSlot(Exist_Device temp_commu_setting)
{
	if (temp_commu_setting.baundrate == "")
	{
		Global::Display_prompt(0, QStringLiteral("ͨ������ʧ�ܣ�"));
		return;
	}
	else
	{
		if (CAN_open)
		{
			VCI_CloseDevice(useful_device.device_type_value, useful_device.device_index);
			CAN_open = false;
			TableWrite("CAN�豸�Ѿ��򿪣����йرգ�", 1);
			QThread::msleep(300);
		}

		useful_device = temp_commu_setting;
		
		if (CANDeviceInit(useful_device.device_type_value, useful_device.device_index, useful_device.channel_number, useful_device.baundrate))
		{
			TableWrite("��CAN�豸�ɹ���", 1);
			startThread();
			TableWrite("��ʼ����CANͨ�ţ�", 1);
		}
		else
		{
			useful_device.Clear();
		}

		if (CAN_open)
		{
			QThread::msleep(200);
			TableWrite("��ʼ��������̣߳�", 1);
			if (thread_index != -1)
			{
				startthread_flag = false;
				QThread::msleep(100);
				thread_index = -1;
				startthread_flag = true;
			}
			startDataThread();
		}
	}
}

void DiagnosePage::close_windowSlot()//�رհ�ť
{
	this->close();
}

void DiagnosePage::min_windowSlot()//��С����ť
{
	setWindowState(Qt::WindowMinimized);
}

void DiagnosePage::mousePressEvent(QMouseEvent *event)//��д��갴���¼� 
{

	//��¼���������ڴ��ڵ�λ��  
	//event->globalPos()��갴��ʱ����������������Ļλ��  
	//pos() this->pos()��갴��ʱ�����������������Ļλ��  
	QRect labelrect = QRect(ui.top_widget->pos(), ui.top_widget->size());
	if (labelrect.contains(event->pos()))
	{
		mMoveing = true;
		mMovePosition = event->globalPos() - pos();
	}

	return QMainWindow::mousePressEvent(event);
}

void DiagnosePage::mouseMoveEvent(QMouseEvent *event)//��д����ƶ��¼�  
{
	//(event->buttons() && Qt::LeftButton)���������  
	//����ƶ��¼���Ҫ�ƶ����ڣ������ƶ��������أ�����Ҫ��ȡ����ƶ��У�������������Ļ�����꣬Ȼ��move��������꣬��ô��ȡ���ꣿ  
	//ͨ���¼�event->globalPos()֪��������꣬��������ȥ�������ڴ���λ�ã����Ǵ�����������Ļ������  
	if (mMoveing && (event->buttons() && Qt::LeftButton)
		&& (event->globalPos() - mMovePosition).manhattanLength() > QApplication::startDragDistance())
	{
		move(event->globalPos() - mMovePosition);
		mMovePosition = event->globalPos() - pos();
	}
	return QMainWindow::mouseMoveEvent(event);
}
void DiagnosePage::mouseReleaseEvent(QMouseEvent *event)
{
	mMoveing = false;
}