#include "ReadRealTimeData.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QQueue>
#include <QMutex>
#include <QList>
#include <QTextCodec>
#include <QBitArray>
#include <boost/function/function0.hpp>   
#include <boost/thread/condition.hpp>  
#include <boost/thread/mutex.hpp>  
#include <boost/atomic.hpp>   
#include "DealSettingFile.h"

#define MAIN_SIGNAL_INDEX 10000

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

ReadRealTimeData::ReadRealTimeData(QWidget *parent)
{
	ui.setupUi(this);

	Global::loadQSS(this, "realtimedata.qss");

	receive_timeout = new QTime;
	receive_timeout->start();
	refresh_flag = 0;
	max_index = -1;
	startthread_flag = true;
	muti_frame_byte_counter = 0;
	muti_frame_byte_number = 0;

	ui.pushButton->setText(QStringLiteral("ȫ��ȡ��"));
	ui.pushButton_2->setText(QStringLiteral("��ʼˢ��"));
	ui.pushButton_3->setText(QStringLiteral("��ȡ����"));
	tree1 = new QStandardItemModel();
	ui.treeView->setModel(tree1);
	table1 = new QStandardItemModel();
	ui.tableView->setModel(table1);

	ui.tableView->verticalHeader()->setVisible(false);
	ui.tableView->horizontalHeader()->setStretchLastSection(true);//���һ���������հ�
	//ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//������
	ui.tableView->setShowGrid(false);//��������
	ui.treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);//���ɱ༭
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//���ɱ༭
	ui.treeView->setItemDelegate(new NoFocusRectangleStyle());
	ui.tableView->setItemDelegate(new NoFocusRectangleStyle());

	connect(this, SIGNAL(send_realtimedata_result_Signal(TreeData)), this, SLOT(send_realtimedata_result_Slot(TreeData)));
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(Check_Signal_Slot()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(Start_Refresh_Slot()));
	connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(ReadData_Slot()));
	connect(this, SIGNAL(stop_refresh_Signal()), this, SLOT(stop_refresh_Slot()));
	connect(tree1, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(treeItemChanged(QStandardItem*)));
	
}

ReadRealTimeData::~ReadRealTimeData()
{
	stopThread();
	delete tree1;
	delete table1;
	delete receive_timeout;
}

void ReadRealTimeData::ReadData_Slot()
{
	QVector<LoaderData> temp_vector = read_realtimedata_command_vector;
	int select_refresh_count = tree1->rowCount();
	for (int i = 0; i < tree1->rowCount(); i++)
	{
		if (tree1->item(i)->checkState() == Qt::Unchecked)
		{
			select_refresh_count--;
			QString temp_signal_name = tree1->item(i, 0)->text();
			for (int n = 0; n < temp_vector.size(); n++)
			{
				if (temp_vector[n].Caption_content == temp_signal_name)
				{
					temp_vector.remove(n);
					break;
				}
			}
		}
	}
	if (select_refresh_count <= 0)//û��ѡ���ź��򱨴�
	{
		TableWrite("û��ѡ���κ��źţ��޷�ˢ�£�������ѡ��һ���źţ�", 5);
		return;
	}
	else
	{
		read_realtimedata_command_vector = temp_vector;
		startReadThread();
	}
	ui.pushButton->setEnabled(false);
	ui.pushButton_2->setEnabled(false);
	ui.pushButton_3->setEnabled(false);
}

void ReadRealTimeData::startReadThread()//�����߳�
{
	boost::function0<void> f1 = boost::bind(&ReadRealTimeData::read_data, this);
	readdata_thread = boost::thread(f1);
}
void ReadRealTimeData::startRealTimeDataThread()//�����߳�
{
	boost::function0<void> f1 = boost::bind(&ReadRealTimeData::read_realtimedata, this);
	realtimedata_thread = boost::thread(f1);
}

void ReadRealTimeData::Init()//��������
{
	read_realtimedata_command_vector = deal_settingfile.AnalyseDiagnoseSettingFile("ReadDataStream");
	if (read_realtimedata_command_vector.size() == 0)
	{
		TableWrite("�����ļ�����ʧ�ܣ�û�л���������ݣ�", 5);
		return;
	}
	else
	{
		if (CAN_open)
		{
			TableWrite("��ʼ��ȡʵʱ���ݣ�", 1);

			tree1->clear();//��տ�������Ϣ�б�Ȼ��������д��
			tree1->setColumnCount(1);

			QStringList tableheader2;
			tableheader2 << QStringLiteral("�ź�����");

			tree1->setHorizontalHeaderLabels(tableheader2);
			QHeaderView * temp_header = ui.treeView->header();
			temp_header->setStyleSheet("\
										QHeaderView::section{\
										background-color:#afdfe4;\
										border-bottom:0px solid #f5f5f5;\
										height:30px;\
										border-top-left-radius:5px;\
										font:bold 20px;\
										font-family:System;\
										}");

			int row_index = 0;
			for (int i = 0; i < read_realtimedata_command_vector.size(); i++)
			{
				LoaderData temp_loaderdata = read_realtimedata_command_vector[i];
				QStandardItem *main_item1 = new QStandardItem(temp_loaderdata.Caption_content);
				treeitem_package temp_item;
				temp_item.content = temp_loaderdata.Caption_content;
				temp_item.m_checkstate = Qt::Checked;
				QMap<int, treeitem_package> sub_item_map;
				sub_item_map.insert(MAIN_SIGNAL_INDEX, temp_item);//Ĭ�ϸ��ź�ѡ��
				int sub_item_count = 0;
			
				for (int j = 0; j < temp_loaderdata.receive_data.data_record_vertor.size(); j++)
				{
					QList<QStandardItem *> items3;
					QStandardItem *header_item1 = new QStandardItem(temp_loaderdata.receive_data.data_record_vertor[j].caption_content);//�ź�����
					header_item1->setCheckable(true);
					header_item1->setCheckState(Qt::Checked);
					items3.push_back(header_item1);
					main_item1->appendRow(items3);
					treeitem_package temp_item2;
					temp_item2.content = temp_loaderdata.receive_data.data_record_vertor[j].caption_content;
					temp_item2.m_checkstate = Qt::Checked;
					sub_item_map.insert(sub_item_count, temp_item2);
					sub_item_count++;
				}
				main_item1->setCheckable(true);//�����ź�ǰ����Ӹ�ѡ��
				main_item1->setTristate(true);//���ź�������ѡ��״̬
				main_item1->setCheckState(Qt::Checked);
				tree1->appendRow(main_item1);
				alltreeitem_map.insert(row_index, sub_item_map);
				row_index++;
			}
			ui.treeView->expandAll();//������ȫ��չ��
			startReadThread();
		}
	}
}

