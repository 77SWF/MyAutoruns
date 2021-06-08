#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "read_register.h"
#include "str_convert.h"

#include <QDebug>
#include <QFileInfo>
#include <QDateTime>

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


#pragma comment(lib, "version.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Wintrust.lib")
#pragma comment(lib, "taskschd.lib")

using namespace std;

// 入口函数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //初始化表列宽:entry descryption publisher timestamp
    ui->autoruns_table->setColumnWidth(0, 20);
    ui->autoruns_table->setColumnWidth(1, 180);
    ui->autoruns_table->setColumnWidth(2, 180);
    ui->autoruns_table->setColumnWidth(3, 180);
    ui->autoruns_table->setColumnWidth(4, 300);

    //logon自启动项
    set_logon_table();

}

MainWindow::~MainWindow()
{
    delete ui;
}

//查找logon自启动项，显示到表中
void MainWindow::set_logon_table()
{
    //print("这里");
    HKEY HKLM_root_key = HKEY_LOCAL_MACHINE;//HKLM
    HKEY HKCU_root_key = HKEY_CURRENT_USER;//HKCU，两个主键下的以下子键都有

    LPCSTR sub_key1 = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCSTR sub_key2 = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
    LPCSTR sub_key3 = "SOFTWARE\\Microsoft\\Active Setup\\Installed Components";
    LPCSTR sub_key4 = "Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCSTR sub_key5 = "SOFTWARE\\Microsoft\\Active Setup\\Installed Components";
    LPCSTR sub_key6 = "SOFTWARE\\Wow6432Node\\Microsoft\\Active Setup\\Installed Components";
    LPCSTR sub_key7 = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx";
    /*
    //7个子键
    LPCSTR sub_keys = {sub_key1,sub_key2,sub_key3，sub_key4，sub_key5，sub_key6，sub_key7};

    for(int i = 0; i < 7; i++)
    {

    }
    */

    //参数：主键，主键下子键
    //map_value_data：子键名，value
    map<char*, LPBYTE> map_value_data = read_value_data(HKLM_root_key, sub_key1);

    //如果map非空，一行行读出每个值
    while (!map_value_data.empty())
    {
        cout << map_value_data.begin()->first << '\t' << map_value_data.begin()->second << endl;

        QString value_name;
        value_name = charstr_to_QString(map_value_data.begin()->first);
        qDebug() << "aaaaaaaaaaaaaaaaaa";
        qDebug() << value_name;
        qDebug() << "aaaaaaaaaaaaaaaaaa";

        map_value_data.erase(map_value_data.begin());//删除第一个子键的<子键名，value>

    }

    //读主键1的路径2
    map_value_data = read_value_data(HKLM_root_key, sub_key2);

    while (!map_value_data.empty())
    {
        cout << map_value_data.begin()->first << '\t' << map_value_data.begin()->second << endl;

        QString value_name;
        value_name = charstr_to_QString(map_value_data.begin()->first);
        qDebug() << "aaaaaaaaaaaaaaaaaa";
        qDebug() << value_name;
        qDebug() << "aaaaaaaaaaaaaaaaaa";
        map_value_data.erase(map_value_data.begin());

        
    }
    //主键2的路径1
    map_value_data = read_value_data(HKCU_root_key, sub_key1);
    while (!map_value_data.empty())
    {
        cout << map_value_data.begin()->first << '\t' << map_value_data.begin()->second << endl;

        QString value_name;
        value_name = charstr_to_QString(map_value_data.begin()->first);
        qDebug() << "aaaaaaaaaaaaaaaaaa";
        qDebug() << value_name;
        qDebug() << "aaaaaaaaaaaaaaaaaa";

        map_value_data.erase(map_value_data.begin());

    }
    //主键2的路径2
    map_value_data = read_value_data(HKCU_root_key, sub_key2);
    while (!map_value_data.empty())
    {
        cout << map_value_data.begin()->first << '\t' << map_value_data.begin()->second << endl;

        QString value_name;
        value_name = charstr_to_QString(map_value_data.begin()->first);
        qDebug() << "aaaaaaaaaaaaaaaaaa";
        qDebug() << value_name;
        qDebug() << "aaaaaaaaaaaaaaaaaa";

        map_value_data.erase(map_value_data.begin());

    }

}

//行号:自启动项名，描述，签名者，可执行文件路径
void MainWindow::write_item_to_table(int row_index,QString entry,QString description,QString publisher,QString imagepath)
{
    ui->autoruns_table->setRowHeight(row_index, 30);//行高

    QTableWidgetItem* entry_item = new QTableWidgetItem(entry);
    ui->autoruns_table->setItem(row_index, 1, entry_item);

    QTableWidgetItem* description_item = new QTableWidgetItem(description);
    ui->autoruns_table->setItem(row_index, 4, description_item);

    QTableWidgetItem* publisher_item = new QTableWidgetItem(publisher);
    ui->autoruns_table->setItem(row_index, 4, publisher_item);

    QTableWidgetItem* imagepath_item = new QTableWidgetItem(imagepath);
    ui->autoruns_table->setItem(row_index, 4, imagepath_item);

    //timestamp
    QFileInfo fileInfo(imagepath);
    QDateTime file_create_datetime = fileInfo.created();
    QString file_create_time = QObject::tr("%1").arg(file_create_datetime.toString("yyyy-MM-dd hh:mm:ss"));
    QTableWidgetItem* timestamp_item = new QTableWidgetItem(file_create_time);
    ui->autoruns_table->setItem(row_index, 5, timestamp_item);
    return;
}


void MainWindow::on_logon_clicked()
{

}
