#include "CommunicationPage.h"

CommunicationPage::CommunicationPage(ModelInfo temp_model)
{
	ui.setupUi(this);


	Global::loadQSS(this, "Communication.qss");

	this->setAttribute(Qt::WA_TranslucentBackground);//���ô��ڱ���͸��
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);//ȥ���߿�

	ui.communication_close_pushButton->setToolTip(QStringLiteral("�ر�"));
	ui.communication_title_label->setToolTip(QStringLiteral("ͨ������"));

	connect(ui.communication_close_pushButton, SIGNAL(clicked()), this, SLOT(close_windowSlot()));

	select_model = temp_model;
	sure_flag = false;

	setWindowTitle(select_model.model_name);
	setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);//�������

	QStringList temp_data;

	ui.device_type_label->setText(QStringLiteral("�豸���ͣ�"));
	ui.device_type_content->setText("VCI_USBCAN2");

	ui.baudrate_label->setText(QStringLiteral("�����ʣ�"));
	ui.baudrate_content->setText(temp_model.Baud_Setting);

	for (int i = 0; i < 3; i++)
	{
		temp_data << QString::number(i);
	}
	ui.device_index_comboBox->addItems(temp_data);
	ui.device_index_label->setText(QStringLiteral("�豸�����ţ�"));
	temp_data.clear();

	for (int i = 0; i < 3; i++)
	{
		temp_data << QString::number(i);
	}
	ui.can_channel_comboBox->addItems(temp_data);
	ui.can_channel_label->setText(QStringLiteral("CANͨ���ţ�"));
	temp_data.clear();

	ui.commucation_sure_pushButton->setText(QStringLiteral("ȷ ��"));

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

void CommunicationPage::close_windowSlot()//�رհ�ť
{
	this->close();
}

void CommunicationPage::mousePressEvent(QMouseEvent *event)//��д��갴���¼� 
{

	//��¼���������ڴ��ڵ�λ��  
	//event->globalPos()��갴��ʱ����������������Ļλ��  
	//pos() this->pos()��갴��ʱ�����������������Ļλ��  
	QRect labelrect = QRect(ui.communication_top_widget->pos(), ui.communication_top_widget->size());
	if (labelrect.contains(event->pos()))
	{
		mMoveing = true;
		mMovePosition = event->globalPos() - pos();
	}

	return QDialog::mousePressEvent(event);
}

void CommunicationPage::mouseMoveEvent(QMouseEvent *event)//��д����ƶ��¼�  
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
	return QDialog::mouseMoveEvent(event);
}
void CommunicationPage::mouseReleaseEvent(QMouseEvent *event)
{
	mMoveing = false;
}


