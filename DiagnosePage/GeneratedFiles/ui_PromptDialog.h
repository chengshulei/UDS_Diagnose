/********************************************************************************
** Form generated from reading UI file 'PromptDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROMPTDIALOG_H
#define UI_PROMPTDIALOG_H

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

class Ui_Prompt_Dialog
{
public:
    QVBoxLayout *verticalLayout_4;
    QWidget *prompt_top_widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *prompt_title_label;
    QPushButton *prompt_close_pushButton;
    QSpacerItem *horizontalSpacer_3;
    QWidget *prompt_content_widget;
    QVBoxLayout *verticalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_6;
    QPushButton *prompt_ico_pushButton;
    QSpacerItem *horizontalSpacer_4;
    QLabel *prompt_message_label;
    QSpacerItem *verticalSpacer;
    QSpacerItem *horizontalSpacer_5;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *verticalSpacer_2;
    QPushButton *prompt_sure_pushButton;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QDialog *Prompt_Dialog)
    {
        if (Prompt_Dialog->objectName().isEmpty())
            Prompt_Dialog->setObjectName(QStringLiteral("Prompt_Dialog"));
        Prompt_Dialog->resize(505, 260);
        verticalLayout_4 = new QVBoxLayout(Prompt_Dialog);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        prompt_top_widget = new QWidget(Prompt_Dialog);
        prompt_top_widget->setObjectName(QStringLiteral("prompt_top_widget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(prompt_top_widget->sizePolicy().hasHeightForWidth());
        prompt_top_widget->setSizePolicy(sizePolicy);
        prompt_top_widget->setMinimumSize(QSize(0, 40));
        prompt_top_widget->setMaximumSize(QSize(16777215, 40));
        prompt_top_widget->setStyleSheet(QStringLiteral(""));
        verticalLayout = new QVBoxLayout(prompt_top_widget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        prompt_title_label = new QLabel(prompt_top_widget);
        prompt_title_label->setObjectName(QStringLiteral("prompt_title_label"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(prompt_title_label->sizePolicy().hasHeightForWidth());
        prompt_title_label->setSizePolicy(sizePolicy1);
        prompt_title_label->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(prompt_title_label);

        prompt_close_pushButton = new QPushButton(prompt_top_widget);
        prompt_close_pushButton->setObjectName(QStringLiteral("prompt_close_pushButton"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(prompt_close_pushButton->sizePolicy().hasHeightForWidth());
        prompt_close_pushButton->setSizePolicy(sizePolicy2);
        prompt_close_pushButton->setMaximumSize(QSize(12, 12));
        prompt_close_pushButton->setStyleSheet(QStringLiteral(""));

        horizontalLayout->addWidget(prompt_close_pushButton);

        horizontalSpacer_3 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_4->addWidget(prompt_top_widget);

        prompt_content_widget = new QWidget(Prompt_Dialog);
        prompt_content_widget->setObjectName(QStringLiteral("prompt_content_widget"));
        verticalLayout_3 = new QVBoxLayout(prompt_content_widget);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_6);

        prompt_ico_pushButton = new QPushButton(prompt_content_widget);
        prompt_ico_pushButton->setObjectName(QStringLiteral("prompt_ico_pushButton"));
        sizePolicy2.setHeightForWidth(prompt_ico_pushButton->sizePolicy().hasHeightForWidth());
        prompt_ico_pushButton->setSizePolicy(sizePolicy2);
        prompt_ico_pushButton->setMinimumSize(QSize(72, 72));
        prompt_ico_pushButton->setMaximumSize(QSize(72, 72));

        horizontalLayout_3->addWidget(prompt_ico_pushButton);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);

        prompt_message_label = new QLabel(prompt_content_widget);
        prompt_message_label->setObjectName(QStringLiteral("prompt_message_label"));
        prompt_message_label->setWordWrap(true);

        horizontalLayout_3->addWidget(prompt_message_label);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout_3->addItem(verticalSpacer);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_5);


        verticalLayout_2->addLayout(horizontalLayout_3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout_2->addItem(verticalSpacer_2);

        prompt_sure_pushButton = new QPushButton(prompt_content_widget);
        prompt_sure_pushButton->setObjectName(QStringLiteral("prompt_sure_pushButton"));

        horizontalLayout_2->addWidget(prompt_sure_pushButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout_2->addLayout(horizontalLayout_2);


        verticalLayout_3->addLayout(verticalLayout_2);


        verticalLayout_4->addWidget(prompt_content_widget);

        prompt_top_widget->raise();
        prompt_title_label->raise();
        prompt_ico_pushButton->raise();
        prompt_message_label->raise();
        prompt_sure_pushButton->raise();
        prompt_content_widget->raise();

        retranslateUi(Prompt_Dialog);

        QMetaObject::connectSlotsByName(Prompt_Dialog);
    } // setupUi

    void retranslateUi(QDialog *Prompt_Dialog)
    {
        Prompt_Dialog->setWindowTitle(QApplication::translate("Prompt_Dialog", "Dialog", Q_NULLPTR));
        prompt_title_label->setText(QString());
        prompt_close_pushButton->setText(QString());
        prompt_ico_pushButton->setText(QString());
        prompt_message_label->setText(QApplication::translate("Prompt_Dialog", "TextLabel", Q_NULLPTR));
        prompt_sure_pushButton->setText(QApplication::translate("Prompt_Dialog", "PushButton", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class Prompt_Dialog: public Ui_Prompt_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROMPTDIALOG_H
