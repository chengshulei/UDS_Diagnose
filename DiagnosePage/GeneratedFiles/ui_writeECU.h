/********************************************************************************
** Form generated from reading UI file 'writeECU.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WRITEECU_H
#define UI_WRITEECU_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_writeECU_Dialog
{
public:
    QVBoxLayout *verticalLayout_4;
    QWidget *top_widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *title_label;
    QPushButton *close_pushButton;
    QSpacerItem *horizontalSpacer_3;
    QWidget *content_widget;

    void setupUi(QDialog *writeECU_Dialog)
    {
        if (writeECU_Dialog->objectName().isEmpty())
            writeECU_Dialog->setObjectName(QStringLiteral("writeECU_Dialog"));
        writeECU_Dialog->resize(679, 451);
        verticalLayout_4 = new QVBoxLayout(writeECU_Dialog);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        top_widget = new QWidget(writeECU_Dialog);
        top_widget->setObjectName(QStringLiteral("top_widget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(top_widget->sizePolicy().hasHeightForWidth());
        top_widget->setSizePolicy(sizePolicy);
        top_widget->setMinimumSize(QSize(0, 40));
        top_widget->setMaximumSize(QSize(16777215, 40));
        top_widget->setStyleSheet(QStringLiteral(""));
        verticalLayout = new QVBoxLayout(top_widget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        title_label = new QLabel(top_widget);
        title_label->setObjectName(QStringLiteral("title_label"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(title_label->sizePolicy().hasHeightForWidth());
        title_label->setSizePolicy(sizePolicy1);
        title_label->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(title_label);

        close_pushButton = new QPushButton(top_widget);
        close_pushButton->setObjectName(QStringLiteral("close_pushButton"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(close_pushButton->sizePolicy().hasHeightForWidth());
        close_pushButton->setSizePolicy(sizePolicy2);
        close_pushButton->setMaximumSize(QSize(12, 12));
        close_pushButton->setStyleSheet(QStringLiteral(""));

        horizontalLayout->addWidget(close_pushButton);

        horizontalSpacer_3 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_4->addWidget(top_widget);

        content_widget = new QWidget(writeECU_Dialog);
        content_widget->setObjectName(QStringLiteral("content_widget"));

        verticalLayout_4->addWidget(content_widget);


        retranslateUi(writeECU_Dialog);

        QMetaObject::connectSlotsByName(writeECU_Dialog);
    } // setupUi

    void retranslateUi(QDialog *writeECU_Dialog)
    {
        writeECU_Dialog->setWindowTitle(QApplication::translate("writeECU_Dialog", "Dialog", Q_NULLPTR));
        title_label->setText(QString());
        close_pushButton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class writeECU_Dialog: public Ui_writeECU_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WRITEECU_H
