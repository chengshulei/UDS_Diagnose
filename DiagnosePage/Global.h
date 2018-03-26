#pragma once


#include <QObject>
#include <QVector>
#include <QMessageBox>
#include <QWidget>
#include <QMap>
#include <cmath> 
#include "Log.h"
#include "Prompt.h"
#include <QFile>
#include <QStyledItemDelegate>

typedef unsigned long       DWORD;  //这些类型的重新命名的类型是在windows库中实现的，
typedef void *              HANDLE; //这里不加载windows库，所以要进行实现
typedef unsigned short      USHORT;
typedef unsigned char       BYTE;
typedef char                CHAR;
typedef short               SHORT;
typedef long                LONG;
typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned char       UCHAR;
typedef unsigned long       ULONG;
typedef void *              PVOID;

//2.定义CAN信息帧的数据类型。
typedef  struct  _VCI_CAN_OBJ{
	UINT	ID;
	UINT	TimeStamp;
	BYTE	TimeFlag;
	BYTE	SendType;
	BYTE	RemoteFlag;//是否是远程帧
	BYTE	ExternFlag;//是否是扩展帧
	BYTE	DataLen;
	BYTE	Data[8];
	BYTE	Reserved[3];
}VCI_CAN_OBJ, *PVCI_CAN_OBJ;

class FaultInfo
{
public:
	FaultInfo()
	{
		dtc_code = "";
		fault_name = "";
		time_state = "";
		fault_reasion = "";
		repair_info = "";
	}
	QString dtc_code;
	QString fault_name;
	QString time_state;
	QString fault_reasion;
	QString repair_info;

	QVector<BYTE> code_vector;

	void Clear()
	{
		dtc_code = "";
		fault_name = "";
		time_state = "";
		fault_reasion = "";
		repair_info = "";
	}
};

class FaultSort
{
public:
	FaultSort()
	{
		dtc_code = "";
		fault_name = "";
		time_state = "";
		fault_times = -1;
		fault_order = -1;
	}
	QString dtc_code;
	QString fault_name;
	QString time_state;
	int fault_times;
	int fault_order;

	void Clear()
	{
		dtc_code = "";
		fault_name = "";
		time_state = "";
		fault_times = -1;
		fault_order = -1;
	}
};


class Exist_Device
{
public:
	int device_type_value;
	int device_index;
	int channel_number;
	QString baundrate;

	void Clear()
	{
		device_type_value = -1;
		device_index = -1;
		channel_number = -1;
		baundrate = "";
	}
};


template <class K, class V>
class UnOrderMap
{
public:
	void insert(K key, QVector<V> v_vector)
	{
		bool exist_flag = false;
		int exist_index = -1;
		for (int i = 0; i < store_data.size(); i++)
		{
			if (store_data[i].first == key)
			{
				exist_index = i;
				exist_flag = true;
				break;
			}
		}
		if (exist_flag && exist_index != -1)
		{
			store_data[exist_index].second.append(v_vector);
		}
		else
		{
			QPair<K, QVector<V> > pair;
			pair.first = key;
			pair.second = v_vector;
			store_data.append(pair);
		}
	}

	void insert(K key, V value)
	{
		bool exist_flag = false;
		int exist_index = -1;
		for (int i = 0; i < store_data.size(); i++)
		{
			if (store_data[i].first == key)
			{
				exist_index = i;
				exist_flag = true;
				break;
			}
		}
		if (exist_flag && exist_index != -1)
		{
			store_data[exist_index].second.append(value);
		}
		else
		{
			QPair<K, QVector<V> > pair;
			pair.first = key;
			pair.second.append(value);
			store_data.append(pair);
		}
	}
	QPair<K, QVector<V> > & operator [](int index)
	{
		return store_data[index];
	}
	int size()
	{
		return store_data.size();
	}
	void clear()
	{
		store_data.clear();
	}

private:
	QVector<QPair<K, QVector<V> > > store_data;
};


