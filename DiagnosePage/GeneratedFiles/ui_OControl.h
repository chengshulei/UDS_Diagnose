/********************************************************************************
** Form generated from reading UI file 'OControl.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OCONTROL_H
#define UI_OCONTROL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OConrolForm
{
public:
    QHBoxLayout *horizontalLayout_3;
    QListWidget *listWidget;
    QWidget *widget;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *pushButton;
    QSpacerItem *horizontalSpacer_3;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *pushButton_2;
    QSpacerItem *horizontalSpacer_5;

    void setupUi(QWidget *OConrolForm)
    {
        if (OConrolForm->objectName().isEmpty())
            OConrolForm->setObjectName(QStringLiteral("OConrolForm"));
        OConrolForm->resize(680, 365);
        horizontalLayout_3 = new QHBoxLayout(OConrolForm);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        listWidget = new QListWidget(OConrolForm);
        listWidget->setObjectName(QStringLiteral("listWidget"));

        horizontalLayout_3->addWidget(listWidget);

        widget = new QWidget(OConrolForm);
        widget->setObjectName(QStringLiteral("widget"));
        verticalLayout_2 = new QVBoxLayout(widget);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label = new QLabel(widget);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        lineEdit = new QLineEdit(widget);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        horizontalLayout->addWidget(lineEdit);

        pushButton = new QPushButton(widget);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout->addWidget(pushButton);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);

        pushButton_2 = new QPushButton(widget);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));

        horizontalLayout_2->addWidget(pushButton_2);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_5);


        verticalLayout->addLayout(horizontalLayout_2);


        verticalLayout_2->addLayout(verticalLayout);


        horizontalLayout_3->addWidget(widget);


        retranslateUi(OConrolForm);

        QMetaObject::connectSlotsByName(OConrolForm);
    } // setupUi

    void retranslateUi(QWidget *OConrolForm)
    {
        OConrolForm->setWindowTitle(QApplication::translate("OConrolForm", "Form", Q_NULLPTR));
        label->setText(QApplication::translate("OConrolForm", "TextLabel", Q_NULLPTR));
        pushButton->setText(QApplication::translate("OConrolForm", "PushButton", Q_NULLPTR));
        pushButton_2->setText(QApplication::translate("OConrolForm", "PushButton", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class OConrolForm: public Ui_OConrolForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OCONTROL_H