void ReadRealTimeData::TableWrite(QString message_content, int message_type)//����������Ϣ����
{
	emit display_messageSignal(message_content, message_type);
}
void ReadRealTimeData::send_realtimedata_result_Slot(TreeData temp_tree_data)//��ʾ���յĿ�������Ϣ��ע������ʹ�õ���Tree view
{
	if (refresh_flag == 0)
	{
		if (temp_tree_data.tree_map.size() == 0)return;

		table1->clear();//��տ�������Ϣ�б�Ȼ��������д��
		table1->setColumnCount(2);

		QStringList tableheader3;
		tableheader3 << QStringLiteral("�ź�����") << QStringLiteral("ֵ");
		table1->setHorizontalHeaderLabels(tableheader3);

		for (int i = 0; i < table1->columnCount(); i++)
		{
			table1->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		}

		UnOrderMap<QString, UnOrderMap<QString, QString> > tree_data = temp_tree_data.tree_map;
		int row_index = 0;
		for (int i = 0; i < tree_data.size(); i++)
		{
			UnOrderMap<QString, QString> temp_map = tree_data[i].second[0];
			QMap<int, QMap<int, treeitem_package> >::iterator it;
			for (it = alltreeitem_map.begin(); it != alltreeitem_map.end(); ++it)
			{
				QMap<int, treeitem_package> tempitem_map = it.value();
				if (tempitem_map[MAIN_SIGNAL_INDEX].content == tree_data[i].first)
				{
					 QMap<int, treeitem_package>::iterator im;
					 for (im = tempitem_map.begin(); im != tempitem_map.end(); ++im)
					{
						if (im.key() != MAIN_SIGNAL_INDEX)
						{
							if (im.value().m_checkstate == Qt::Checked)
							{
								for (int j = 0; j < temp_map.size(); j++)
								{
									if (im.value().content == temp_map[j].first)
									{
										QStandardItem *header_item3 = new QStandardItem(temp_map[j].first);//�ź�����
										QStandardItem *header_item4 = new QStandardItem(temp_map[j].second[0]);//ֵ
										QList<QStandardItem *> items4;
										items4.push_back(header_item3);
										items4.push_back(header_item4);
										table1->appendRow(items4);
									}
								}
							}
						}
					}
				}
			}
		}
		if (table1->rowCount() > 0)ui.tableView->resizeColumnToContents(0);
		ui.pushButton->setEnabled(true);
		ui.pushButton_2->setEnabled(true);
		ui.pushButton_3->setEnabled(true);
	}
	else if (refresh_flag == 1)//ˢ���ź�����
	{
		UnOrderMap<QString, UnOrderMap<QString, QString> > tree_data = temp_tree_data.tree_map;
		for (int i = 0; i < tree_data.size(); i++)
		{
			UnOrderMap<QString, QString> temp_map = tree_data[i].second[0];
			for (int j = 0; j < table1->rowCount(); j++)
			{
				for (int m = 0; m < temp_map.size(); m++)
				{
					if (table1->item(j, 0)->text() == temp_map[m].first)
					{
						if (table1->item(j, 1)->text() != temp_map[m].second[0])
						{
							table1->item(j, 1)->setText(temp_map[m].second[0]);//ˢ������
						}
					}
				}
			}
		}
		if (table1->rowCount() > 0)ui.tableView->resizeColumnToContents(0);
	}
	else return;
}

void ReadRealTimeData::Check_Signal_Slot()//ȫ��ѡ��ȡ��ȫѡ
{
	TableWrite(ui.pushButton->text(), 3);
	if (ui.pushButton->text() == QStringLiteral("ȫ��ѡ��"))
	{
		for (int i = 0; i < tree1->rowCount(); i++)
		{
			if (alltreeitem_map[i][MAIN_SIGNAL_INDEX].m_checkstate == Qt::Unchecked)
			{
				tree1->item(i)->setCheckState(Qt::Checked);
				alltreeitem_map[i][MAIN_SIGNAL_INDEX].m_checkstate = Qt::Checked;
				for (int j = 0; j < alltreeitem_map[i].size() - 1; j++)
				{
					alltreeitem_map[i][j].m_checkstate = Qt::Checked;
				}
			}
		}
		ui.pushButton->setText(QStringLiteral("ȫ��ȡ��"));
	}
	else if (ui.pushButton->text() == QStringLiteral("ȫ��ȡ��"))
	{
		for (int i = 0; i < tree1->rowCount(); i++)
		{
			if (alltreeitem_map[i][MAIN_SIGNAL_INDEX].m_checkstate == Qt::Checked)
			{
				tree1->item(i)->setCheckState(Qt::Unchecked);
				alltreeitem_map[i][MAIN_SIGNAL_INDEX].m_checkstate = Qt::Unchecked;
				for (int j = 0; j < alltreeitem_map[i].size() - 1; j++)
				{
					alltreeitem_map[i][j].m_checkstate = Qt::Unchecked;
				}
			}
		}
		ui.pushButton->setText(QStringLiteral("ȫ��ѡ��"));
	}
}