class ECUData
{
public:
	ECUData()
	{
		ECUinfo_name = "";
		ECUinfo_value = "";
		writeECU_ID = "";

		store_name = "";
	}
	QString ECUinfo_name;
	QString ECUinfo_value;
	QString writeECU_ID;
	QVector<BYTE> code_vector;

	QString store_name;

	void Clear()
	{
		ECUinfo_name = "";
		ECUinfo_value = "";
		writeECU_ID = "";

		store_name = "";
	}
};

class SnapshotLib
{
public:
	SnapshotLib()
	{
		SnapshotDID = "";
		total_byte_length = 0;
		SnapshotDID_Data_map.clear();
	}
	QString SnapshotDID;
	int total_byte_length;
	QMap<QString, QString> SnapshotDID_Data_map;

	void Clear()
	{
		SnapshotDID = "";
		total_byte_length = 0;
		SnapshotDID_Data_map.clear();
	}
};

class SnapshotRecord
{
public:
	SnapshotRecord()
	{
		SnapshotRecord_index = 0;
		DID_num = 0;
		SnapshotLib_vector.clear();
	}
	int SnapshotRecord_index;
	int DID_num;

	QVector<SnapshotLib> SnapshotLib_vector;

	void Clear()
	{
		SnapshotRecord_index = 0;
		DID_num = 0;
		SnapshotLib_vector.clear();
	}
};


class SnapShotInfo
{
public:
	SnapShotInfo()
	{
		SnapshotRecord_vector.clear();
		data_vector.clear();
	}
	QVector<SnapshotRecord> SnapshotRecord_vector;
	QVector<BYTE> data_vector;

	void Clear()
	{
		SnapshotRecord_vector.clear();
		data_vector.clear();
	}
};

class FaultTableData
{
public:
	FaultTableData()
	{
		faultinfo_vector.clear();
	}
	QVector<FaultInfo> faultinfo_vector;
};

class FaultSortTableData
{
public:
	FaultSortTableData()
	{
		faultsort_vector.clear();
	}
	QVector<FaultSort> faultsort_vector;
};

class TreeData
{
public:
	TreeData()
	{
		tree_map.clear();
	}
	UnOrderMap<QString, UnOrderMap<QString, QString> > tree_map;
};

class ECUTableData
{
public:
	ECUTableData()
	{
		ecuinfo_vector.clear();
	}
	QVector<ECUData> ecuinfo_vector;
};

class Record
{
public:
	Record()
	{
		frame_index = -1;
	}
	qlonglong frame_index;
	VCI_CAN_OBJ this_frame;
};


class Filter_Setting
{
public:
	Filter_Setting()
	{
		filter_flag = false;
		receive_send_index = QStringLiteral("不限");
		frame_format_index = QStringLiteral("不限");
		frame_type_index = QStringLiteral("不限");
		ID_low_limit = -1;
		ID_high_limit = -1;
	}

	bool filter_flag;
	QString receive_send_index;
	QString frame_format_index;
	QString frame_type_index;
	qlonglong ID_low_limit;
	qlonglong ID_high_limit;

	void Clear()
	{
		filter_flag = false;
		receive_send_index = QStringLiteral("不限");
		frame_format_index = QStringLiteral("不限");
		frame_type_index = QStringLiteral("不限");
		ID_low_limit = -1;
		ID_high_limit = -1;
	}
};

class Display_Data
{
public:
	Display_Data()
	{
		display_index = "";
		receive_send_flag = "";
		time_internal = "";
		frame_id = "";
		frame_format = "";
		frame_type = "";
		data_length = "";
		data_content = "";
		time_stampt = "";

		frame_time_value = 0;
	}

public:

	QString display_index;
	QString receive_send_flag;
	QString time_internal;
	QString frame_id;
	QString frame_format;
	QString frame_type;
	QString data_length;
	QString data_content;
	QString time_stampt;
	UINT frame_time_value;

