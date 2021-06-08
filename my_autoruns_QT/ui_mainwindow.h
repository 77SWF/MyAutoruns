/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QTableWidget *autoruns_table;
    QPushButton *logon;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        autoruns_table = new QTableWidget(centralwidget);
        if (autoruns_table->columnCount() < 6)
            autoruns_table->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        autoruns_table->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        __qtablewidgetitem1->setTextAlignment(Qt::AlignCenter);
        autoruns_table->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        __qtablewidgetitem2->setTextAlignment(Qt::AlignCenter);
        autoruns_table->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        __qtablewidgetitem3->setTextAlignment(Qt::AlignCenter);
        autoruns_table->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        __qtablewidgetitem4->setTextAlignment(Qt::AlignCenter);
        autoruns_table->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        __qtablewidgetitem5->setTextAlignment(Qt::AlignCenter);
        autoruns_table->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        autoruns_table->setObjectName(QString::fromUtf8("autoruns_table"));
        autoruns_table->setGeometry(QRect(10, 30, 1080, 720));
        autoruns_table->setMinimumSize(QSize(0, 471));
        autoruns_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        autoruns_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        autoruns_table->setSelectionMode(QAbstractItemView::SingleSelection);
        autoruns_table->setSelectionBehavior(QAbstractItemView::SelectRows);
        autoruns_table->setRowCount(0);
        autoruns_table->setColumnCount(6);
        autoruns_table->horizontalHeader()->setCascadingSectionResizes(false);
        autoruns_table->horizontalHeader()->setDefaultSectionSize(100);
        autoruns_table->horizontalHeader()->setStretchLastSection(true);
        autoruns_table->verticalHeader()->setVisible(false);
        autoruns_table->verticalHeader()->setCascadingSectionResizes(false);
        autoruns_table->verticalHeader()->setStretchLastSection(false);
        logon = new QPushButton(centralwidget);
        logon->setObjectName(QString::fromUtf8("logon"));
        logon->setGeometry(QRect(10, 0, 80, 20));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        QTableWidgetItem *___qtablewidgetitem = autoruns_table->horizontalHeaderItem(1);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "Autorun Entry", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = autoruns_table->horizontalHeaderItem(2);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Description", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = autoruns_table->horizontalHeaderItem(3);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Publisher", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = autoruns_table->horizontalHeaderItem(4);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "Image Path", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = autoruns_table->horizontalHeaderItem(5);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "Timestamp", nullptr));
        logon->setText(QCoreApplication::translate("MainWindow", "logon", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
