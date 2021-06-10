#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef WIN32  
#pragma execution_character_set("utf-8")  
#endif


#include <QMainWindow>
#pragma once
#include <iostream>
#include <windows.h> //MultiByteToWideChar等
#include <tchar.h>
#include <map>

#include <Wincrypt.h>
#include <stdlib.h>
#include <Softpub.h>

#include <vector>

#include <comdef.h>
#include <taskschd.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void set_logon_table();
    void set_services_table();
    void write_item_to_table(int row_index,QString entry,QString description,QString publisher,QString imagepath);
    void write_header_to_table(int row_index,QString root_key,QString sub_key);

private slots:
    void on_logon_clicked();
    void on_Services_clicked();


private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