	void Clear()
	{
		display_index = "";
		receive_send_flag = "";
		frame_id = "";
		frame_format = "";
		frame_type = "";
		data_length = "";
		data_content = "";
		time_stampt = "";
		frame_time_value = 0;
	}
};

class SendData
{
public:
	SendData()
	{
		frame_id = 0;
		byte_length = 0;
		data_content = "";
		hex_data.clear();
		single_muti_frame_flag = 0;
		data_analyse_flag = false;
	}

	UINT frame_id;
	UINT byte_length;
	QString data_content;
	QVector<BYTE> hex_data;
	int single_muti_frame_flag;
	bool data_analyse_flag;
	void Clear()
	{
		frame_id = 0;
		byte_length = 0;
		data_content = "";
		hex_data.clear();
		single_muti_frame_flag = 0;
		data_analyse_flag = false;
	}
};


class LitteRecord
{
public:
	LitteRecord()
	{
		data_offset = 0;
		byte_length = 0;
		data_content = "";
		hex_data.clear();
		data_analyse_flag = false;
	}

	int data_offset;
	int byte_length;
	QString data_content;
	QVector<BYTE> hex_data;
	bool data_analyse_flag;
	void Clear()
	{
		data_offset = 0;
		byte_length = 0;
		data_content = "";
		hex_data.clear();
		data_analyse_flag = false;
	}
};

struct ModelInfo
{
	QString model_name;
	QString picture_path;
	QString settingfile_path;
	QString DiagnosisBased;
	QString Baud_Setting;

};

class ReceiveAnalyseData
{
public:
	ReceiveAnalyseData()
	{
		caption_content = "";
		Caculate_Type = -1;
		Unit = "";
		startbit = -1;
		bitlength = -1;
		invalid_value = -1;
		Formula = "";
		value_content_map.clear();
		Format = "";
		store_name = "";
	}
	QString caption_content;
	int Caculate_Type;//0：state,1:Numerical,2:ASCII,3:HEX
	QString Unit;
	int startbit;
	int bitlength;
	int invalid_value;
	QString Formula;
	QMap<int, QString> value_content_map;
	QString Format;
	QString store_name;

	void Clear()
	{
		caption_content = "";
		Caculate_Type = 0;
		Unit = "";
		startbit = 0;
		bitlength = 0;
		invalid_value = -1;
		Formula = "";
		value_content_map.clear();
		Format = "";
		store_name = "";
	}
};
class ReceiveDiagloseData
{
public:
	ReceiveDiagloseData()
	{
		frame_id = 0;
		byte_length = 0;
		data_record_vertor.clear();
		litte_record_vertor.clear();
	}
	UINT frame_id;
	UINT byte_length;
	
	QVector<ReceiveAnalyseData> data_record_vertor;
	QVector<LitteRecord> litte_record_vertor;
	void Clear()
	{
		frame_id = 0;
		byte_length = 0;
		data_record_vertor.clear();
		litte_record_vertor.clear();
	}
};

class ECUInfoData
{
public:
	ECUInfoData()
	{
		Caption = "";
		send_data.Clear();
		receive_data.Clear();
		writeECU_ID = "";
	}

	QString Caption;
	SendData send_data;
	ReceiveDiagloseData receive_data;
	QString writeECU_ID;
	void Clear()
	{
		Caption = "";
		send_data.Clear();
		receive_data.Clear();
		writeECU_ID = "";
	}
};

class WriteECUData
{
public:
	WriteECUData()
	{
		Caption = "";
		send_data.Clear();
		receive_data.Clear();
		writeECU_ID = "";
		lasttime_readvalue = "";
	}

	QString Caption;
	SendData send_data;
	ReceiveDiagloseData receive_data;
	QString writeECU_ID;
	QString lasttime_readvalue;
	void Clear()
	{
		Caption = "";
		send_data.Clear();
		receive_data.Clear();
		writeECU_ID = "";
		lasttime_readvalue = "";
	}
};

