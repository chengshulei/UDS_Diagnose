/********************************************************************************
** Form generated from reading UI file 'Communication.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Communication_Dialog
{
public:
    QVBoxLayout *verticalLayout_7;
    QWidget *communication_top_widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *communication_title_label;
    QPushButton *communication_close_pushButton;
    QSpacerItem *horizontalSpacer_3;
    QWidget *communication_content_widget;
    QVBoxLayout *verticalLayout_6;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer_7;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_4;
    QLabel *device_type_label;
    QSpacerItem *verticalSpacer_3;
    QLabel *device_type_content;
    QHBoxLayout *horizontalLayout_5;
    QLabel *baudrate_label;
    QSpacerItem *verticalSpacer_5;
    QLabel *baudrate_content;
    QHBoxLayout *horizontalLayout_7;
    QLabel *device_index_label;
    QSpacerItem *verticalSpacer_4;
    QComboBox *device_index_comboBox;
    QHBoxLayout *horizontalLayout_8;
    QLabel *can_channel_label;
    QSpacerItem *verticalSpacer_6;
    QComboBox *can_channel_comboBox;
    QSpacerItem *horizontalSpacer_8;
    QHBoxLayout *horizontalLayout_9;
    QSpacerItem *horizontalSpacer_9;
    QSpacerItem *verticalSpacer_7;
    QPushButton *commucation_sure_pushButton;
    QSpacerItem *horizontalSpacer_10;

    void setupUi(QDialog *Communication_Dialog)
    {
        if (Communication_Dialog->objectName().isEmpty())
            Communication_Dialog->setObjectName(QStringLiteral("Communication_Dialog"));
        Communication_Dialog->resize(539, 334);
        verticalLayout_7 = new QVBoxLayout(Communication_Dialog);
        verticalLayout_7->setSpacing(0);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        verticalLayout_7->setContentsMargins(0, 0, 0, 0);
        communication_top_widget = new QWidget(Communication_Dialog);
        communication_top_widget->setObjectName(QStringLiteral("communication_top_widget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(communication_top_widget->sizePolicy().hasHeightForWidth());
        communication_top_widget->setSizePolicy(sizePolicy);
        communication_top_widget->setMinimumSize(QSize(0, 40));
        communication_top_widget->setMaximumSize(QSize(16777215, 40));
        communication_top_widget->setStyleSheet(QStringLiteral(""));
        verticalLayout = new QVBoxLayout(communication_top_widget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        communication_title_label = new QLabel(communication_top_widget);
        communication_title_label->setObjectName(QStringLiteral("communication_title_label"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(communication_title_label->sizePolicy().hasHeightForWidth());
        communication_title_label->setSizePolicy(sizePolicy1);
        communication_title_label->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(communication_title_label);

        communication_close_pushButton = new QPushButton(communication_top_widget);
        communication_close_pushButton->setObjectName(QStringLiteral("communication_close_pushButton"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(communication_close_pushButton->sizePolicy().hasHeightForWidth());
        communication_close_pushButton->setSizePolicy(sizePolicy2);
        communication_close_pushButton->setMaximumSize(QSize(12, 12));
        communication_close_pushButton->setStyleSheet(QStringLiteral(""));

        horizontalLayout->addWidget(communication_close_pushButton);

        horizontalSpacer_3 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_7->addWidget(communication_top_widget);

        communication_content_widget = new QWidget(Communication_Dialog);
        communication_content_widget->setObjectName(QStringLiteral("communication_content_widget"));
        verticalLayout_6 = new QVBoxLayout(communication_content_widget);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_7);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        device_type_label = new QLabel(communication_content_widget);
        device_type_label->setObjectName(QStringLiteral("device_type_label"));

        horizontalLayout_4->addWidget(device_type_label);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout_4->addItem(verticalSpacer_3);

        device_type_content = new QLabel(communication_content_widget);
        device_type_content->setObjectName(QStringLiteral("device_type_content"));

        horizontalLayout_4->addWidget(device_type_content);


        verticalLayout_4->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        baudrate_label = new QLabel(communication_content_widget);
        baudrate_label->setObjectName(QStringLiteral("baudrate_label"));

        horizontalLayout_5->addWidget(baudrate_label);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout_5->addItem(verticalSpacer_5);

        baudrate_content = new QLabel(communication_content_widget);
        baudrate_content->setObjectName(QStringLiteral("baudrate_content"));

        horizontalLayout_5->addWidget(baudrate_content);


        verticalLayout_4->addLayout(horizontalLayout_5);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        device_index_label = new QLabel(communication_content_widget);
        device_index_label->setObjectName(QStringLiteral("device_index_label"));

        horizontalLayout_7->addWidget(device_index_label);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout_7->addItem(verticalSpacer_4);

        device_index_comboBox = new QComboBox(communication_content_widget);
        device_index_comboBox->setObjectName(QStringLiteral("device_index_comboBox"));

        horizontalLayout_7->addWidget(device_index_comboBox);


        verticalLayout_4->addLayout(horizontalLayout_7);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        can_channel_label = new QLabel(communication_content_widget);
        can_channel_label->setObjectName(QStringLiteral("can_channel_label"));

        horizontalLayout_8->addWidget(can_channel_label);

        verticalSpacer_6 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout_8->addItem(verticalSpacer_6);

        can_channel_comboBox = new QComboBox(communication_content_widget);
        can_channel_comboBox->setObjectName(QStringLiteral("can_channel_comboBox"));

        horizontalLayout_8->addWidget(can_channel_comboBox);


        verticalLayout_4->addLayout(horizontalLayout_8);


        horizontalLayout_6->addLayout(verticalLayout_4);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_8);


        verticalLayout_5->addLayout(horizontalLayout_6);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QStringLiteral("horizontalLayout_9"));
        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_9);

        verticalSpacer_7 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout_9->addItem(verticalSpacer_7);

        commucation_sure_pushButton = new QPushButton(communication_content_widget);
        commucation_sure_pushButton->setObjectName(QStringLiteral("commucation_sure_pushButton"));

        horizontalLayout_9->addWidget(commucation_sure_pushButton);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_10);


        verticalLayout_5->addLayout(horizontalLayout_9);


        verticalLayout_6->addLayout(verticalLayout_5);


        verticalLayout_7->addWidget(communication_content_widget);


        retranslateUi(Communication_Dialog);

        QMetaObject::connectSlotsByName(Communication_Dialog);
    } // setupUi

    void retranslateUi(QDialog *Communication_Dialog)
    {
        Communication_Dialog->setWindowTitle(QApplication::translate("Communication_Dialog", "Dialog", Q_NULLPTR));
        communication_title_label->setText(QString());
        communication_close_pushButton->setText(QString());
        device_type_label->setText(QApplication::translate("Communication_Dialog", "TextLabel", Q_NULLPTR));
        device_type_content->setText(QApplication::translate("Communication_Dialog", "TextLabel", Q_NULLPTR));
        baudrate_label->setText(QApplication::translate("Communication_Dialog", "TextLabel", Q_NULLPTR));
        baudrate_content->setText(QApplication::translate("Communication_Dialog", "TextLabel", Q_NULLPTR));
        device_index_label->setText(QApplication::translate("Communication_Dialog", "TextLabel", Q_NULLPTR));
        can_channel_label->setText(QApplication::translate("Communication_Dialog", "TextLabel", Q_NULLPTR));
        commucation_sure_pushButton->setText(QApplication::translate("Communication_Dialog", "PushButton", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class Communication_Dialog: public Ui_Communication_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // COMMUNICATION_H
