#pragma once

#include <QObject>
#include "Global.h"

class DealSettingFile : public QObject
{
	Q_OBJECT

public:
	DealSettingFile();
	~DealSettingFile();

	QString vehiclefile_path;
	QString settingfile_path;
	QVector<QString> item_vector;
	
	QVector<LoaderData> AnalyseDiagnoseSettingFile(QString item_name);
	QString GetCaption(QString file_data, QString caption_id);
	SendData GetSendData(QString file_data, QString send_id);
	ReceiveDiagloseData GetReceiveData(QString file_data, QString receive_id);
	ReceiveAnalyseData GetByteAnalyseData(QString file_data, QString ByteAnalyse_id);
	QMap<int, QString> GetContent(QString file_data, QString content_id);
	void TableWrite(QString message_content, int message_type);
	QVector<ECUInfoData> GetReadECUData();
	QVector<WriteECUData> GetWriteECUData();
	QVector<ControlData> GetOperateControlData();
	QVector<ControlData> GetRoutineControlData();
	QVector<ModelInfo> GetVehicleModel();
	QVector<LoaderData> AnalyseDiagnoseVehicleFile(QString DiagnosisBased_content);
	QMap<int, QString> GetPageList();
	LoaderData GetSingleCommandFromDiagnoseSettingFile(QString function_name, QString command_caption);
	ReceiveDiagloseData AnalyseSnapshotLib(QString function_name, QString SnapshotDID);
	QString GetByteAnalyseID();
	FaultInfo GetFaultDetailInfo(QString temp_fault_code);
	QString GetFaultCauses(QString FaultCauses_ID);
	QString GetCorrecticveAction(QString CorrecticveAction_ID);
	QString GetByteAnalyseTimeState(QString ByteAnalyse_id, BYTE analyse_byte);

public:
signals :
		void display_messageSignal(QString, int);
		public slots:
};