class InputWriteECUData
{
public:
	InputWriteECUData()
	{
		input_writedata_vector.clear();
	}
	QVector<WriteECUData> input_writedata_vector;
	void Clear()
	{
		input_writedata_vector.clear();
	}
};

class FunctionBlock
{
public:
	FunctionBlock()
	{
		send_frame_id = 0;
		receive_frame_id = 0;
		settingfile = "";
		model_name = "";
	}
	UINT receive_frame_id;
	UINT send_frame_id;
	QString settingfile;
	QString model_name;

	void Clear()
	{
		send_frame_id = 0;
		receive_frame_id = 0;
		settingfile = "";
		model_name = "";
	}
};

class LoaderData
{
public:
	LoaderData()
	{
		step_number = 0;
		special_item = "";
		Caption_content = "";
		Dely_Time = 0;
		CircleTimes_content = "";
		circle_times = 0;
		send_data.Clear();
		circletime_analyse_flag = false;
		receive_data.Clear();
	}

	int step_number;
	QString special_item;
	QString Caption_content;
	int Dely_Time;
	QString CircleTimes_content;
	int circle_times;
	SendData send_data;
	ReceiveDiagloseData receive_data;
	bool circletime_analyse_flag;
	void Clear()
	{
		step_number = 0;
		special_item = "";
		Caption_content = "";
		Dely_Time = 0;
		CircleTimes_content = "";
		circle_times = 0;
		send_data.Clear();
		circletime_analyse_flag = false;
		receive_data.Clear();
	}
};

class ControlData
{
public:
	ControlData()
	{
		item_caption = "";
		item_id = "";
		command_vector.clear();
	}
	QString item_caption;
	QString item_id;
	QVector<LoaderData> command_vector;
	void Clear()
	{
		item_caption = "";
		item_id = "";
		command_vector.clear();
	}
};

class PageInit :public QWidget
{
public:
	virtual void Init() = 0;
};


class Global
{
public:
	static int changeHexToUINT(QString temp_hex)
	{
		if (temp_hex == "")return 1000;
		int temp_value = 0;
		temp_hex = temp_hex.toUpper();
		if (temp_hex.mid(0,2) == "0X")temp_hex = temp_hex.mid(2, temp_hex.length() - 2);
		if (temp_hex.at(temp_hex.length() - 1) == ' ')temp_hex = temp_hex.mid(0, temp_hex.length() - 1);
		for (int i = 0; i < temp_hex.length(); i++)
		{
			if (!(temp_hex.at(i) >= 'A' && temp_hex.at(i) <= 'Z' || temp_hex.at(i) >= '0' && temp_hex.at(i) <= '9'))
				return 1001;
			int calculate_index = pow(16, temp_hex.length() - i - 1);
			temp_value += HexCharToUINT(temp_hex[i].toLatin1()) * calculate_index;
		}
		return temp_value;
	}

	static int HexCharToUINT(char str)
	{
		if (str >= 'A' && str <= 'Z')
		{
			return str - 55;
		}
		else if (str >= '0' && str <= '9')
		{
			return str - 48;
		}
		else return -1;
	}

	static QVector<BYTE> changeHexToByteVector(QString temp_hex)
	{
		QVector<BYTE> temp_byte_vector;
		if (temp_hex == "")return temp_byte_vector;
		int temp_value = 0;
		temp_hex = temp_hex.toUpper();
		if (temp_hex.indexOf("0X") != -1)
		{
			for (int i = 0; i < temp_hex.length(); i = i + 5)
			{
				if (temp_hex.mid(i, 2) == "0X")
				{
					BYTE temp_byte = changeHexToUINT(temp_hex.mid(i + 2, 2));
					if ((int)temp_byte < 256)temp_byte_vector.push_back(temp_byte);
				}
			}
		}
		else
		{
			for (int i = 0; i < temp_hex.length(); i = i + 3)
			{
				BYTE temp_byte = changeHexToUINT(temp_hex.mid(i, 2));
				if ((int)temp_byte < 256)temp_byte_vector.push_back(temp_byte);
			}
		}
		return temp_byte_vector;
	}

