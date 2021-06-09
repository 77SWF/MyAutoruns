#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "read_register.h"
#include "str_convert.h"

#include <QDebug>
#include <QFileInfo>
#include <QFileIconProvider>
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
    //初始化表列宽:entry descryption publisher imagepath timestamp
    ui->autoruns_table->setColumnWidth(0, 10);
    ui->autoruns_table->setColumnWidth(1, 180);
    ui->autoruns_table->setColumnWidth(2, 180);
    ui->autoruns_table->setColumnWidth(3, 180);
    ui->autoruns_table->setColumnWidth(4, 600);
    ui->autoruns_table->setColumnWidth(5, 180);

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
    HKEY root_keys[] = {HKLM_root_key,HKCU_root_key};

    LPCSTR sub_key1 = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCSTR sub_key2 = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
    LPCSTR sub_key3 = "SOFTWARE\\Microsoft\\Active Setup\\Installed Components";
    LPCSTR sub_key4 = "Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCSTR sub_key5 = "SOFTWARE\\Microsoft\\Active Setup\\Installed Components";
    LPCSTR sub_key6 = "SOFTWARE\\Wow6432Node\\Microsoft\\Active Setup\\Installed Components";
    LPCSTR sub_key7 = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx";
    LPCSTR sub_key8 = "SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\AlternateShell";
    //所有子键
    LPCSTR sub_keys[] = {sub_key1,sub_key2,sub_key3,sub_key4,sub_key5,sub_key6,sub_key7,sub_key8};

    int row_index = ui->autoruns_table->rowCount();
    ui->autoruns_table->setRowCount(row_index + 1);
    //cout << "total" << row_index << "rows" <<endl;

    for(int j = 0;j<2;j++)
    {
        for(int i = 0; i < 8; i++)
        {
            //cout << i << " times" <<endl;
            //map_value_data：子键名，value
            map<char*, LPBYTE> map_value_data = read_value_data(root_keys[j], sub_keys[i]);

            //画表：注册表子键头部
            if(!map_value_data.empty())
            {
                QString root_key_str;
                if(j==1) root_key_str = "HKLM";
                else root_key_str = "HKCU";
                write_header_to_table(row_index,root_key_str,sub_keys[i]);
                row_index++;
                ui->autoruns_table->setRowCount(row_index + 1);
            }


            //如果map非空，一行行读出每个值的信息
            while (!map_value_data.empty())
            {
                QString entry,imagepath;
                entry = charstr_to_QString(map_value_data.begin()->first);
                imagepath = LPBYTE_to_QString(map_value_data.begin()->second);

                format_imagepath(&imagepath);//标准化路径
                //qDebug() << imagepath;
                //qDebug()<<entry;

                write_item_to_table(row_index,entry,"","",imagepath);
                row_index++;
                ui->autoruns_table->setRowCount(row_index + 1);

                map_value_data.erase(map_value_data.begin());//删除第一个子键的<子键名，value>
            }
        }
    }

}

// 行号:自启动项名，描述，签名者，可执行文件路径
void MainWindow::write_item_to_table(int row_index,QString entry,QString description,QString publisher,QString imagepath)
{
    ui->autoruns_table->setRowHeight(row_index, 30);//行高

    //QString a = "test_entry";
    QTableWidgetItem* entry_item = new QTableWidgetItem(entry);
    ui->autoruns_table->setItem(row_index, 1, entry_item);

    QTableWidgetItem* description_item = new QTableWidgetItem(description);
    ui->autoruns_table->setItem(row_index, 2, description_item);

    QTableWidgetItem* publisher_item = new QTableWidgetItem(publisher);
    ui->autoruns_table->setItem(row_index, 3, publisher_item);

    QTableWidgetItem* imagepath_item = new QTableWidgetItem(imagepath);
    ui->autoruns_table->setItem(row_index, 4, imagepath_item);

    //timestamp
    QFileInfo fileInfo(imagepath);
    QDateTime file_create_datetime = fileInfo.created();
    QString file_create_time = QObject::tr("%1").arg(file_create_datetime.toString("yyyy-MM-dd hh:mm:ss"));
    QTableWidgetItem* timestamp_item = new QTableWidgetItem(file_create_time);
    ui->autoruns_table->setItem(row_index, 5, timestamp_item);

    //图标
    QFileIconProvider fileIcon;
    QIcon icon = fileIcon.icon(fileInfo);
    QTableWidgetItem* icon_item = new QTableWidgetItem;
    icon_item->setIcon(icon);
    ui->autoruns_table->setItem(row_index, 0, icon_item);
    return;
}

void MainWindow::write_header_to_table(int row_index,QString root_key,QString sub_key)
{
    QString header_str = QString(" %1 %2 %3").arg(root_key).arg("\\").arg(sub_key);
    QTableWidgetItem* header_item = new QTableWidgetItem(header_str);
    header_item->setBackgroundColor(QColor(200, 200, 255));
    ui->autoruns_table->setItem(row_index, 0, header_item);
    ui->autoruns_table->setSpan(row_index, 0, 1, 6);
}

void MainWindow::on_logon_clicked()
{
}

void MainWindow::on_Services_clicked()
{
}