void ReadRealTimeData::Start_Refresh_Slot()//��ʼˢ�¡�ֹͣˢ��
{
	if (ui.pushButton_2->text() == QStringLiteral("��ʼˢ��"))//��ʼˢ��
	{
		QVector<LoaderData> temp_vector = read_realtimedata_command_vector;
		int select_refresh_count = tree1->rowCount();
		for (int i = 0; i < tree1->rowCount(); i++)
		{
			if (tree1->item(i)->checkState() == Qt::Unchecked)
			{
				select_refresh_count--;
				QString temp_signal_name = tree1->item(i, 0)->text();
				for (int n = 0; n < temp_vector.size(); n++)
				{
					if (temp_vector[n].Caption_content == temp_signal_name)
					{
						temp_vector.remove(n);
						break;
					}
				}
			}
		}
		if (select_refresh_count <= 0)//û��ѡ���ź��򱨴�
		{
			TableWrite("û��ѡ���κ��źţ��޷�ˢ�£�������ѡ��һ���źţ�", 5);
			return;
		}
		else
		{
			TableWrite("ֹͣ�����̣߳���ʼˢ��ʵʱ���ݣ�", 1);
			refresh_flag = 1;
			startthread_flag = true;
			startRealTimeDataThread();
		}
		ui.pushButton_2->setText(QStringLiteral("ֹͣˢ��"));
		ui.pushButton->setEnabled(false);
		ui.pushButton_3->setEnabled(false);
	}
	else if (ui.pushButton_2->text() == QStringLiteral("ֹͣˢ��"))//ֹͣˢ��
	{
		TableWrite("���������̣߳�ֹͣˢ��ʵʱ���ݣ�", 1);
		emit stop_refresh_Signal();
	}
}

void ReadRealTimeData::stop_refresh_Slot()//ֹͣˢ�£������������̣߳���Ҫ����Ϊʵʱ���ݽ���ʧ�ܣ�˵���豸�Ѿ���������������Ҳ��ʧ��
{
	startthread_flag = false;
	ui.pushButton_2->setText(QStringLiteral("��ʼˢ��"));
	ui.pushButton->setEnabled(true);
	ui.pushButton_3->setEnabled(true);
}


void ReadRealTimeData::treeItemChanged(QStandardItem * item)//�޸�ѡ��״̬
{
	if (item == NULL)return;
	if (refresh_flag == 1)//��ˢ��ʱ�������޸�ѡ��״̬�����޸ĺ�Ҫ���л�ԭ
	{
		if (item->rowCount() > 0)//����item��˵���������źţ��Ǹ��ź�
		{
			for (int i = 0; i < tree1->rowCount(); i++)
			{
				if (tree1->item(i)->text() == item->text())
				{
					QMap<int, treeitem_package> temp_map = alltreeitem_map[i];
					item->setCheckState(temp_map[MAIN_SIGNAL_INDEX].m_checkstate);//���źŵ�ѡ��״̬Ҳ����map�У�KEY��MAIN_SIGNAL_INDEX
					break;
				}
			}
		}
		else  if (item->rowCount() == 0)//���ź�
		{
			QStandardItem * temp_parent = item->parent();//��ø��ź�
			for (int i = 0; i < tree1->rowCount(); i++)
			{
				if (tree1->item(i)->text() == temp_parent->text())//���Ҹ��źŵ�λ��
				{
					QMap<int, treeitem_package> temp_map = alltreeitem_map[i];
					for (int j = 0; j < temp_parent->rowCount(); j++)
					{
						if (temp_parent->child(j)->text() == item->text())//���źŵ�����
						{
							item->setCheckState(temp_map[j].m_checkstate);
							break;
						}
					}
					break;
				}
			}
		}
	}
	else//����ˢ��״̬�����������޸�ѡ��״̬
	{
		if (item->rowCount() > 0)
		{
			for (int i = 0; i < tree1->rowCount(); i++)
			{
				if (tree1->item(i)->text() == item->text())
				{
					int main_item_index = item->rowCount();
					alltreeitem_map[i][main_item_index].m_checkstate = item->checkState();

					if (item->checkState() == Qt::Checked)//���ź�ѡ�У����ź�ȫ��ѡ��
					{
						for (int j = 0; j < item->rowCount(); j++)
						{
							item->child(j)->setCheckState(Qt::Checked);
							alltreeitem_map[i][j].m_checkstate = Qt::Checked;
						}
					}
					else if (item->checkState() == Qt::Unchecked)//���ź�ȡ��ѡ�У����ź�ȫ��ȡ��ѡ��
					{
						for (int j = 0; j < item->rowCount(); j++)
						{
							item->child(j)->setCheckState(Qt::Unchecked);
							alltreeitem_map[i][j].m_checkstate = Qt::Unchecked;
						}
					}
					break;
				}
			}
		}
		else  if (item->rowCount() == 0)//���ź�
		{
			if (item->checkState() == Qt::Checked)
			{
				QStandardItem * temp_parent = item->parent();
				for (int i = 0; i < tree1->rowCount(); i++)
				{
					if (tree1->item(i)->text() == temp_parent->text())
					{
						for (int j = 0; j < temp_parent->rowCount(); j++)
						{
							if (temp_parent->child(j)->text() == item->text())
							{
								alltreeitem_map[i][j].m_checkstate = Qt::Checked;
								break;
							}
						}
						break;
					}
				}
				int child_check_count = 0;
				for (int i = 0; i < temp_parent->rowCount(); i++)//ע�⣬�����ź�ȫ��ѡ�У����ź�ҲΪѡ��״̬
				{
					if (temp_parent->child(i)->checkState() == Qt::Checked)
					{
						child_check_count++;
					}
				}
				if (child_check_count == temp_parent->rowCount() && child_check_count > 0)
				{
					temp_parent->setCheckState(Qt::Checked);

					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::Checked;
							break;
						}
					}
				}
				else if(child_check_count < temp_parent->rowCount() && child_check_count > 0)//���źŲ���ѡ�У����ź�Ϊ����̬
				{
					temp_parent->setCheckState(Qt::PartiallyChecked);
					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::PartiallyChecked;
							break;
						}
					}
				}
				else if (child_check_count == 0)
				{
					temp_parent->setCheckState(Qt::Unchecked);
					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::Unchecked;
							break;
						}
					}
				}
			}
			else if (item->checkState() == Qt::Unchecked)
			{
				QStandardItem * temp_parent = item->parent();
				for (int i = 0; i < tree1->rowCount(); i++)
				{
					if (tree1->item(i)->text() == temp_parent->text())
					{
						for (int j = 0; j < temp_parent->rowCount(); j++)
						{
							if (temp_parent->child(j)->text() == item->text())
							{
								alltreeitem_map[i][j].m_checkstate = Qt::Unchecked;
								break;
							}
						}
						break;
					}
				}
				int child_uncheck_count = 0;
				for (int i = 0; i < temp_parent->rowCount(); i++)
				{
					if (temp_parent->child(i)->checkState() == Qt::Unchecked)
					{
						child_uncheck_count++;
					}
				}
				if (child_uncheck_count == temp_parent->rowCount() && child_uncheck_count > 0)//ע�⣬�����ź�ȫ��ȡ��ѡ�У����ź�ҲΪȡ��ѡ��״̬
				{
					temp_parent->setCheckState(Qt::Unchecked);
					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::Unchecked;
							break;
						}
					}
				}
				else if (child_uncheck_count < temp_parent->rowCount() && child_uncheck_count > 0)//�����ź�Ϊ����̬
				{
					temp_parent->setCheckState(Qt::PartiallyChecked);
					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::PartiallyChecked;
							break;
						}
					}
				}
				else if (child_uncheck_count == 0)
				{
					temp_parent->setCheckState(Qt::Checked);
					for (int i = 0; i < tree1->rowCount(); i++)
					{
						if (tree1->item(i)->text() == temp_parent->text())
						{
							alltreeitem_map[i][temp_parent->rowCount()].m_checkstate = Qt::Checked;
							break;
						}
					}
				}
			}
		}
		refresh_table();
	}
}
void ReadRealTimeData::refresh_table()//����������б�ѡ��״̬�б仯֮���ұ������б�Ҫ����ˢ��
{
	table1->clear();//��տ�������Ϣ�б�Ȼ��������д��
	table1->setColumnCount(2);
	QStringList tableheader3;
	tableheader3 << QStringLiteral("�ź�����") << QStringLiteral("ֵ");
	table1->setHorizontalHeaderLabels(tableheader3);
	for (int i = 0; i < tree1->rowCount(); i++)
	{
		QMap<int, treeitem_package> temp_map = alltreeitem_map[i];
		for (int j = 0; j < tree1->item(i)->rowCount(); j++)
		{
			if (temp_map[j].m_checkstate == Qt::Checked)
			{
				QStandardItem *header_item3 = new QStandardItem(tree1->item(i)->child(j)->text());//�ź�����
				QStandardItem *header_item4 = new QStandardItem("");//ֵ
				QList<QStandardItem *> items4;
				items4.push_back(header_item3);
				items4.push_back(header_item4);
				table1->appendRow(items4);
			}
		}
	}
	if (table1->rowCount() > 0)
	{
		ui.tableView->resizeColumnToContents(0);
		ui.tableView->horizontalHeader()->setStretchLastSection(true);//���һ���������հ�
	}
	
}


