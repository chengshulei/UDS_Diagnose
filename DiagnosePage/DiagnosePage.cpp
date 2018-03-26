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

	this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);//去掉边框

	ui.close_pushButton->setToolTip(QStringLiteral("关闭"));//按钮提示
	ui.min_pushButton->setToolTip(QStringLiteral("最小化"));//按钮提示

	startthread_flag = true;
	temp_index = -1;
	select_receive_frame_id = 0;
	select_send_frame_id = 0;
	thread_index = -1;
	max_index = -1;
	last_select_list_index = -1;

	ui.listWidget->insertItem(0, QStringLiteral("选择车型"));
	ui.listWidget->setCurrentRow(0);
	ui.widget->setVisible(false);

	setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);//禁用最大化
	setFixedSize(960, 720);//固定窗口高度和宽度
	

	QString temp_version;//获取当前版本，从version.ini中获取
	QString version_path = QCoreApplication::applicationDirPath() + "/" + "version.ini";
	QFile file(version_path);
	if (file.open(QIODevice::ReadOnly))
	{
		QTextStream stream(&file);
		QString line;
		while (!stream.atEnd())
		{
			line = stream.readLine(); // 不包括“/n”的一行文本
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
	setWindowTitle(temp_title);//改变软件版本号
	ui.title_label->setText(temp_title);

	TableWrite("软件启动！", 1);

	selectmodelpage = new SelectModelPage;
	readecupage = NULL;
	realtimedata_page = NULL;
	dealfault_page = NULL;
	communicationcontrol_page = NULL;
	operatecontrol_page = NULL;
	specialfunction_page = NULL;

	while (ui.stackedWidget->count() > 0)//清除所有widget，注意：一定要清除，因为有2个默认的widget
	{
		ui.stackedWidget->removeWidget(ui.stackedWidget->currentWidget());
	}

	ui.stackedWidget->addWidget(selectmodelpage);
	ui.stackedWidget->setCurrentIndex(0);

	receivetime_count = new QTime;
	receivetime_count->start();


	connect(ui.close_pushButton, SIGNAL(clicked()), this, SLOT(close_windowSlot()));//按钮实现
	connect(ui.min_pushButton, SIGNAL(clicked()), this, SLOT(min_windowSlot()));//按钮实现
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
		TableWrite("CAN设备已经打开，进行关闭！", 1);
	}

	if (selectmodelpage != NULL)delete selectmodelpage;
	if (readecupage != NULL)delete readecupage;
	delete receivetime_count;
	if (realtimedata_page != NULL)delete realtimedata_page;
	if (dealfault_page != NULL)delete dealfault_page;
	if (specialfunction_page != NULL)delete specialfunction_page;
}

bool DiagnosePage::CANDeviceInit(int m_devtype, int index, int cannum, QString baud_setting)//初始化CAN设备，包含打开、初始化、启动等
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
		TableWrite("CAN设备已经打开，进行关闭！", 1);
	}
	if (!CAN_open)
	{
		if (VCI_OpenDevice(m_devtype, index, cannum) != STATUS_OK)
		{
			TableWrite("打开设备失败！", 5);
			return false;
		}
		QThread::msleep(10);
		if (VCI_ReadBoardInfo(m_devtype, index, &board_info) != STATUS_OK)
		{
			VCI_CloseDevice(m_devtype, index);
			TableWrite("读取board信息失败！", 5);
			return false;
		}
		QThread::msleep(10);
		if (VCI_InitCAN(m_devtype, index, cannum, &init_config) != STATUS_OK)
		{
			VCI_CloseDevice(m_devtype, index);
			TableWrite("初始化CAN失败！请重启与CAN设备的连接！", 5);
			return false;
		}
		QThread::msleep(10);
		if (VCI_ReadBoardInfo(m_devtype, index, &board_info) != STATUS_OK)
		{
			VCI_CloseDevice(m_devtype, index);
			TableWrite("读取board信息失败！", 5);
			return false;
		}
		QThread::msleep(10);
		if (VCI_ClearBuffer(m_devtype, index, cannum) != STATUS_OK)
		{
			VCI_CloseDevice(m_devtype, index);
			TableWrite("擦除CAN设备flash失败！", 5);
			return false;
		}
		QThread::msleep(10);
		if (VCI_StartCAN(m_devtype, index, cannum) != STATUS_OK)
		{
			VCI_CloseDevice(m_devtype, index);
			TableWrite("启动CAN设备信息失败！", 5);
			return false;
		}
		CAN_open = true;
	}
	return true;
}
void DiagnosePage::startThread()//启动线程
{
	TableWrite("启动数据收发线程！", 1);
	boost::function0<void> f1 = boost::bind(&DiagnosePage::send_receive, this);
	send_receive_thread = boost::thread(f1);
}

