#pragma once

#include <QWidget>
#include "writeECU.h"
#include "Global.h"
#include <QMap>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

class WriteECUPage : public QDialog
{
	Q_OBJECT

public:
	WriteECUPage(QVector<WriteECUData> temp_vector, QMap<QString, QVector<BYTE> > temp_map, ECUData temp_item_data);
	~WriteECUPage();

	QVector<WriteECUData> current_wirtedata_vector;
	QMap<QString, QString> name_change_list;
	bool sure_flag;
	QMap<QString, QVector<BYTE> > content_map;
	ECUData ECUinfo_data;

	void AnalyseWriteECUData();

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);

	bool        mMoveing;
	QPoint      mMovePosition;

signals:
	void input_resultSignal(InputWriteECUData);
	public slots :
	void Sure_Slot();
	void close_windowSlot();

private:
	Ui_writeECU_Dialog ui;
};