void ReadRealTimeData::read_data()
{
	int re_send_item = 0;
	bool re_receive_flag = false;
	UnOrderMap<QString, UnOrderMap<QString, QString> > read_ECU_info_vector;
	LoaderData current_command;
	int total_signal_number = 0;
	int no_receive_count = 0;
	int i = -1;
	QVector<LoaderData> command_data_vector = read_realtimedata_command_vector;
	if (total_signal_number == 0)total_signal_number = command_data_vector.size();//���ź�����
	QVector<BYTE> receive_byte_vector;
	current_thread_order++;
	data_thread_mutex.lock();
	while (CAN_open && ++i < command_data_vector.size())//�����б�һ����ִ��
	{
		boost::this_thread::interruption_point();//��������жϵ㣬�жϲŻ�������
		UnOrderMap<QString, QString> one_DID_data_map;
		
		if (!re_receive_flag)current_command = command_data_vector[i];//û�����½���ʱ������ָ���Ϊ���½��պ�ָ��ᷢ���仯
		SendData temp_senddata = command_data_vector[i].send_data;
		ReceiveDiagloseData temp_receive_settingdata = command_data_vector[i].receive_data;
		send_realtime_data(temp_senddata);//����ָ��
		Get_Record();//��ȡ���ͽ�������֡
		if (temp_receive_settingdata.frame_id != 0)//��������֡��Ϊ����ʾ���ղ�Ϊ�գ���Ҫ�����Ӧ����֡������
		{
			QVector<BYTE> temp_receive_byte_vector = receive_realtime_data(current_command.send_data, temp_receive_settingdata);//��ý���֡�������ֽڣ�ע���֡���շ�2�ν���
			if (no_receive_count >= 50)//������50�ζ�û���յ�����˵���������⣬Ӧ��ֹͣˢ��
			{
				TableWrite("ʵʱ���ݽ����쳣���ж�ˢ�£��������豸��", 5);
				no_receive_count = 0;
				break;
			}
			if (temp_receive_byte_vector.size() == 0)//û�н��յ�����
			{
				if (receive_timeout->elapsed() < 10)//ʱ��С��10ms����������
				{
					LoaderData tempdata1 = command_data_vector[i];
					tempdata1.send_data.frame_id = 0;//����֡IDΪ0��������ִ�к��÷��ͣ�ֱ�ӽ���
					command_data_vector.replace(i, tempdata1);//�滻��ǰ����
					i = i - 1;//�´�ѭ������ִ�е�ǰ���������ֻ����
					re_receive_flag = true;
					continue;
				}
				else if (re_send_item < 2)//��ʱ����100ms��������С��3��ʱ�����ظ�����ִ�д�������
				{
					if (current_command.send_data.hex_data[0] == 0x30)//�жϵ�ǰ֡�Ƿ�Ϊ0x30֡�������˻ص�ǰһ������֡���·��ͣ���Ϊ0x30֡�ط���Ч
					{
						command_data_vector.remove(i);
						i = i - 2;
					}
					else//��������֡����ʧ�ܣ������·���
					{
						command_data_vector.replace(i, current_command);
						i = i - 1;
					}
					no_receive_count++;
					re_send_item++;//�ظ�����+1
					re_receive_flag = false;
					continue;
				}
				else if (re_send_item == 2)//��������ʧ��ʱ��������һ������
				{
					re_send_item = 0;
					TableWrite("�����η��ͽ���ʧ�ܣ�����ָ��ʧ�ܣ�", 2);
					if (current_command.send_data.hex_data[0] == 0x30)//�жϵ�ǰ֡�Ƿ�Ϊ0x30֡�������˻ص�ǰһ������֡���·��ͣ���Ϊ0x30֡�ط���Ч
					{
						command_data_vector.remove(i);
						i = i - 2;
					}
					else//��������֡����ʧ�ܣ������·���
					{
						command_data_vector.replace(i, current_command);
						i = i - 1;
					}
					re_receive_flag = false;
					no_receive_count++;
					continue;
				}
			}
			else
			{
				re_receive_flag = false;
				no_receive_count = 0;
				if (temp_receive_byte_vector[0] != 0x10 && current_command.send_data.hex_data[0] != 0x30 && temp_receive_byte_vector.size() == 8)//��ͨ��֡����
				{
					temp_receive_byte_vector.remove(0, 4);//ȡ��4���ֽ�Ϊ��Ч�ֽ�
					receive_byte_vector.append(temp_receive_byte_vector);
					one_DID_data_map = GetAnalyseReceiveDataResult(temp_receive_settingdata, receive_byte_vector);
					receive_byte_vector.clear();
				}
				else if (current_command.send_data.hex_data[0] == 0x30 && temp_receive_byte_vector[0] >= 0x20 && temp_receive_byte_vector[0] <= 0x2f)//��֡�У�0x30����֮֡��Ľ�������֡
				{
					QVector<BYTE> temp_vector;
					for (int m = 0; m < temp_receive_byte_vector.size(); m++)
					{
						if (m % 8 != 0)temp_vector.push_back(temp_receive_byte_vector[m]);//ȥ��ÿ֡�����ֽ�
					}
					receive_byte_vector.append(temp_vector);
					one_DID_data_map = GetAnalyseReceiveDataResult(temp_receive_settingdata, receive_byte_vector);
					receive_byte_vector.clear();
				}
				else if (temp_receive_byte_vector[0] == 0x10 && temp_receive_byte_vector.size() == 8)//0x10����֡������֮����뷢��0x30֡
				{
					temp_receive_byte_vector.remove(0, 5);//ȡ��3���ֽ�Ϊ��Ч�ֽ�
					receive_byte_vector.append(temp_receive_byte_vector);
					LoaderData tempdata1 = current_command;
					re_receive_flag = false;
					tempdata1.send_data.byte_length = 8;
					tempdata1.send_data.data_analyse_flag = false;
					tempdata1.send_data.data_content = "";
					tempdata1.send_data.single_muti_frame_flag = 0;
					tempdata1.send_data.hex_data.clear();
					tempdata1.send_data.hex_data.push_back(0x30);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					tempdata1.send_data.hex_data.push_back(0x00);
					command_data_vector.insert(i + 1, tempdata1);
					continue;
				}
			}
		}
		if (one_DID_data_map.size() == 0)//����ʧ��
		{
			if (current_command.send_data.hex_data[0] == 0x30)//�жϵ�ǰ֡�Ƿ�Ϊ0x30֡�������˻ص�ǰһ������֡���·��ͣ���Ϊ0x30֡�ط���Ч
			{
				command_data_vector.remove(i);
				i = i - 2;
			}
			else//��������֡����ʧ�ܣ������·���
			{
				command_data_vector.replace(i, current_command);
				i = i - 1;
			}
		}
		else
		{
			read_ECU_info_vector.insert(command_data_vector[i].Caption_content, one_DID_data_map);//����������
		}
	}
	current_thread_order--;
	heart_thread_condition.notify_one();
	data_thread_mutex.unlock();
	TreeData temp_treedata;
	refresh_flag = 0;
	if (read_ECU_info_vector.size() == total_signal_number)//�������ź������ﵽ���ź������󣬽�����������ݸ����̲߳���ʾ
	{
		temp_treedata.tree_map = read_ECU_info_vector;
		TableWrite("��ȡʵʱ���ݳɹ���", 4);
		emit send_realtimedata_result_Signal(temp_treedata);
		read_ECU_info_vector.clear();
	}
	else
	{
		TableWrite("��ȡʵʱ����ʧ�ܣ�", 5);
		emit send_realtimedata_result_Signal(temp_treedata);
		read_ECU_info_vector.clear();
	}
}


