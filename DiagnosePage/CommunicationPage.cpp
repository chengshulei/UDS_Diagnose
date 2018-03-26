#include "CommunicationPage.h"

CommunicationPage::CommunicationPage(ModelInfo temp_model)
{
	ui.setupUi(this);


	Global::loadQSS(this, "Communication.qss");

	this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);//去掉边框

	ui.communication_close_pushButton->setToolTip(QStringLiteral("关闭"));
	ui.communication_title_label->setToolTip(QStringLiteral("通信设置"));

	connect(ui.communication_close_pushButton, SIGNAL(clicked()), this, SLOT(close_windowSlot()));

	select_model = temp_model;
	sure_flag = false;

	setWindowTitle(select_model.model_name);
	setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);//禁用最大化

	QStringList temp_data;

	ui.device_type_label->setText(QStringLiteral("设备类型："));
	ui.device_type_content->setText("VCI_USBCAN2");

	ui.baudrate_label->setText(QStringLiteral("波特率："));
	ui.baudrate_content->setText(temp_model.Baud_Setting);

	for (int i = 0; i < 3; i++)
	{
		temp_data << QString::number(i);
	}
	ui.device_index_comboBox->addItems(temp_data);
	ui.device_index_label->setText(QStringLiteral("设备索引号："));
	temp_data.clear();

	for (int i = 0; i < 3; i++)
	{
		temp_data << QString::number(i);
	}
	ui.can_channel_comboBox->addItems(temp_data);
	ui.can_channel_label->setText(QStringLiteral("CAN通道号："));
	temp_data.clear();

	ui.commucation_sure_pushButton->setText(QStringLiteral("确 定"));

	connect(ui.commucation_sure_pushButton, SIGNAL(clicked()), this, SLOT(Sure_Slot()));
}

CommunicationPage::~CommunicationPage()
{
}

void CommunicationPage::Sure_Slot()
{
	Exist_Device current_communication_setting;
	QString temp_device_type = ui.device_type_content->text();
	current_communication_setting.device_type_value = Global::GetDeviceTypeValue(temp_device_type);
	current_communication_setting.baundrate = ui.baudrate_content->text();
	current_communication_setting.device_index = ui.device_index_comboBox->currentIndex();
	current_communication_setting.channel_number = ui.can_channel_comboBox->currentIndex();
	emit communication_resultSignal(current_communication_setting);
	sure_flag = true;
	this->close();
}

void CommunicationPage::closeEvent(QCloseEvent * event)
{
	if (!sure_flag)
	{
		Exist_Device current_communication_setting;
		emit communication_resultSignal(current_communication_setting);
	}
}

void CommunicationPage::close_windowSlot()//关闭按钮
{
	this->close();
}

void CommunicationPage::mousePressEvent(QMouseEvent *event)//重写鼠标按下事件 
{

	//记录下鼠标相对于窗口的位置  
	//event->globalPos()鼠标按下时，鼠标相对于整个屏幕位置  
	//pos() this->pos()鼠标按下时，窗口相对于整个屏幕位置  
	QRect labelrect = QRect(ui.communication_top_widget->pos(), ui.communication_top_widget->size());
	if (labelrect.contains(event->pos()))
	{
		mMoveing = true;
		mMovePosition = event->globalPos() - pos();
	}

	return QDialog::mousePressEvent(event);
}

void CommunicationPage::mouseMoveEvent(QMouseEvent *event)//重写鼠标移动事件  
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
	return QDialog::mouseMoveEvent(event);
}
void CommunicationPage::mouseReleaseEvent(QMouseEvent *event)
{
	mMoveing = false;
}