	static int changeByteVectorToInt(QVector<BYTE> temp_byte_vector)
	{
		int temp_value = 0;
		if (temp_byte_vector.size() > 3)return temp_value;
		for (int i = 0; i < temp_byte_vector.size(); i++)
		{
			qlonglong temp_parameter = pow(256,temp_byte_vector.size() - i - 1);
			temp_value += temp_byte_vector[i] * temp_parameter;
		}
		return temp_value;
	}

	static void Display_prompt(int type, QString message)
	{
		Prompt* temp_prompt_dialog = new Prompt(type, message);
		temp_prompt_dialog->show();
	}

	static qlonglong timeToMillisecond(QString time_data)
	{
		if (time_data.length() != 12 && time_data.length() != 14)return 0;//分别对应两种文件格式，GLB_Spy(时间12位)，CANTest(时间14位)
		unsigned int temp_data[4] = { 0 };

		temp_data[0] = time_data.mid(0, 2).toInt();
		temp_data[1] = time_data.mid(3, 2).toInt();
		temp_data[2] = time_data.mid(6, 2).toInt();
		temp_data[3] = time_data.mid(9, 3).toInt();

		qlonglong temp_time = (qlonglong)(temp_data[0] * 3600 + temp_data[1] * 60 + temp_data[2]) * 1000 + (qlonglong)temp_data[3];
		return temp_time;
	}

	static QVector<QString> AnanlyseOneLine(QString line_data)
	{
		QVector<QString> temp_tile_vector;
		int pos = 0;
		while (1)//解析前面8列，以逗号为区分
		{
			QRegExp rx1("(\n((?!,).)*,)");
			int temp_index = -1;
			if ((temp_index = rx1.indexIn(line_data, pos)) != -1)
			{
				pos = temp_index;
				QString temp_data2 = rx1.cap(1);
				if (temp_data2.length() > 1)temp_data2 = temp_data2.mid(1, temp_data2.length() - 2);
				pos += rx1.matchedLength();
				if (temp_data2 != "")temp_tile_vector.push_back(temp_data2);
			}
			else
			{
				QRegExp rx2("(((?!,).)*,)");
				if (rx2.indexIn(line_data, pos) != -1)
				{
					QString temp_data2 = rx2.cap(1);
					if (temp_data2.length() > 1)temp_data2 = temp_data2.mid(0, temp_data2.length() - 1);
					pos += rx2.matchedLength();
					if (temp_data2 != "")temp_tile_vector.push_back(temp_data2);
				}
				else
				{
					break;
				}
			}
		}
		QRegExp rx3("(((?!,).)*)");//最后一列
		if (rx3.indexIn(line_data, pos) != -1)
		{
			QString temp_data3 = rx3.cap(1);
			pos += rx3.matchedLength();
			if (temp_data3 != "")temp_tile_vector.push_back(temp_data3);//保存解析的数据
		}
		return temp_tile_vector;
	}

	static QString GetFramePrintContent(VCI_CAN_OBJ temp_signal_data, int send_receive_flag)
	{
		QString temp_print;

		if (send_receive_flag == 0)
		{
			temp_print += QStringLiteral("发送 ");
			temp_print += "0000000000 ";
		}
		else if(send_receive_flag == 1)
		{
			temp_print += QStringLiteral("接收 ");
			temp_print += "0x" + QString("%1").arg(temp_signal_data.TimeStamp, 8, 16, QLatin1Char('0'));
			temp_print += " ";
		}

		temp_print += "0x" + QString("%1").arg(temp_signal_data.ID, 8, 16, QLatin1Char('0'));
		temp_print += " ";

		if (temp_signal_data.RemoteFlag == 0)
		{
			temp_print += QStringLiteral("数据帧 ");
		}
		else if (temp_signal_data.RemoteFlag == 1)
		{
			temp_print += QStringLiteral("远程帧 ");
		}

		if (temp_signal_data.ExternFlag == 0)
		{
			temp_print += QStringLiteral("标准帧 ");
		}
		else if (temp_signal_data.ExternFlag == 1)
		{
			temp_print += QStringLiteral("扩展帧 ");
		}

		temp_print += QString("%1").arg(temp_signal_data.DataLen, 2, 10, QLatin1Char('0'));
		temp_print += " ";

		for (int j = 0; j < temp_signal_data.DataLen; j++)
		{
			QString temp_str = QString("%1").arg(temp_signal_data.Data[j], 2, 16, QLatin1Char('0')).toUpper();
			temp_str += " ";
			temp_print += temp_str;
		}
		temp_print.chop(1);
		return temp_print;
	}