void DiagnosePage::showEvent(QShowEvent *event)//当窗口大小改变时也会执行这个事件（最大化）
{
	int n_wildth = this->width();
	int temp1 = qLn(n_wildth) / qLn(10) * 80;//这里是以10为底的对数 * 100,这样避免宽度因为窗口太大而变得很大，同时在窗口较窄时也跟着缩窄
	ui.listWidget->setMaximumWidth(temp1);

	this->move((QApplication::desktop()->width() - this->width()) / 2, (QApplication::desktop()->height() - this->height()) / 2 - 20);//让窗口在桌面上居中显示
}

void DiagnosePage::ReceiveSelectModelSlot(ModelInfo selected_modelinfo)
{
	if (selected_modelinfo.settingfile_path != "")
	{
		QThread::msleep(200);
		select_model = selected_modelinfo;
		TableWrite("选择汽车型号： " + select_model.model_name, 1);
		init_analyse_vector = deal_settingfile.AnalyseDiagnoseVehicleFile(selected_modelinfo.DiagnosisBased);
		if (init_analyse_vector.size() == 0)
		{
			TableWrite("配置文件解析失败！没有获得命令数据！", 5);
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

void DiagnosePage::TableWrite(QString message_content, int message_type)//处理消息
{
	if (message_type == 0)//错误信息背景色为红色
	{
		LOG_E << message_content;
	}
	else if (message_type == 1)//正常信息背景色为红色
	{
		LOG << message_content;
	}
	else if (message_type == 2)//错误信息，不在界面信息栏中显示，只在log文件中记录
	{
		LOG_E << message_content;
	}
	else if (message_type == 3)//正常信息，不在界面信息栏中显示，只在log文件中记录
	{
		LOG << message_content;
	}
	else if (message_type == 4)//正常信息，表格显示+Log记录+弹出提示
	{
		LOG << message_content;
		Global::Display_prompt(1, message_content);
	}
	else if (message_type == 5)//错误信息，表格显示+Log记录+弹出提示
	{
		LOG_E << message_content;
		Global::Display_prompt(0, message_content);
	}
}

void DiagnosePage::DiagnoseEnterSlot()//进入命令执行完毕，自动跳入下一个页面
{
	QString select_settingfile = QCoreApplication::applicationDirPath() + "/" + select_model.settingfile_path + "/" + "Diag_Set.txt";
	TableWrite("选中车型对应配置文件：" + select_settingfile, 3);
	deal_settingfile.settingfile_path = select_settingfile;
	QFile file(select_settingfile);//打开并读取配置文件
	QMap<int, QString> pagelist_map = deal_settingfile.GetPageList();

	if (pagelist_map.size() == 0)
	{
		TableWrite("配置文件解析失败！没有页面列表！", 5);
		return;
	}
	else
	{
		QVector<QString> total_listitem_vector;
		total_listitem_vector.append(QStringLiteral("控制器信息"));
		total_listitem_vector.append(QStringLiteral("实时数据"));
		total_listitem_vector.append(QStringLiteral("故障数据"));
		total_listitem_vector.append(QStringLiteral("通信控制"));
		total_listitem_vector.append(QStringLiteral("动作控制"));
		total_listitem_vector.append(QStringLiteral("例程控制"));
		total_listitem_vector.append(QStringLiteral("特殊功能"));

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

void DiagnosePage::ChangeModeSlot(int temp_row)//点击左侧列表，界面跳转
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
		temp_page->Init();//多态应用，都继承了Init()函数
	}
}

void DiagnosePage::startDataThread()//开始进行进入心跳线程
{
	boost::function0<void> f1 = boost::bind(&DiagnosePage::dealwith_sendreceive, this);
	data_thread = boost::thread(f1);
}

void DiagnosePage::dealwith_sendreceive()//进入心跳线程函数
{
	int re_send_item = 0;
	LoaderData current_command;
	bool re_receive_flag = false;
	bool DiagnoseEnter_flag = false;
	int no_receive_count = 0;
	data_thread_mutex.lock();
	while (CAN_open && ++thread_index < init_analyse_vector.size() && startthread_flag)//命令列表一条条执行，CAN设备打开，主线程没有发送停止信号
	{
		if (!re_receive_flag)current_command = init_analyse_vector[thread_index];
		send_EnterAndHeart_data(init_analyse_vector[thread_index].send_data);
		Get_Record();
		if (!receive_EnterAndHeart_data(current_command.send_data, init_analyse_vector[thread_index].receive_data))
		{
			if (no_receive_count >= 20)//当连续20次都没有收到数据说明出了问题，应该停止刷新
			{
				emit display_messageSignal("数据接收异常！请重启设备！", 5);
				no_receive_count = 0;
				break;
			}
			if (receivetime_count->elapsed() < 100)//时间小于100ms，继续接收
			{
				LoaderData tempdata1 = init_analyse_vector[thread_index];
				tempdata1.send_data.frame_id = 0;
				init_analyse_vector.replace(thread_index, tempdata1);
				thread_index--;
				re_receive_flag = true;	
			}
			else if (re_send_item < 2)//计时超过200ms，当次数小于3次时进行重复重新执行此条命令
			{
				init_analyse_vector.replace(thread_index, current_command);
				thread_index--;
				re_send_item++;//重复次数+1
				re_receive_flag = false;
				no_receive_count++;
			}
			else if (re_send_item == 2)//当第三次失败时，进行下一条命令
			{
				emit display_messageSignal(current_command.Caption_content + " 命令接收失败！", 2);
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
					if (current_thread_order != 0)heart_thread_condition.wait(data_thread_mutex);//这里是条件变量，一旦遇到其他线程进行数据收发，本线程机会休眠
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

void DiagnosePage::send_EnterAndHeart_data(SendData send_data)//发送进入心跳命令
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
		receivetime_count->restart();//计时开始
		QThread::msleep(5);
	}
}

QVector<BYTE> DiagnosePage::analyse_send_singleframe(QString data_content)//解析单帧
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
				if (temp_special_name == "Key")//key，根据seed计算
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

QVector<BYTE> DiagnosePage::ComputeKey(QVector<BYTE> seed)//计算key
{
	QVector<BYTE> temp_data;
	if (select_model.model_name == "车和家")
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
	else if (select_model.model_name == "新时空" || select_model.model_name == "新时空8849")
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
	else if (select_model.model_name.mid(0,2) == "众泰")
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
	else if (select_model.model_name.mid(0, 2) == "吉利")
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

bool DiagnosePage::receive_EnterAndHeart_data(SendData send_data, ReceiveDiagloseData temp_receive_data)//接收进入心跳命令
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
				if (send_data.data_analyse_flag && send_data.data_content != "")//这里考虑到有seed、key需要解析，所有这里必须加上
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
				if (EnterAndHeart_Record_Vector[m].this_frame.ID == select_receive_frame_id)//在接收数据帧中查找指定ID
				{
					if (temp_receive_data.byte_length == 8)
					{
						for (int n = 0; n < temp_receive_data.litte_record_vertor.size(); n++)
						{
							LitteRecord temp_little_record = temp_receive_data.litte_record_vertor[n];
							if (!temp_little_record.data_analyse_flag)//接收数据帧中不需要解析
							{
								for (int s = 0; s < temp_little_record.byte_length; s++)
								{
									if (EnterAndHeart_Record_Vector[m].this_frame.Data[s + temp_little_record.data_offset] == temp_little_record.hex_data[s])
									{
										same_byte_count++;//计算和配置文件中相同的字节数
									}
								}
								real_same_byte_count += temp_little_record.byte_length;
							}
						}
					}
					if (same_byte_count != real_same_byte_count)//接收与配置文件中总字节数是否相同
					{
						EnterAndHeart_Record_Vector.remove(0, m);
						emit display_messageSignal("对应接收的数据帧中值与配置文件不匹配！", 2);
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

									temp_byte_data.push_back(EnterAndHeart_Record_Vector[m].this_frame.Data[s + temp_little_record.data_offset]);
								}
								QString analyse_name = temp_little_record.data_content.mid(1, temp_little_record.data_content.length() - 2);
								receive_map_data.insert(analyse_name, temp_byte_data);//保存解析的数据和名称
							}
						}
						EnterAndHeart_Record_Vector.remove(0, m);
						emit display_messageSignal("对应接收的数据帧中值与配置文件完全一致，本条指令完成！", 3);
						return true;
					}
				}
			}
		}
		return false;
	}
}

