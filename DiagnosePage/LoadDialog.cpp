#include "LoadDialog.h"
#include <QMovie>
#include "Global.h"
#include <QLibrary>

LoadDialog::LoadDialog()
{
	ui.setupUi(this);

	this->setAttribute(Qt::WA_TranslucentBackground);//���ô��ڱ���͸��
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);//ȥ���߿�
	ui.title_label->setText(QStringLiteral("�����У���ȴ�..."));

	QMovie* movie = new QMovie(QCoreApplication::applicationDirPath() + "/image/loading.gif");//���صȴ�����
	ui.label->setMovie(movie);
	movie->start();

	Global::loadQSS(this, "loading.qss");
}

LoadDialog::~LoadDialog()
{
}