void ReadRealTimeData::read_realtimedata()
{
	int re_send_item = 0;
	bool re_receive_flag = false;
	UnOrderMap<QString, UnOrderMap<QString, QString> > read_ECU_info_vector;
	LoaderData current_command;
	int total_signal_number = 0;
	int no_receive_count = 0;
	current_thread_order++;
	data_thread_mutex.lock();
	realtimedata_is_running = true;
	while (CAN_open && startthread_flag)
	{
		boost::this_thread::interruption_point();//��������жϵ㣬�жϲŻ�������
		int i = -1;
		QVector<LoaderData> command_data_vector = read_realtimedata_command_vector;
		if (total_signal_number == 0)total_signal_number = command_data_vector.size();//���ź�����
		QVector<BYTE> receive_byte_vector;
		while (CAN_open && ++i < command_data_vector.size() && startthread_flag)//�����б�һ����ִ��
		{
			UnOrderMap<QString, QString> one_DID_data_map;
			
			if (!re_receive_flag)current_command = command_data_vector[i];//û�����½���ʱ������ָ���Ϊ���½��պ�ָ��ᷢ���仯
			SendData temp_senddata = command_data_vector[i].send_data;
			ReceiveDiagloseData temp_receive_settingdata = command_data_vector[i].receive_data;

			send_realtime_data(temp_senddata);//����ָ��
			Get_Record();//��ȡ���ͽ�������֡
			if (temp_receive_settingdata.frame_id != 0)//��������֡��Ϊ����ʾ���ղ�Ϊ�գ���Ҫ�����Ӧ����֡������
			{
				QVector<BYTE> temp_receive_byte_vector = receive_realtime_data(current_command.send_data, temp_receive_settingdata);//��ý���֡�������ֽڣ�ע���֡���շ�2�ν���
				if (temp_receive_byte_vector.size() == 0)//û�н��յ�����
				{
					if (no_receive_count >= 50)//������50�ζ�û���յ�����˵���������⣬Ӧ��ֹͣˢ��
					{
						TableWrite("���ݽ����쳣���������豸��", 5);
						no_receive_count = 0;
						emit stop_refresh_Signal();
						break;
					}
					if (receive_timeout->elapsed() < 10)//ʱ��С��10ms����������
					{
						LoaderData tempdata1 = command_data_vector[i];
						tempdata1.send_data.frame_id = 0;//����֡IDΪ0��������ִ�к��÷��ͣ�ֱ�ӽ���
						command_data_vector.replace(i, tempdata1);//�滻��ǰ����
						i = i - 1;//�´�ѭ������ִ�е�ǰ���������ֻ����
						re_receive_flag = true;
						continue;
					}
					else if (re_send_item < 2)//��ʱ����100ms��������С��3��ʱ�����ظ�����ִ�д�������
					{
						if (current_command.send_data.hex_data[0] == 0x30)//�жϵ�ǰ֡�Ƿ�Ϊ0x30֡�������˻ص�ǰһ������֡���·��ͣ���Ϊ0x30֡�ط���Ч
						{
							command_data_vector.remove(i);
							i = i - 2;

						}
						else//��������֡����ʧ�ܣ������·���
						{
							command_data_vector.replace(i, current_command);
							i = i - 1;
						}
						no_receive_count++;
						re_send_item++;//�ظ�����+1
						re_receive_flag = false;
						continue;
					}
					else if (re_send_item == 2)//��������ʧ��ʱ��������һ������
					{
						re_send_item = 0;
						TableWrite("�����η��ͽ���ʧ�ܣ�����ָ��ʧ�ܣ�", 2);
						re_receive_flag = false;
						no_receive_count++;
						continue;
					}
				}
				else
				{
					re_receive_flag = false;
					no_receive_count = 0;
					if (temp_receive_byte_vector[0] != 0x10 && current_command.send_data.hex_data[0] != 0x30 && temp_receive_byte_vector.size() == 8)//��ͨ��֡����
					{
						temp_receive_byte_vector.remove(0, 4);//ȡ��4���ֽ�Ϊ��Ч�ֽ�
						receive_byte_vector.append(temp_receive_byte_vector);
						one_DID_data_map = GetAnalyseReceiveDataResult(temp_receive_settingdata, receive_byte_vector);
						receive_byte_vector.clear();
					}
					else if (current_command.send_data.hex_data[0] == 0x30 && temp_receive_byte_vector[0] >= 0x20 && temp_receive_byte_vector[0] <= 0x2f)//��֡�У�0x30����֮֡��Ľ�������֡
					{
						QVector<BYTE> temp_vector;
						for (int m = 0; m < temp_receive_byte_vector.size(); m++)
						{
							if (m % 8 != 0)temp_vector.push_back(temp_receive_byte_vector[m]);//ȥ��ÿ֡�����ֽ�
						}
						
						receive_byte_vector.append(temp_vector);
						one_DID_data_map = GetAnalyseReceiveDataResult(temp_receive_settingdata, receive_byte_vector);
						receive_byte_vector.clear();
					}
					else if (temp_receive_byte_vector[0] == 0x10 && temp_receive_byte_vector.size() == 8)//0x10����֡������֮����뷢��0x30֡
					{
						temp_receive_byte_vector.remove(0, 5);//ȡ��3���ֽ�Ϊ��Ч�ֽ�
						receive_byte_vector.append(temp_receive_byte_vector);
						LoaderData tempdata1 = current_command;
						re_receive_flag = false;
						tempdata1.send_data.byte_length = 8;
						tempdata1.send_data.data_analyse_flag = false;
						tempdata1.send_data.data_content = "";
						tempdata1.send_data.single_muti_frame_flag = 0;
						tempdata1.send_data.hex_data.clear();
						tempdata1.send_data.hex_data.push_back(0x30);
						tempdata1.send_data.hex_data.push_back(0x00);
						tempdata1.send_data.hex_data.push_back(0x00);
						tempdata1.send_data.hex_data.push_back(0x00);
						tempdata1.send_data.hex_data.push_back(0x00);
						tempdata1.send_data.hex_data.push_back(0x00);
						tempdata1.send_data.hex_data.push_back(0x00);
						tempdata1.send_data.hex_data.push_back(0x00);
						command_data_vector.insert(i + 1, tempdata1);
						continue;
					}
				}
			}
			if (one_DID_data_map.size() == 0)//����ʧ��
			{
				if (current_command.send_data.hex_data[0] == 0x30)//�жϵ�ǰ֡�Ƿ�Ϊ0x30֡�������˻ص�ǰһ������֡���·��ͣ���Ϊ0x30֡�ط���Ч
				{
					command_data_vector.remove(i);
					i = i - 2;
				}
				else//��������֡����ʧ�ܣ������·���
				{
					command_data_vector.replace(i, current_command);
					i = i - 1;
				}
			}
			else read_ECU_info_vector.insert(command_data_vector[i].Caption_content, one_DID_data_map);//����������
		}
		if (read_ECU_info_vector.size() == total_signal_number)//�������ź������ﵽ���ź������󣬽�����������ݸ����̲߳���ʾ
		{
			TreeData temp_treedata;
			refresh_flag = 1;
			temp_treedata.tree_map = read_ECU_info_vector;
			emit send_realtimedata_result_Signal(temp_treedata);
			read_ECU_info_vector.clear();
			QThread::msleep(30);
		}
		else
		{
			read_ECU_info_vector.clear();
			continue;
		}
	}
	current_thread_order--;
	heart_thread_condition.notify_one();
	data_thread_mutex.unlock();
	refresh_flag = 0;
	realtimedata_is_running = false;
}

