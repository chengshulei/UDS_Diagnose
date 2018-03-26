
#include "WriteECUPage.h"



WriteECUPage::WriteECUPage(QVector<WriteECUData> temp_vector, QMap<QString, QVector<BYTE> > temp_map, ECUData temp_item_data)
{
	ui.setupUi(this);

	Global::loadQSS(this, "writeecu.qss");

	this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);//去掉边框

	ui.close_pushButton->setToolTip(QStringLiteral("关闭"));//按钮提示

	sure_flag = false;

	ECUinfo_data = temp_item_data;
	content_map = temp_map;
	current_wirtedata_vector = temp_vector;

	name_change_list.insert("Year", QStringLiteral("年"));
	name_change_list.insert("Month", QStringLiteral("月"));
	name_change_list.insert("Day", QStringLiteral("日"));
	name_change_list.insert("VinCode", QStringLiteral("VIN码"));
	name_change_list.insert("FacCode", QStringLiteral("厂商编码"));
	name_change_list.insert("HardVer", QStringLiteral("硬件版本号"));
	name_change_list.insert("SoftVer", QStringLiteral("软件版本号"));
	name_change_list.insert("HardYearMonthDay", QStringLiteral("硬件版本号写入日期"));
	name_change_list.insert("SoftYearMonthDay", QStringLiteral("软件版本号写入日期"));

	ui.title_label->setText(QStringLiteral("写控制器信息"));

	setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);//禁用最大化
	AnalyseWriteECUData();

	connect(ui.close_pushButton, SIGNAL(clicked()), this, SLOT(close_windowSlot()));//按钮实现
}

WriteECUPage::~WriteECUPage()
{
}

