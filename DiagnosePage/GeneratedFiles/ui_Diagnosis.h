/********************************************************************************
** Form generated from reading UI file 'Diagnosis.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIAGNOSIS_H
#define UI_DIAGNOSIS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_3;
    QWidget *top_widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *title_label;
    QPushButton *min_pushButton;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *close_pushButton;
    QSpacerItem *horizontalSpacer_3;
    QWidget *content_widget;
    QHBoxLayout *horizontalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QWidget *widget;
    QVBoxLayout *verticalLayout_2;
    QListWidget *listWidget;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QWidget *page_2;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        verticalLayout_3 = new QVBoxLayout(centralwidget);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        top_widget = new QWidget(centralwidget);
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

        min_pushButton = new QPushButton(top_widget);
        min_pushButton->setObjectName(QStringLiteral("min_pushButton"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(min_pushButton->sizePolicy().hasHeightForWidth());
        min_pushButton->setSizePolicy(sizePolicy2);
        min_pushButton->setMaximumSize(QSize(12, 12));
        min_pushButton->setStyleSheet(QStringLiteral(""));

        horizontalLayout->addWidget(min_pushButton);

        horizontalSpacer_2 = new QSpacerItem(5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        close_pushButton = new QPushButton(top_widget);
        close_pushButton->setObjectName(QStringLiteral("close_pushButton"));
        sizePolicy2.setHeightForWidth(close_pushButton->sizePolicy().hasHeightForWidth());
        close_pushButton->setSizePolicy(sizePolicy2);
        close_pushButton->setMaximumSize(QSize(12, 12));
        close_pushButton->setStyleSheet(QStringLiteral(""));

        horizontalLayout->addWidget(close_pushButton);

        horizontalSpacer_3 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_3->addWidget(top_widget);

        content_widget = new QWidget(centralwidget);
        content_widget->setObjectName(QStringLiteral("content_widget"));
        horizontalLayout_3 = new QHBoxLayout(content_widget);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        widget = new QWidget(content_widget);
        widget->setObjectName(QStringLiteral("widget"));
        verticalLayout_2 = new QVBoxLayout(widget);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, -1, 0, -1);
        listWidget = new QListWidget(widget);
        listWidget->setObjectName(QStringLiteral("listWidget"));

        verticalLayout_2->addWidget(listWidget);


        horizontalLayout_2->addWidget(widget);

        stackedWidget = new QStackedWidget(content_widget);
        stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
        page = new QWidget();
        page->setObjectName(QStringLiteral("page"));
        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QStringLiteral("page_2"));
        stackedWidget->addWidget(page_2);

        horizontalLayout_2->addWidget(stackedWidget);


        horizontalLayout_3->addLayout(horizontalLayout_2);


        verticalLayout_3->addWidget(content_widget);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        stackedWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
        title_label->setText(QString());
        min_pushButton->setText(QString());
        close_pushButton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIAGNOSIS_H
