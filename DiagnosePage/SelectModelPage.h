#pragma once

#include "SelectModel.h"
#include <QPainter>
#include <QPaintEvent>
#include "Global.h"
#include <QStandardItemModel>
#include <QVector>
#include <QDialog>


class SelectModelPage : public QDialog
{
	Q_OBJECT

public:
	SelectModelPage(QDialog *parent = Q_NULLPTR);
	~SelectModelPage();

	QVector<ModelInfo> readfile_info;

	void Init();
	void TableWrite(QString message_content, int message_type);

signals:
	void selectmodelsignal(ModelInfo);
	void display_messageSignal(QString, int);
	public slots :
	void SelectModelSlot();

private:
	Ui_SelectModelDialog ui;
};