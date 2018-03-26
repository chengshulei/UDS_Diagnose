/********************************************************************************
** Form generated from reading UI file 'SelectModel.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SELECTMODEL_H
#define UI_SELECTMODEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>

QT_BEGIN_NAMESPACE

class Ui_SelectModelDialog
{
public:

    void setupUi(QDialog *SelectModelDialog)
    {
        if (SelectModelDialog->objectName().isEmpty())
            SelectModelDialog->setObjectName(QStringLiteral("SelectModelDialog"));
        SelectModelDialog->resize(833, 615);

        retranslateUi(SelectModelDialog);

        QMetaObject::connectSlotsByName(SelectModelDialog);
    } // setupUi

    void retranslateUi(QDialog *SelectModelDialog)
    {
        SelectModelDialog->setWindowTitle(QApplication::translate("SelectModelDialog", "Dialog", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class SelectModelDialog: public Ui_SelectModelDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SELECTMODEL_H
