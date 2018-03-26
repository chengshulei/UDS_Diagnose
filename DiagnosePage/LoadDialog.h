#pragma once

#include <QDialog>
#include "Loading.h"

class LoadDialog : public QDialog
{
	Q_OBJECT

public:
	LoadDialog();
	~LoadDialog();

private:
	Ui_Dialog ui;
};