void WriteECUPage::AnalyseWriteECUData()
{
	QVBoxLayout *mainLayout = new QVBoxLayout;
	QString temp_special_name = ECUinfo_data.store_name;
	if (temp_special_name == "VehicleDate")//车辆生成日期，这是车和家独有的
	{
		QString temp_label_name;

		QHBoxLayout* temp_hlayout = new QHBoxLayout;
		QSpacerItem *temp_horizontalSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QLabel* temp_label = new QLabel;
		temp_label->setText(QStringLiteral("年") + ":");
		QLineEdit* temp_linetext = new QLineEdit;
		temp_linetext->setText(ECUinfo_data.ECUinfo_value.mid(0, 4));
		QSpacerItem *temp_horizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		temp_linetext->setFixedHeight(30);
		temp_linetext->setStyleSheet("\
									QLineEdit{\
									border: 1px solid #9d9087;\
									border-radius:3px;\
									background:white;\
									selection-background-color:green;\
									font-size:14px;}\
									QLineEdit:hover{\
									border: 1px solid blue;\
									}");
		temp_label->setStyleSheet("QLabel{font:Bold 20px;font-family: 仿宋;}");


		temp_hlayout->addItem(temp_horizontalSpacer1);
		temp_hlayout->addWidget(temp_label);
		temp_hlayout->addWidget(temp_linetext);
		temp_hlayout->addItem(temp_horizontalSpacer2);
		mainLayout->addLayout(temp_hlayout);

		QHBoxLayout* temp_hlayout2 = new QHBoxLayout;
		QSpacerItem *temp_horizontalSpacer3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QLabel* temp_label2 = new QLabel;
		temp_label2->setText(QStringLiteral("月") + ":");
		QLineEdit* temp_linetext2 = new QLineEdit;
		temp_linetext2->setText(ECUinfo_data.ECUinfo_value.mid(4, 2));
		QSpacerItem *temp_horizontalSpacer4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		temp_linetext->setFixedHeight(30);
		temp_linetext2->setStyleSheet("\
									QLineEdit{\
									border: 1px solid #9d9087;\
									border-radius:3px;\
									background:white;\
									selection-background-color:green;\
									font-size:14px;}\
									QLineEdit:hover{\
									border: 1px solid blue;\
									}");
		temp_label2->setStyleSheet("QLabel{font:Bold 20px;font-family: 仿宋;}");

		temp_hlayout2->addItem(temp_horizontalSpacer3);
		temp_hlayout2->addWidget(temp_label2);
		temp_hlayout2->addWidget(temp_linetext2);
		temp_hlayout2->addItem(temp_horizontalSpacer4);
		mainLayout->addLayout(temp_hlayout2);

		QHBoxLayout* temp_hlayout3 = new QHBoxLayout;
		QSpacerItem *temp_horizontalSpacer5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QLabel* temp_label3 = new QLabel;
		temp_label3->setText(QStringLiteral("日") + ":");
		QLineEdit* temp_linetext3 = new QLineEdit;
		temp_linetext3->setText(ECUinfo_data.ECUinfo_value.mid(6, 2));
		QSpacerItem *temp_horizontalSpacer6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		temp_linetext->setFixedHeight(30);
		temp_linetext3->setStyleSheet("\
									QLineEdit{\
									border: 1px solid #9d9087;\
									border-radius:3px;\
									background:white;\
									selection-background-color:green;\
									font-size:14px;}\
									QLineEdit:hover{\
									border: 1px solid blue;\
									}");
		temp_label3->setStyleSheet("QLabel{font:Bold 20px;font-family: 仿宋;}");

		temp_hlayout3->addItem(temp_horizontalSpacer5);
		temp_hlayout3->addWidget(temp_label3);
		temp_hlayout3->addWidget(temp_linetext3);
		temp_hlayout3->addItem(temp_horizontalSpacer6);
		mainLayout->addLayout(temp_hlayout3);
	}
	if (temp_special_name == "VinCode")//天，当前值
	{
		QString temp_label_name;
		QMap<QString, QString>::iterator  mi;
		for (mi = name_change_list.begin(); mi != name_change_list.end(); ++mi)
		{
			if (mi.key() == temp_special_name)
			{
				temp_label_name = mi.value();
				break;
			}
		}
		QHBoxLayout* temp_hlayout = new QHBoxLayout;
		QSpacerItem *temp_horizontalSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QLabel* temp_label = new QLabel;
		temp_label->setText(temp_label_name + ":");
		QLineEdit* temp_linetext = new QLineEdit;
		temp_linetext->setText(ECUinfo_data.ECUinfo_value);
		QSpacerItem *temp_horizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		temp_linetext->setFixedHeight(30);
		temp_linetext->setStyleSheet("\
									QLineEdit{\
									border: 1px solid #9d9087;\
									border-radius:3px;\
									background:white;\
									selection-background-color:green;\
									font-size:14px;}\
									QLineEdit:hover{\
									border: 1px solid blue;\
									}");
		temp_label->setStyleSheet("QLabel{font:Bold 20px;font-family: 仿宋;}");

		temp_hlayout->addItem(temp_horizontalSpacer1);
		temp_hlayout->addWidget(temp_label);
		temp_hlayout->addWidget(temp_linetext);
		temp_hlayout->addItem(temp_horizontalSpacer2);
		mainLayout->addLayout(temp_hlayout);
	}
	if (temp_special_name == "FacCode")//厂商编码
	{
		QString temp_label_name;
		QMap<QString, QString>::iterator  mi;
		for (mi = name_change_list.begin(); mi != name_change_list.end(); ++mi)
		{
			if (mi.key() == temp_special_name)
			{
				temp_label_name = mi.value();
				break;
			}
		}
		QHBoxLayout* temp_hlayout = new QHBoxLayout;
		QSpacerItem *temp_horizontalSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QLabel* temp_label = new QLabel;
		temp_label->setText(temp_label_name + ":");
		QLineEdit* temp_linetext = new QLineEdit;
		temp_linetext->setText(ECUinfo_data.ECUinfo_value);
		QSpacerItem *temp_horizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		temp_linetext->setFixedHeight(30);
		temp_linetext->setStyleSheet("\
									QLineEdit{\
									border: 1px solid #9d9087;\
									border-radius:3px;\
									background:white;\
									selection-background-color:green;\
									font-size:14px;}\
									QLineEdit:hover{\
									border: 1px solid blue;\
									}");
		temp_label->setStyleSheet("QLabel{font:Bold 20px;font-family: 仿宋;}");

		temp_hlayout->addItem(temp_horizontalSpacer1);
		temp_hlayout->addWidget(temp_label);
		temp_hlayout->addWidget(temp_linetext);
		temp_hlayout->addItem(temp_horizontalSpacer2);
		mainLayout->addLayout(temp_hlayout);
	}
	if (temp_special_name == "HardVer")//硬件版本号
	{
		QString temp_label_name;
		QMap<QString, QString>::iterator  mi;
		for (mi = name_change_list.begin(); mi != name_change_list.end(); ++mi)
		{
			if (mi.key() == temp_special_name)
			{
				temp_label_name = mi.value();
				break;
			}
		}
		QHBoxLayout* temp_hlayout = new QHBoxLayout;
		QSpacerItem *temp_horizontalSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QLabel* temp_label = new QLabel;
		temp_label->setText(temp_label_name + ":");
		QLineEdit* temp_linetext = new QLineEdit;
		temp_linetext->setText(ECUinfo_data.ECUinfo_value);
		QSpacerItem *temp_horizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		temp_linetext->setFixedHeight(30);
		temp_linetext->setStyleSheet("\
									QLineEdit{\
									border: 1px solid #9d9087;\
									border-radius:3px;\
									background:white;\
									selection-background-color:green;\
									font-size:14px;}\
									QLineEdit:hover{\
									border: 1px solid blue;\
									}");
		temp_label->setStyleSheet("QLabel{font:Bold 20px;font-family: 仿宋;}");

		temp_hlayout->addItem(temp_horizontalSpacer1);
		temp_hlayout->addWidget(temp_label);
		temp_hlayout->addWidget(temp_linetext);
		temp_hlayout->addItem(temp_horizontalSpacer2);
		mainLayout->addLayout(temp_hlayout);
	}

	if (temp_special_name == "SoftVer")//软件版本号
	{
		QString temp_label_name;
		QMap<QString, QString>::iterator  mi;
		for (mi = name_change_list.begin(); mi != name_change_list.end(); ++mi)
		{
			if (mi.key() == temp_special_name)
			{
				temp_label_name = mi.value();
				break;
			}
		}
		QHBoxLayout* temp_hlayout = new QHBoxLayout;
		QSpacerItem *temp_horizontalSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QLabel* temp_label = new QLabel;
		temp_label->setText(temp_label_name + ":");
		QLineEdit* temp_linetext = new QLineEdit;
		temp_linetext->setText(ECUinfo_data.ECUinfo_value);
		QSpacerItem *temp_horizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		temp_linetext->setFixedHeight(30);
		temp_linetext->setStyleSheet("\
									QLineEdit{\
									border: 1px solid #9d9087;\
									border-radius:3px;\
									background:white;\
									selection-background-color:green;\
									font-size:14px;}\
									QLineEdit:hover{\
									border: 1px solid blue;\
									}");
		temp_label->setStyleSheet("QLabel{font:Bold 20px;font-family: 仿宋;}");

		temp_hlayout->addItem(temp_horizontalSpacer1);
		temp_hlayout->addWidget(temp_label);
		temp_hlayout->addWidget(temp_linetext);
		temp_hlayout->addItem(temp_horizontalSpacer2);
		mainLayout->addLayout(temp_hlayout);
	}
	if (temp_special_name == "HardYearMonthDay")//硬件版本号写入日期
	{
		QString temp_label_name;
		QMap<QString, QString>::iterator  mi;
		for (mi = name_change_list.begin(); mi != name_change_list.end(); ++mi)
		{
			if (mi.key() == temp_special_name)
			{
				temp_label_name = mi.value();
				break;
			}
		}
		QHBoxLayout* temp_hlayout = new QHBoxLayout;
		QSpacerItem *temp_horizontalSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QLabel* temp_label = new QLabel;
		temp_label->setText(temp_label_name + ":");
		QLineEdit* temp_linetext = new QLineEdit;
		temp_linetext->setText(ECUinfo_data.ECUinfo_value);
		QSpacerItem *temp_horizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		temp_linetext->setFixedHeight(30);
		temp_linetext->setStyleSheet("\
									QLineEdit{\
									border: 1px solid #9d9087;\
									border-radius:3px;\
									background:white;\
									selection-background-color:green;\
									font-size:14px;}\
									QLineEdit:hover{\
									border: 1px solid blue;\
									}");
		temp_label->setStyleSheet("QLabel{font:Bold 20px;font-family: 仿宋;}");

		temp_hlayout->addItem(temp_horizontalSpacer1);
		temp_hlayout->addWidget(temp_label);
		temp_hlayout->addWidget(temp_linetext);
		temp_hlayout->addItem(temp_horizontalSpacer2);
		mainLayout->addLayout(temp_hlayout);
	}
	if (temp_special_name == "SoftYearMonthDay")//软件版本号写入日期
	{
		QString temp_label_name;
		QMap<QString, QString>::iterator  mi;
		for (mi = name_change_list.begin(); mi != name_change_list.end(); ++mi)
		{
			if (mi.key() == temp_special_name)
			{
				temp_label_name = mi.value();
				break;
			}
		}
		QHBoxLayout* temp_hlayout = new QHBoxLayout;
		QSpacerItem *temp_horizontalSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QLabel* temp_label = new QLabel;
		temp_label->setText(temp_label_name + ":");
		QLineEdit* temp_linetext = new QLineEdit;
		temp_linetext->setText(ECUinfo_data.ECUinfo_value);
		QSpacerItem *temp_horizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		temp_linetext->setFixedHeight(30);
		temp_linetext->setStyleSheet("\
									QLineEdit{\
									border: 1px solid #9d9087;\
									border-radius:3px;\
									background:white;\
									selection-background-color:green;\
									font-size:14px;}\
									QLineEdit:hover{\
									border: 1px solid blue;\
									}");
		temp_label->setStyleSheet("QLabel{font:Bold 20px;font-family: 仿宋;}");

		temp_hlayout->addItem(temp_horizontalSpacer1);
		temp_hlayout->addWidget(temp_label);
		temp_hlayout->addWidget(temp_linetext);
		temp_hlayout->addItem(temp_horizontalSpacer2);
		mainLayout->addLayout(temp_hlayout);
	}
	QSpacerItem *temp_horizontalSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	QPushButton* temp_button1 = new QPushButton;
	temp_button1->setText(QStringLiteral("确定"));
	QSpacerItem *temp_horizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	QHBoxLayout* temp_button_hlayout = new QHBoxLayout;
	temp_button_hlayout->addItem(temp_horizontalSpacer1);
	temp_button_hlayout->addWidget(temp_button1);
	temp_button_hlayout->addItem(temp_horizontalSpacer2);
	connect(temp_button1, SIGNAL(clicked()), this, SLOT(Sure_Slot()));

	temp_button1->setStyleSheet("\
								QPushButton {\
								background-color: #A4D3EE;\
								border-style: outset;\
								border-width:1px;\
								border-radius:5px;\
								border-color: #dddddd;\
								font: bold 20px;\
								font-family:仿宋;\
								min-width:10em;\
								padding:6px;\
								}\
								QPushButton:pressed{\
								background-color: #9AC0CD;\
								border-style: inset;\
								}");

	mainLayout->addLayout(temp_button_hlayout);
	ui.content_widget->setLayout(mainLayout);
}

