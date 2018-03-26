#pragma once

#include <QDialog>
#include "Communication.h"
#include "Global.h"
#include <QMouseEvent>

class CommunicationPage : public QDialog
{
	Q_OBJECT

public:
	CommunicationPage(ModelInfo temp_model);
	~CommunicationPage();

	void closeEvent(QCloseEvent * e);

	ModelInfo select_model;
	bool sure_flag;

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);

	bool        mMoveing;
	QPoint      mMovePosition;

signals:
	void communication_resultSignal(Exist_Device);
	public slots :
	void Sure_Slot();
	void close_windowSlot();

private:
	Ui_Communication_Dialog ui;
};