void ReadRealTimeData::send_realtime_data(SendData send_data)//���͹���
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
		receive_timeout->restart();//��ʱ��ʼ
		QThread::msleep(5);
	}
}

QVector<BYTE> ReadRealTimeData::receive_realtime_data(SendData send_data, ReceiveDiagloseData temp_receive_data)//���չ���
{
	QVector<BYTE> temp_byte_vector;
	int find_send_frame_index = -1;
	for (int m = 0; m < ReadRealTime_Record_Vector.size(); m++)//�ڻ�ȡ�ķ��ͽ�������֡�У�����ɸѡ
	{
		Record temp_can_record = ReadRealTime_Record_Vector[m];
		int same_byte_count = 0;
		if (temp_can_record.this_frame.ID == send_data.frame_id && temp_can_record.this_frame.DataLen == send_data.byte_length)//���ҷ���֡�Ƿ����
		{
			for (int n = 0; n < temp_can_record.this_frame.DataLen; n++)//���͵�ÿ���ֽ�Ҫ��ͬ
			{
				if (temp_can_record.this_frame.Data[n] == send_data.hex_data[n])same_byte_count++;
			}
			if (same_byte_count == temp_can_record.this_frame.DataLen)//�ҵ�����֡
			{
				find_send_frame_index = m;
				break;
			}
		}
	}
	if (find_send_frame_index >= 0)//�ҵ�����֡
	{
		int muti_frame_flag = 0;
		for (int m = find_send_frame_index; m < ReadRealTime_Record_Vector.size(); m++)//�ӷ���֡λ�ú�ʼ���ҽ���֡
		{
			if (ReadRealTime_Record_Vector[m].this_frame.ID == current_page_block.receive_frame_id)//�ڽ�������֡�в���ָ��ID
			{
				if (ReadRealTime_Record_Vector[m].this_frame.Data[0] == 0x10)//��֡����֡
				{
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(ReadRealTime_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_number = ReadRealTime_Record_Vector[m].this_frame.Data[1];//��֡���ֽ���
					muti_frame_byte_counter = 6;//��ǰ֡�а���6����֡��Ч�ֽ�
					ReadRealTime_Record_Vector.remove(0, m + 1);//��շ��ͽ���֡����
					break;
				}
				else if (send_data.hex_data[0] == 0x30 &&
					(ReadRealTime_Record_Vector[m].this_frame.Data[0] >= 0x20
					&& ReadRealTime_Record_Vector[m].this_frame.Data[0] <= 0x2f))//��֡�ģ�0x30����֡��Ӧ�Ľ���֡
				{
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(ReadRealTime_Record_Vector[m].this_frame.Data[j]);
					}
					muti_frame_byte_counter += 7;//ÿ֡����7����Ч�ֽڣ����ֽڲ���
					muti_frame_flag++;//����0x30����֡��Ӧ�Ľ���֡
					if (muti_frame_byte_counter >= muti_frame_byte_number)//�ж϶�֡�Ƿ������ɣ�������Ч�ֽ��ж�
					{
						ReadRealTime_Record_Vector.remove(0, m + 1);//��շ��ͽ���֡����
						break;
					}
				}
				else//��ͨ����֡
				{
					muti_frame_byte_number = 0;
					muti_frame_byte_counter = 0;
					for (int j = 0; j < 8; j++)
					{
						temp_byte_vector.push_back(ReadRealTime_Record_Vector[m].this_frame.Data[j]);
					}
					ReadRealTime_Record_Vector.remove(0, m + 1);//��շ��ͽ���֡����
					break;
				}
			}
		}
		if (muti_frame_flag > 0)//����0x30����֡��Ӧ�Ľ���֡
		{
			if (muti_frame_byte_counter < muti_frame_byte_number)//����ﵽ��Ч�ֽڵ��������ܷ��أ��������
			{
				muti_frame_byte_counter = 6;
				temp_byte_vector.clear();
			}
		}
		if (temp_byte_vector.size() > 0)//���ؽ����ֽ�
		{
			TableWrite("�������ݽ�����ϣ�����ָ����ɣ�", 3);
		}
	}
	return temp_byte_vector;
}