	static int GetDeviceTypeValue(QString temp_devicetype)//获得设备类型对应的值
	{
		int m_devtype = -1;
		QMap<QString, int> device_type;
		device_type.insert("VCI_PCI5121", 1);
		device_type.insert("VCI_PCI9810", 2);
		device_type.insert("VCI_USBCAN1", 3);
		device_type.insert("VCI_USBCAN2", 4);
		device_type.insert("VCI_USBCAN2A", 4);
		device_type.insert("VCI_PCI9820", 5);
		device_type.insert("VCI_CAN232", 6);
		device_type.insert("VCI_PCI5110", 7);
		device_type.insert("VCI_CANLITE", 8);
		device_type.insert("VCI_ISA9620", 9);
		device_type.insert("VCI_ISA5420", 10);
		device_type.insert("VCI_PC104CAN", 11);
		device_type.insert("VCI_CANETUDP", 12);
		device_type.insert("VCI_CANETE", 12);
		device_type.insert("VCI_DNP9810", 13);
		device_type.insert("VCI_PCI9840", 14);
		device_type.insert("VCI_PC104CAN2", 15);
		device_type.insert("VCI_PCI9820I", 16);
		device_type.insert("VCI_CANETTCP", 17);
		device_type.insert("VCI_PEC9920", 18);
		device_type.insert("VCI_PCIE_9220", 18);
		device_type.insert("VCI_PCI5010U", 19);
		device_type.insert("VCI_USBCAN_E_U", 20);
		device_type.insert("VCI_USBCAN_2E_U", 21);
		device_type.insert("VCI_PCI5020U", 22);
		device_type.insert("VCI_EG20T_CAN", 23);
		device_type.insert("VCI_PCIE9221", 24);

		QMap<QString, int>::iterator  mi;
		int temp_flag = -1;
		for (mi = device_type.begin(); mi != device_type.end(); ++mi)
		{
			temp_flag++;
			if (temp_devicetype == mi.key())
			{
				m_devtype = mi.value();
				break;
			}
		}
		return m_devtype;
	}

	static void loadQSS(QWidget* m_form, QString qss_path)
	{
		QString stylefile_path = QCoreApplication::applicationDirPath() + "/qss/" + qss_path;
		QFile qss(stylefile_path);
		if (qss.open(QFile::ReadOnly))
		{
			m_form->setStyleSheet(qss.readAll());
			qss.close();
		}
	}

};

class NoFocusRectangleStyle :public QStyledItemDelegate
{
	void paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
	{
		QStyleOptionViewItem itemOption(option);
		if (itemOption.state & QStyle::State_HasFocus)
		{
			itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
		}
		QStyledItemDelegate::paint(painter, itemOption, index);
	}
};

Q_DECLARE_METATYPE(VCI_CAN_OBJ);
Q_DECLARE_METATYPE(TreeData);
Q_DECLARE_METATYPE(ECUTableData);
Q_DECLARE_METATYPE(InputWriteECUData);
Q_DECLARE_METATYPE(FaultTableData);
Q_DECLARE_METATYPE(SnapShotInfo);
Q_DECLARE_METATYPE(Exist_Device);
Q_DECLARE_METATYPE(FaultSortTableData);