void WriteECUPage::Sure_Slot()
{
	QMap<QString, QVector<BYTE> > input_byte_map;
	QVBoxLayout* mainlayout = (QVBoxLayout*)ui.content_widget->layout();
	if (ECUinfo_data.store_name == "VehicleDate")//车辆生成日期，这是车和家独有的
	{
		QMap<QString, QString> temp_list;
		temp_list.insert(QStringLiteral("年"), "Year");
		temp_list.insert(QStringLiteral("月"), "Month");
		temp_list.insert(QStringLiteral("日"), "Day");
		QVector<BYTE> temp_input_vector;
		for (int i = 0; i < mainlayout->count() - 1; i++)
		{
			QHBoxLayout* temp_hlayout = (QHBoxLayout*)mainlayout->itemAt(i)->layout();
			QLabel* temp_label = (QLabel*)temp_hlayout->itemAt(1)->widget();
			QString label_text = temp_label->text();
			label_text.chop(1);

			QString original_name;

			QMap<QString, QString>::iterator  mi;
			for (mi = temp_list.begin(); mi != temp_list.end(); ++mi)
			{
				if (mi.key() == label_text)
				{
					original_name = mi.value();
					break;
				}
			}
			if (original_name == "Year")
			{
				QLineEdit* temp_text = (QLineEdit*)temp_hlayout->itemAt(2)->widget();
				QString input_data = temp_text->text();
				if (input_data.length() != 4)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入长度错误！输入长度为4位！"));
					return;
				}
				for (int j = 0; j < input_data.length(); j++)
				{
					if (input_data.at(j) < '0' || input_data.at(j) > '9')
					{
						Global::Display_prompt(0, label_text + QStringLiteral(" 输入格式错误！"));
						return;
					}
				}
				temp_input_vector.push_back((BYTE)(input_data.toInt() - 2000));
			}
			if (original_name == "Month")
			{
				QLineEdit* temp_text = (QLineEdit*)temp_hlayout->itemAt(2)->widget();
				QString input_data = temp_text->text();
				if (input_data.length() != 2)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入长度错误！输入长度为2位！"));
					return;
				}
				for (int j = 0; j < input_data.length(); j++)
				{
					if (input_data.at(j) < '0' || input_data.at(j) > '9')
					{
						Global::Display_prompt(0, label_text + QStringLiteral(" 输入格式错误！"));
						return;
					}
				}
				if (input_data.toInt() < 1 || input_data.toInt() > 12)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入范围错误！"));
					return;
				}
				temp_input_vector.push_back((BYTE)(input_data.toInt()));
			}
			if (original_name == "Day")
			{
				QLineEdit* temp_text = (QLineEdit*)temp_hlayout->itemAt(2)->widget();
				QString input_data = temp_text->text();
				if (input_data.length() != 2)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入长度错误！输入长度为2位！"));
					return;
				}
				for (int j = 0; j < input_data.length(); j++)
				{
					if (input_data.at(j) < '0' || input_data.at(j) > '9')
					{
						Global::Display_prompt(0, label_text + QStringLiteral(" 输入格式错误！"));
						return;
					}
				}
				if (input_data.toInt() < 1 || input_data.toInt() > 31)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入范围错误！"));
					return;
				}
				temp_input_vector.push_back((BYTE)(input_data.toInt()));
			}
		}
		input_byte_map.insert(ECUinfo_data.store_name, temp_input_vector);
	}
	else
	{
		for (int i = 0; i < mainlayout->count() - 1; i++)
		{
			QHBoxLayout* temp_hlayout = (QHBoxLayout*)mainlayout->itemAt(i)->layout();
			QLabel* temp_label = (QLabel*)temp_hlayout->itemAt(1)->widget();
			QString label_text = temp_label->text();
			label_text.chop(1);

			QString original_name;

			QMap<QString, QString>::iterator  mi;
			for (mi = name_change_list.begin(); mi != name_change_list.end(); ++mi)
			{
				if (mi.value() == label_text)
				{
					original_name = mi.key();
					break;
				}
			}
			if (original_name == "VinCode")
			{
				QLineEdit* temp_text = (QLineEdit*)temp_hlayout->itemAt(2)->widget();
				QString input_data = temp_text->text();
				input_data = input_data.toUpper();
				if (input_data.length() != 17)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入长度错误！输入长度为17位！"));
					return;
				}
				for (int j = 0; j < input_data.length(); j++)
				{
					if (!(input_data.at(j) >= '0' && input_data.at(j) <= '9' || input_data.at(j) >= 'A' && input_data.at(j) <= 'Z'))
					{
						Global::Display_prompt(0, label_text + QStringLiteral(" 输入格式错误！"));
						return;
					}
				}
				QVector<BYTE> temp_input_vector;
				for (int j = 0; j < input_data.length(); j++)
				{
					if (j < 3)
					{
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
					else if (j == 3)
					{
						input_byte_map.insert("VinCode1", temp_input_vector);
						temp_input_vector.clear();
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
					else if (j < 10)
					{
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
					else if (j == 10)
					{
						input_byte_map.insert("VinCode2", temp_input_vector);
						temp_input_vector.clear();
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
					else
					{
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
				}
				input_byte_map.insert("VinCode3", temp_input_vector);
			}
			if (original_name == "FacCode")
			{
				QLineEdit* temp_text = (QLineEdit*)temp_hlayout->itemAt(2)->widget();
				QString input_data = temp_text->text();
				input_data = input_data.toUpper();
				if (input_data.length() != 9)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入长度错误！输入长度为9位！"));
					return;
				}
				if (input_data != "010119101" && input_data != "020104102")
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入错误！\r\n厂商编号输入范围：010119101或020104102！"));
					return;
				}
				QVector<BYTE> temp_input_vector;
				for (int j = 0; j < input_data.length(); j++)
				{
					if (j < 3)
					{
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
					else if (j == 3)
					{
						input_byte_map.insert("FacCode1", temp_input_vector);
						temp_input_vector.clear();
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
					else
					{
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
				}
				input_byte_map.insert("FacCode2", temp_input_vector);
				QMap<QString, QVector<BYTE> >::iterator  mi;
				for (mi = content_map.begin(); mi != content_map.end(); ++mi)
				{
					if (mi.key() == "HardVer")
					{
						QVector<BYTE> temp_byte_vector = mi.value();
						input_byte_map.insert("HardVer1", temp_byte_vector.mid(0, 1));
						input_byte_map.insert("HardVer2", temp_byte_vector.mid(1, 2));
					}
					if (mi.key() == "SoftVer")
					{
						QVector<BYTE> temp_byte_vector = mi.value();
						input_byte_map.insert("SoftVer", temp_byte_vector.mid(0, 3));
					}
				}
			}
			if (original_name == "HardVer")
			{
				QLineEdit* temp_text = (QLineEdit*)temp_hlayout->itemAt(2)->widget();
				QString input_data = temp_text->text();
				input_data = input_data.toUpper();
				if (input_data.length() != 3)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入长度错误！输入长度为3位！"));
					return;
				}
				if (input_data.at(0) != 'H')
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入错误！\r\n输入范围：H00~H99或HAA~HZZ！"));
					return;
				}
				int flag1 = 0;
				int flag2 = 0;
				for (int j = 1; j < 3; j++)
				{
					if (j == 1)
					{
						if (input_data.at(j) >= '0' && input_data.at(j) <= '9')
						{
							flag1 = 1;
						}
						if (input_data.at(j) >= 'A' && input_data.at(j) <= 'Z')
						{
							flag1 = 2;
						}
					}
					if (j == 2)
					{
						if (input_data.at(j) >= '0' && input_data.at(j) <= '9')
						{
							flag2 = 1;
						}
						if (input_data.at(j) >= 'A' && input_data.at(j) <= 'Z')
						{
							flag2 = 2;
						}
					}
				}
				if (flag1 == 0 || flag2 == 0 || flag1 != flag2)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入错误！\r\n输入范围：H00~H99或HAA~HZZ！"));
					return;
				}
				QVector<BYTE> temp_input_vector;
				for (int j = 0; j < input_data.length(); j++)
				{
					if (j == 0)
					{
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
						input_byte_map.insert("HardVer1", temp_input_vector);
						temp_input_vector.clear();
					}
					else
					{
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
				}
				input_byte_map.insert("HardVer2", temp_input_vector);
				QMap<QString, QVector<BYTE> >::iterator  mi;
				for (mi = content_map.begin(); mi != content_map.end(); ++mi)
				{
					if (mi.key() == "FacCode")
					{
						QVector<BYTE> temp_byte_vector = mi.value();
						input_byte_map.insert("FacCode1", temp_byte_vector.mid(0, 3));
						input_byte_map.insert("FacCode2", temp_byte_vector.mid(3, 6));
					}
					if (mi.key() == "SoftVer")
					{
						QVector<BYTE> temp_byte_vector = mi.value();
						input_byte_map.insert("SoftVer", temp_byte_vector.mid(0, 3));
					}
				}
			}
			if (original_name == "SoftVer")
			{
				QLineEdit* temp_text = (QLineEdit*)temp_hlayout->itemAt(2)->widget();
				QString input_data = temp_text->text();
				input_data = input_data.toUpper();
				if (input_data.length() != 3)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入长度错误！输入长度为3位！"));
					return;
				}
				if (input_data.at(0) != 'S')
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入错误！\r\n输入范围：H00~H99或HAA~HZZ！"));
					return;
				}
				int flag1 = 0;
				int flag2 = 0;
				for (int j = 1; j < 3; j++)
				{
					if (j == 1)
					{
						if (input_data.at(j) >= '0' && input_data.at(j) <= '9')
						{
							flag1 = 1;
						}
						if (input_data.at(j) >= 'A' && input_data.at(j) <= 'Z')
						{
							flag1 = 2;
						}
					}
					if (j == 2)
					{
						if (input_data.at(j) >= '0' && input_data.at(j) <= '9')
						{
							flag2 = 1;
						}
						if (input_data.at(j) >= 'A' && input_data.at(j) <= 'Z')
						{
							flag2 = 2;
						}
					}
				}
				if (flag1 == 0 || flag2 == 0 || flag1 != flag2)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入错误！\r\n输入范围：H00~H99或HAA~HZZ！"));
					return;
				}
				QVector<BYTE> temp_input_vector;
				for (int j = 0; j < input_data.length(); j++)
				{
					temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
				}
				input_byte_map.insert("SoftVer", temp_input_vector);
				QMap<QString, QVector<BYTE> >::iterator  mi;
				for (mi = content_map.begin(); mi != content_map.end(); ++mi)
				{
					if (mi.key() == "FacCode")
					{
						QVector<BYTE> temp_byte_vector = mi.value();
						input_byte_map.insert("FacCode1", temp_byte_vector.mid(0, 3));
						input_byte_map.insert("FacCode2", temp_byte_vector.mid(3, 6));
					}
					if (mi.key() == "HardVer")
					{
						QVector<BYTE> temp_byte_vector = mi.value();
						input_byte_map.insert("HardVer1", temp_byte_vector.mid(0, 1));
						input_byte_map.insert("HardVer2", temp_byte_vector.mid(1, 2));
					}
				}
			}
			if (original_name == "HardYearMonthDay")
			{
				QLineEdit* temp_text = (QLineEdit*)temp_hlayout->itemAt(2)->widget();
				QString input_data = temp_text->text();
				input_data = input_data.toUpper();
				if (input_data.length() != 8)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入长度错误！输入长度为8位！"));
					return;
				}
				for (int j = 0; j < input_data.length(); j++)
				{
					if (!(input_data.at(j) >= '0' && input_data.at(j) <= '9'))
					{
						Global::Display_prompt(0, label_text + QStringLiteral(" 输入格式错误！"));
						return;
					}
				}
				int m_year = input_data.mid(0, 4).toInt();
				int m_month = input_data.mid(4, 2).toInt();
				int m_day = input_data.mid(6, 2).toInt();
				int m_toal = input_data.mid(0, 8).toInt();
				if (!(m_year >= 2015 && m_year <= 2099))
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 中 年 输入超出范围！\r\n输入范围：2015~2099！"));
					return;
				}
				if (!(m_month >= 1 && m_month <= 12))
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 中 月 输入超出范围！\r\n输入范围：1~12！"));
					return;
				}
				if (!(m_day >= 1 && m_day <= 31))
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 中 日 输入超出范围！\r\n输入范围：1~31！"));
					return;
				}
				if (m_toal < 20150701)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入超出范围！\r\n最早日期不能早于：20150701！"));
					return;
				}
				QVector<BYTE> temp_input_vector;
				for (int j = 0; j < input_data.length(); j++)
				{
					if (j < 3)
					{
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
					else if (j == 3)
					{
						input_byte_map.insert("HardYearMonthDay1", temp_input_vector);
						temp_input_vector.clear();
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
					else
					{
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
				}
				input_byte_map.insert("HardYearMonthDay2", temp_input_vector);
				QMap<QString, QVector<BYTE> >::iterator  mi;
				for (mi = content_map.begin(); mi != content_map.end(); ++mi)
				{
					if (mi.key() == "SoftYearMonthDay")
					{
						QVector<BYTE> temp_byte_vector = mi.value();
						input_byte_map.insert("SoftYearMonthDay1", temp_byte_vector.mid(0, 2));
						input_byte_map.insert("SoftYearMonthDay2", temp_byte_vector.mid(2, 6));
					}
				}
			}
			if (original_name == "SoftYearMonthDay")
			{
				QLineEdit* temp_text = (QLineEdit*)temp_hlayout->itemAt(2)->widget();
				QString input_data = temp_text->text();
				input_data = input_data.toUpper();
				if (input_data.length() != 8)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入长度错误！输入长度为8位！"));
					return;
				}
				for (int j = 0; j < input_data.length(); j++)
				{
					if (!(input_data.at(j) >= '0' && input_data.at(j) <= '9'))
					{
						Global::Display_prompt(0, label_text + QStringLiteral(" 输入格式错误！"));
						return;
					}
				}
				int m_year = input_data.mid(0, 4).toInt();
				int m_month = input_data.mid(4, 2).toInt();
				int m_day = input_data.mid(6, 2).toInt();
				int m_toal = input_data.mid(0, 8).toInt();
				if (!(m_year >= 2015 && m_year <= 2099))
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 中 年 输入超出范围！\r\n输入范围：2015~2099！"));
					return;
				}
				if (!(m_month >= 1 && m_month <= 12))
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 中 月 输入超出范围！\r\n输入范围：1~12！"));
					return;
				}
				if (!(m_day >= 1 && m_day <= 31))
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 中 日 输入超出范围！\r\n输入范围：1~31！"));
					return;
				}
				if (m_toal < 20150701)
				{
					Global::Display_prompt(0, label_text + QStringLiteral(" 输入超出范围！\r\n最早日期不能早于：20150701！"));
					return;
				}
				QVector<BYTE> temp_input_vector;
				for (int j = 0; j < input_data.length(); j++)
				{
					if (j < 2)
					{
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
					else if (j == 2)
					{
						input_byte_map.insert("SoftYearMonthDay1", temp_input_vector);
						temp_input_vector.clear();
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
					else
					{
						temp_input_vector.push_back((BYTE)(input_data.at(j).toLatin1()));
					}
				}
				input_byte_map.insert("SoftYearMonthDay2", temp_input_vector);
				QMap<QString, QVector<BYTE> >::iterator  mi;
				for (mi = content_map.begin(); mi != content_map.end(); ++mi)
				{
					if (mi.key() == "HardYearMonthDay")
					{
						QVector<BYTE> temp_byte_vector = mi.value();
						input_byte_map.insert("HardYearMonthDay1", temp_byte_vector.mid(0, 3));
						input_byte_map.insert("HardYearMonthDay2", temp_byte_vector.mid(3, 5));
					}
				}
			}
		}
	}
	for (int i = 0; i < current_wirtedata_vector.size(); i++)
	{
		WriteECUData temp_writeecudata = current_wirtedata_vector[i];
		QString data_content = temp_writeecudata.send_data.data_content;
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
					QMap<QString, QVector<BYTE> >::iterator  mi;
					for (mi = input_byte_map.begin(); mi != input_byte_map.end(); ++mi)
					{
						if (mi.key() == temp_special_name)
						{
							tempp_data.append(mi.value());
						}
					}
				}
			}
		}
		current_wirtedata_vector[i].send_data.hex_data = tempp_data;
		current_wirtedata_vector[i].send_data.data_analyse_flag = false;
		current_wirtedata_vector[i].send_data.data_content = "";
		current_wirtedata_vector[i].Caption = ECUinfo_data.ECUinfo_name;
	}
	InputWriteECUData input_data_vector;
	input_data_vector.input_writedata_vector = current_wirtedata_vector;
	emit input_resultSignal(input_data_vector);
	sure_flag = true;
	this->close();
}

void WriteECUPage::close_windowSlot()//关闭按钮
{
	if (!sure_flag)
	{
		InputWriteECUData input_data_vector;
		emit input_resultSignal(input_data_vector);
	}
	this->close();
}

void WriteECUPage::mousePressEvent(QMouseEvent *event)//重写鼠标按下事件 
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

	return QDialog::mousePressEvent(event);
}

void WriteECUPage::mouseMoveEvent(QMouseEvent *event)//重写鼠标移动事件
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
void WriteECUPage::mouseReleaseEvent(QMouseEvent *event)
{
	mMoveing = false;
}