void ReadRealTimeData::Get_Record()//��ȡ���ͽ�������֡���������֡ID����
{
	record_mutex.lock();
	for (int i = 0; i < CAN_Record.size(); i++)
	{
		Record temp_record = CAN_Record[i];
		if (current_page_block.send_frame_id != 0 && temp_record.this_frame.ID == current_page_block.send_frame_id && temp_record.frame_index > max_index)//�������֡ID�������������ظ���ȡ
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			ReadRealTime_Record_Vector.push_back(temp_record);
		}
		if (current_page_block.receive_frame_id != 0 && temp_record.this_frame.ID == current_page_block.receive_frame_id && temp_record.frame_index > max_index)//�������֡ID�������������ظ���ȡ
		{
			if (temp_record.frame_index > max_index)max_index = temp_record.frame_index;
			ReadRealTime_Record_Vector.push_back(temp_record);
		}
	}
	record_mutex.unlock();
}

UnOrderMap<QString, QString> ReadRealTimeData::GetAnalyseReceiveDataResult(ReceiveDiagloseData temp_data_setting, QVector<BYTE> receive_byte_vector)//���ݽ��ղ�������֡���н���
{
	UnOrderMap<QString, QString> temp_data_map;
	if (receive_byte_vector.size() == 0)return temp_data_map;
	if (temp_data_setting.frame_id == 0)return temp_data_map;

	QBitArray data_group(receive_byte_vector.size() * 8);//����λ����
	for (int m = 0; m < receive_byte_vector.size(); m++)
	{
		BYTE temp_char = receive_byte_vector[m];
		for (int n = 0; n < 8; n++)
		{
			bool temp = temp_char &(1 << (7 - n));
			data_group.setBit(m * 8 + n, temp_char &(1 << (7 - n)));//��λbit����ǰ�棬�±�С
		}
	}
	QString special_data_value;
	for (int i = 0; i < temp_data_setting.data_record_vertor.size(); i++)
	{
		ReceiveAnalyseData temp_analyse = temp_data_setting.data_record_vertor[i];
		if (data_group.size() < temp_analyse.startbit + temp_analyse.bitlength)//��֤Ϊ���ȴﵽҪ�󣬷�������Ǵ���֡���߽��մ��󣬱������鳬����Χ
		{
			return temp_data_map;
		}
		qlonglong raw_value = 0;
		QBitArray sg_data_group(temp_analyse.bitlength);
		for (int m = 0; m < temp_analyse.bitlength; m++)//��ȡ�źŶ�Ӧ��bit����
		{
			sg_data_group.setBit(m, data_group.at(temp_analyse.startbit + m));
		}
		for (int m = 0; m < sg_data_group.size(); m++)//��ȡʵ��ֵ
		{
			raw_value += (sg_data_group.at(m) << (sg_data_group.size() - m - 1));
		}

		if (temp_analyse.invalid_value != -1)
		{
			if (raw_value == temp_analyse.invalid_value)
			{
				temp_data_map.insert(temp_analyse.caption_content, "Invalid");
				continue;
			}
		}
		switch (temp_analyse.Caculate_Type)//����Caculate_Type�ֱ���н���
		{
		case 0://state��λ��ʾ״̬
		{
			QMap<int, QString>::iterator  mi;
			for (mi = temp_analyse.value_content_map.begin(); mi != temp_analyse.value_content_map.end(); ++mi)
			{
				if (raw_value == mi.key())
				{
					QString temp_content = mi.value();
					temp_data_map.insert(temp_analyse.caption_content, temp_content);
					break;
				}
			}
			break;
		}
		case 1://Numerical����ʾ��ֵ
		{
			QString temp_value = AnalyseFormula(temp_analyse.Formula, (double)raw_value);
			if (temp_analyse.Format == "Date")
			{
				if (temp_value.length() == 1)temp_value = "0" + temp_value;
				special_data_value += temp_value;
			}
			else
			{
				if (temp_analyse.Unit == "")
				{
					temp_data_map.insert(temp_analyse.caption_content, temp_value);
				}
				else
				{
					temp_data_map.insert(temp_analyse.caption_content, temp_value + "  " + temp_analyse.Unit);
				}
			}
			break;
		}
		case 2://ASC2��ת����ASC2��
		{
			QString temp_value;
			for (int m = 0; m < receive_byte_vector.size() && m < temp_analyse.bitlength / 8; m++)
			{
				BYTE temp_char = receive_byte_vector[m];
				temp_value.append((uchar)temp_char);
			}
			temp_data_map.insert(temp_analyse.caption_content, temp_value);
			break;
		}
		case 3://16����
		{
			QString temp_value;
			for (int m = 0; m < receive_byte_vector.size() && m < temp_analyse.bitlength / 8; m++)
			{
				BYTE temp_char = receive_byte_vector[m];
				temp_value += QString("%1").arg((unsigned short)temp_char, 2, 16, QLatin1Char('0'));
			}
			temp_data_map.insert(temp_analyse.caption_content, temp_value);
			break;
		}
		default:
			break;
		}
	}
	if (special_data_value != "")//����������
	{
		temp_data_map.insert(temp_data_setting.data_record_vertor[0].caption_content, special_data_value);
	}
	return temp_data_map;
}

