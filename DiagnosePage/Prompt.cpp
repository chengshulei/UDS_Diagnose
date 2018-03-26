#include "Prompt.h"
#include <QFile>

Prompt::Prompt(int temp_type, QString temp_message)
{
	ui.setupUi(this);

	prompt_type = temp_type;
	prompt_message = temp_message;

	this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);//去掉边框
	

	QString stylefile_path;
	if (prompt_type == 0)
	{
		stylefile_path = QCoreApplication::applicationDirPath() + "/qss/ErrorPrompt.qss";
		ui.prompt_title_label->setText(QStringLiteral("错 误"));
	}
	else if (prompt_type == 1)
	{
		stylefile_path = QCoreApplication::applicationDirPath() + "/qss/NormalPrompt.qss";
		ui.prompt_title_label->setText(QStringLiteral("提示"));
	}
	QFile qss(stylefile_path);
	if (qss.open(QFile::ReadOnly))
	{
		this->setStyleSheet(qss.readAll());
		qss.close();
	}
	
	
	ui.prompt_sure_pushButton->setText(QStringLiteral("确 定"));
	ui.prompt_message_label->setText(prompt_message);
	ui.prompt_close_pushButton->setToolTip(QStringLiteral("关闭"));


	connect(ui.prompt_close_pushButton, SIGNAL(clicked()), this, SLOT(close_windowSlot()));
	connect(ui.prompt_sure_pushButton, SIGNAL(clicked()), this, SLOT(close_windowSlot()));
}

Prompt::~Prompt()
{
}

void Prompt::close_windowSlot()//关闭按钮
{
	this->close();
}

void Prompt::mousePressEvent(QMouseEvent *event)//重写鼠标按下事件 
{

	//记录下鼠标相对于窗口的位置  
	//event->globalPos()鼠标按下时，鼠标相对于整个屏幕位置  
	//pos() this->pos()鼠标按下时，窗口相对于整个屏幕位置  
	QRect labelrect = QRect(ui.prompt_top_widget->pos(), ui.prompt_top_widget->size());
	if (labelrect.contains(event->pos()))
	{
		mMoveing = true;
		mMovePosition = event->globalPos() - pos();
	}

	return QDialog::mousePressEvent(event);
}


void Prompt::mouseMoveEvent(QMouseEvent *event)//重写鼠标移动事件  
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
void Prompt::mouseReleaseEvent(QMouseEvent *event)
{
	mMoveing = false;
}

