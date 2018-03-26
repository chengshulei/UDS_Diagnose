#pragma once

#include <QDialog>
#include "PromptDialog.h"
#include <QMouseEvent>

class Prompt : public QDialog
{
	Q_OBJECT

public:
	Prompt(int temp_type,QString temp_message);
	~Prompt();
	int prompt_type;
	QString prompt_message;

public:
signals :
	public slots :
	void close_windowSlot();

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);

	bool        mMoveing;
	QPoint      mMovePosition;

private:
	Ui_Prompt_Dialog ui;


};