QString ReadRealTimeData::AnalyseFormula(QString temp_formula, double temp_value)//��ʽ������Ҫ��+ - * /����
{
	if (temp_formula == "")return "";
	if (temp_formula.mid(0, 1) != "A")return "";
	QRegExp rx1("([\\+\\-\\*/]+)");
	int pos = 1;
	QString temp_unit1, temp_unit2;
	int pos1 = 0;
	int pos2 = 0;
	QString value1, value2;
	while ((pos = rx1.indexIn(temp_formula, pos)) != -1)
	{
		if (pos1 == 0)pos1 = pos;
		else pos2 = pos;
		if (temp_unit1 == "")temp_unit1 = rx1.cap(1);
		else temp_unit2 = rx1.cap(1);
		if (pos1 != 0 && pos2 != 0)
		{
			value1 = temp_formula.mid(pos1 + 1, pos2 - pos1 - 1);
		}
		pos += rx1.matchedLength();
	}
	int temp_pos = pos1 > pos2 ? pos1 : pos2;
	if (temp_pos + 1 < temp_formula.length())
	{
		if (value1 == "")value1 = temp_formula.mid(temp_pos + 1, temp_formula.length() - temp_pos - 1);
		else value2 = temp_formula.mid(temp_pos + 1, temp_formula.length() - temp_pos - 1);

		if (value1 != "")
		{
			if (temp_unit1 == "+")
			{
				temp_value = temp_value + value1.toDouble();
			}
			if (temp_unit1 == "-")
			{
				temp_value = temp_value - value1.toDouble();
			}
			if (temp_unit1 == "*")
			{
				temp_value = temp_value * value1.toDouble();
			}
			if (temp_unit1 == "/")
			{
				temp_value = temp_value / value1.toDouble();
			}
		}
		if (value2 != "")
		{
			if (temp_unit2 == "+")
			{
				temp_value = temp_value + value2.toDouble();
			}
			if (temp_unit2 == "-")
			{
				temp_value = temp_value - value2.toDouble();
			}
			if (temp_unit2 == "*")
			{
				temp_value = temp_value * value2.toDouble();
			}
			if (temp_unit2 == "/")
			{
				temp_value = temp_value / value2.toDouble();
			}
		}
	}
	if (fabs(temp_value - (int)temp_value) < 0.00000001)//�����ж��Ƿ�������
	{
		return QString::number((qlonglong)temp_value);
	}
	else return QString::number(temp_value, 'f', 2);
}

void ReadRealTimeData::stopThread()//ֹͣʵʱ�����߳�
{
	startthread_flag = false;
	realtimedata_thread.interrupt();
	realtimedata_thread.join();
}