void DiagnosePage::Get_Record()//筛选进入心跳对应的发送接收命令
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
		unsigned long len = VCI_Transmit(useful_device.device_type_value, useful_device.device_index, useful_device.channel_number, &frameinfo, 1);//发送数据
		if (len != 1)
		{
			TableWrite("数据帧发送未成功！", 2);
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
		try//因为出现过接收数据帧为255个字节的情况，这里特进行异常处理，防止数组溢出
		{
			len = VCI_Receive(useful_device.device_type_value, useful_device.device_index, useful_device.channel_number, frameinfo.get(), receive_length, 200);//读取的bufffer的长度不能超过VCI_CAN_OBJ数组的大小
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
		Global::Display_prompt(0, QStringLiteral("通信设置失败！"));
		return;
	}
	else
	{
		if (CAN_open)
		{
			VCI_CloseDevice(useful_device.device_type_value, useful_device.device_index);
			CAN_open = false;
			TableWrite("CAN设备已经打开，进行关闭！", 1);
			QThread::msleep(300);
		}

		useful_device = temp_commu_setting;
		
		if (CANDeviceInit(useful_device.device_type_value, useful_device.device_index, useful_device.channel_number, useful_device.baundrate))
		{
			TableWrite("打开CAN设备成功！", 1);
			startThread();
			TableWrite("开始进行CAN通信！", 1);
		}
		else
		{
			useful_device.Clear();
		}

		if (CAN_open)
		{
			QThread::msleep(200);
			TableWrite("开始进行诊断线程！", 1);
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

void DiagnosePage::close_windowSlot()//关闭按钮
{
	this->close();
}

void DiagnosePage::min_windowSlot()//最小化按钮
{
	setWindowState(Qt::WindowMinimized);
}

void DiagnosePage::mousePressEvent(QMouseEvent *event)//重写鼠标按下事件 
{

	//记录下鼠标相对于窗口的位置  
	//event->globalPos()鼠标按下时，鼠标相对于整个屏幕位置  
	//pos() this->pos()鼠标按下时，窗口相对于整个屏幕位置  
	QRect labelrect = QRect(ui.top_widget->pos(), ui.top_widget->size());
	if (labelrect.contains(event->pos()))
	{
		mMoveing = true;
		mMovePosition = event->globalPos() - pos();
	}

	return QMainWindow::mousePressEvent(event);
}

void DiagnosePage::mouseMoveEvent(QMouseEvent *event)//重写鼠标移动事件  
{
	//(event->buttons() && Qt::LeftButton)按下是左键  
	//鼠标移动事件需要移动窗口，窗口移动到哪里呢？就是要获取鼠标移动中，窗口在整个屏幕的坐标，然后move到这个坐标，怎么获取坐标？  
	//通过事件event->globalPos()知道鼠标坐标，鼠标坐标减去鼠标相对于窗口位置，就是窗口在整个屏幕的坐标  
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