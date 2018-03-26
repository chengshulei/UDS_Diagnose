#include "Prompt.h"
#include <QFile>

Prompt::Prompt(int temp_type, QString temp_message)
{
	ui.setupUi(this);

	prompt_type = temp_type;
	prompt_message = temp_message;

	this->setAttribute(Qt::WA_TranslucentBackground);//���ô��ڱ���͸��
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);//ȥ���߿�
	

	QString stylefile_path;
	if (prompt_type == 0)
	{
		stylefile_path = QCoreApplication::applicationDirPath() + "/qss/ErrorPrompt.qss";
		ui.prompt_title_label->setText(QStringLiteral("�� ��"));
	}
	else if (prompt_type == 1)
	{
		stylefile_path = QCoreApplication::applicationDirPath() + "/qss/NormalPrompt.qss";
		ui.prompt_title_label->setText(QStringLiteral("��ʾ"));
	}
	QFile qss(stylefile_path);
	if (qss.open(QFile::ReadOnly))
	{
		this->setStyleSheet(qss.readAll());
		qss.close();
	}
	
	
	ui.prompt_sure_pushButton->setText(QStringLiteral("ȷ ��"));
	ui.prompt_message_label->setText(prompt_message);
	ui.prompt_close_pushButton->setToolTip(QStringLiteral("�ر�"));


	connect(ui.prompt_close_pushButton, SIGNAL(clicked()), this, SLOT(close_windowSlot()));
	connect(ui.prompt_sure_pushButton, SIGNAL(clicked()), this, SLOT(close_windowSlot()));
}

Prompt::~Prompt()
{
}

void Prompt::close_windowSlot()//�رհ�ť
{
	this->close();
}

void Prompt::mousePressEvent(QMouseEvent *event)//��д��갴���¼� 
{

	//��¼���������ڴ��ڵ�λ��  
	//event->globalPos()��갴��ʱ����������������Ļλ��  
	//pos() this->pos()��갴��ʱ�����������������Ļλ��  
	QRect labelrect = QRect(ui.prompt_top_widget->pos(), ui.prompt_top_widget->size());
	if (labelrect.contains(event->pos()))
	{
		mMoveing = true;
		mMovePosition = event->globalPos() - pos();
	}

	return QDialog::mousePressEvent(event);
}


void Prompt::mouseMoveEvent(QMouseEvent *event)//��д����ƶ��¼�  
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
void Prompt::mouseReleaseEvent(QMouseEvent *event)
{
	mMoveing = false;
}

