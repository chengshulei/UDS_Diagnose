#include "SelectModelPage.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QToolButton>
#include "DealSettingFile.h"

#define ColumnNumber 3

extern DealSettingFile deal_settingfile;

SelectModelPage::SelectModelPage(QDialog *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	deal_settingfile.vehiclefile_path = QCoreApplication::applicationDirPath() + "/" + ("VehicleSelect.txt");
	TableWrite("ƽ̨�����ļ��� " + deal_settingfile.vehiclefile_path, 3);
	readfile_info = deal_settingfile.GetVehicleModel();
	Init();
	Global::loadQSS(this, "SelectModel.qss");
}

SelectModelPage::~SelectModelPage()
{
}

void SelectModelPage::Init()//�������ļ��еĳ������ã�ÿ�����ʹ���һ����ť
{
	if (readfile_info.size() > 0)
	{
		QString temp_modelfilepath = QCoreApplication::applicationDirPath();
		QVBoxLayout *mainLayout = new QVBoxLayout;
		for (int i = 0; i <= readfile_info.size() / ColumnNumber; i++)
		{
			QHBoxLayout* temp_hlayout = new QHBoxLayout;
			for (int j = 0; j < ColumnNumber && i * ColumnNumber + j < readfile_info.size(); j++)
			{
				QSpacerItem *temp_horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
				QToolButton* temp_button = new QToolButton;
				temp_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
				const QPixmap temp_pix(temp_modelfilepath + "/" + readfile_info[i *ColumnNumber + j].picture_path);
				const QSize temp_size(temp_pix.size().width(), temp_pix.size().height());
				QIcon temp_ico = QIcon(temp_pix);
				temp_button->setIcon(temp_ico);
				temp_button->setIconSize(temp_size);
				temp_button->setText(readfile_info[i *ColumnNumber + j].model_name);
				temp_hlayout->addItem(temp_horizontalSpacer);
				temp_hlayout->addWidget(temp_button);
				connect(temp_button, SIGNAL(clicked()), this, SLOT(SelectModelSlot()));
				temp_button->setToolTip(QStringLiteral("����ѡ���ͣ�"));
			}
			QSpacerItem *lineend_horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
			temp_hlayout->addItem(lineend_horizontalSpacer);
			mainLayout->addLayout(temp_hlayout);
		}
		this->setLayout(mainLayout);
	}
}

void SelectModelPage::SelectModelSlot()//����ѡ���ͼƬ�ҵ���Ӧ�������ļ�
{
	QObject* temp_obj = sender();
	QString selectmodel_name = ((QToolButton*)temp_obj)->text();
	for (int i = 0; i <= readfile_info.size(); i++)
	{
		if (selectmodel_name == readfile_info[i].model_name)
		{
			ModelInfo temp_info = readfile_info[i];
			emit selectmodelsignal(temp_info);//���ҵ��������ļ�����·�����ݳ�ȥ
			break;
		}
	}
}

void SelectModelPage::TableWrite(QString message_content, int message_type)//����������Ϣ����
{
	emit display_messageSignal(message_content, message_type);
}