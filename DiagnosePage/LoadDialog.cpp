#include "LoadDialog.h"
#include <QMovie>
#include "Global.h"
#include <QLibrary>

LoadDialog::LoadDialog()
{
	ui.setupUi(this);

	this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);//去掉边框
	ui.title_label->setText(QStringLiteral("加载中，请等待..."));

	QMovie* movie = new QMovie(QCoreApplication::applicationDirPath() + "/image/loading.gif");//加载等待动画
	ui.label->setMovie(movie);
	movie->start();

	Global::loadQSS(this, "loading.qss");
}

LoadDialog::~LoadDialog()
{
}
