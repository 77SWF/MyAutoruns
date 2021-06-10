#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "read_register.h"
#include "str_convert.h"
#include "check_PE_signature.h"
#include "get_PE_publisher.h"
//#include "read_register_service_drivers.h"

#include <QDebug>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QDateTime>
#include <stdlib.h>

#include <QString>

#pragma once
#include <iostream>
#include <windows.h> //MultiByteToWideChar等
#include <tchar.h>
#include <map>

#include <stdio.h>
#include <tchar.h>
#include <shlwapi.h>

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
    //set_logon_table();

    set_services_table();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//查找logon自启动项，显示到表中
void MainWindow::set_logon_table()
{
    //cout<<"here"<<endl;
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
    cout<<row_index<<endl;
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

                //标准化可执行文件路径
                format_imagepath(&imagepath);
                //qDebug() << imagepath;
                //qDebug()<<entry;

                //签名验证结果
                //只能用imagepath.toStdWString().c_str()，用QString_to_LPCWSTR()无效
                //参数类型：LPCWSTR
                bool is_or_not_verified = VerifyEmbeddedSignature(imagepath.toStdWString().c_str());
                QString verify_result;
                if(is_or_not_verified) verify_result = "Verified";
                else verify_result = "Not Verified";

                //获得publisher

                /*失败
                QString publisher;
                LPTSTR* pt_publisher = NULL;
                //QString *buf = new QString;


                //LPTSTR* ptStr = NULL;
                char a[] = "C:\\Program Files\\Kite\\kited.exe";
                char *b = a;
                bool is_get_publisher = get_PE_publisher(b,pt_publisher);
                //bool is_get_publisher = get_PE_publisher(CharToWchar(imagepath),pt_publisher);
                //if(is_get_publisher) publisher = *pt_publisher;//非Unicode直接 赋值
                //else publisher = "Unknown";
                */

                //write_item_to_table(row_index,entry,"",verify_result+publisher,imagepath);
                write_item_to_table(row_index,entry,"",verify_result,imagepath);
                row_index++;
                ui->autoruns_table->setRowCount(row_index + 1);

                //删除第一个子键的<子键名，value>
                map_value_data.erase(map_value_data.begin());
            }
        }
    }

}

void MainWindow::set_services_table()
{
    HKEY root_key = HKEY_LOCAL_MACHINE;
    LPCSTR sub_key = "SYSTEM\\CurrentControlSet\\Services";
    cout<<"begin"<<endl;
    map<int, char*> map_int_subkey_name = read_subkey_name(root_key,sub_key);
    cout<<"end"<<endl;

    int row_index = ui->autoruns_table->rowCount();
    cout<<row_index<<endl;
    ui->autoruns_table->setRowCount(row_index + 1);

    //画表：注册表子键头部
    QString root_key_str;
    root_key_str = "HKLM";
    write_header_to_table(row_index,root_key_str,sub_key);
    row_index++;
    ui->autoruns_table->setRowCount(row_index + 1);

    //遍历所有服务子键：map_int_subkey_name
    string service_subkey_name,new_service_subkey;
    for (map<int, char*>::iterator it = map_int_subkey_name.begin(); it != map_int_subkey_name.end(); ++it)
    {
        //子键名，char*可直接转string
        service_subkey_name = it->second;
        //完整子键,每个service_subkey对应一个服务
        new_service_subkey = "SYSTEM\\CurrentControlSet\\Services\\" + service_subkey_name;
        LPCTSTR service_subkey;//new_service_subkey类型转换
        //错误service_subkey = ("SYSTEM\\CurrentControlSet\\Services\\" + service_subkey_name).c_str();
        //测试成功
        //service_subkey= "SYSTEM\\CurrentControlSet\\Services\\acpitime";
        service_subkey = new_service_subkey.c_str();//成功

        //每个服务子键下的所有值：map<值名,数据>
        map<char*,LPBYTE> map_service_value_data;
        map_service_value_data = read_value_data(root_key,service_subkey);

        /*
            Type=16/32（>=16）   ：是用户态服务；
            Type=1/2/4/8(<16)：内核态驱动服务
            若start的数据为2，表示该服务被设置成自启动,但不作为过滤条件
        */
        const char * value_name_Type = "Type";
        const char * value_name_Description = "Description";
        const char * value_name_ImagePath = "ImagePath";
        const char * value_name_Start = "Start";
        QString description,imagepath,type,verify_result;

        //非空，遍历每个服务子键下所有value
        while(!map_service_value_data.empty())
        {
            //查描述
            if(strcmp((char*)map_service_value_data.begin()->first,value_name_Description) == 0)
            {
                LPBYTE data = (LPBYTE)map_service_value_data.begin()->second;
                description = LPBYTE_to_QString(data);
            }
            
            //查路径
            if(strcmp((char*)map_service_value_data.begin()->first,value_name_ImagePath) == 0)
            {
                LPBYTE data = (LPBYTE)map_service_value_data.begin()->second;
                imagepath = LPBYTE_to_QString(data);
                //标准化可执行文件路径
                format_imagepath(&imagepath);
            }

            //查Type
            if(strcmp((char*)map_service_value_data.begin()->first,value_name_Type) == 0)
            {

                //LPBYTE->QString,可与int比较大小
                LPBYTE type_lpbyte = (LPBYTE)map_service_value_data.begin()->second;
                type = LPBYTE_to_QString(type_lpbyte);
                //string type_str = type_qstr.toStdString(); // \001形式
                //int type_int = stoi(type_str); 异常
                //type_str = "0"+type_str.substr(1, type_str.length() -1);// 0001形式
                //const char * type_char = type_str.data();
            }
            map_service_value_data.erase(map_service_value_data.begin());//删除map第一项
        }

        if(type >= 16) //服务
        {
            //签名验证结果
            bool is_or_not_verified = VerifyEmbeddedSignature(imagepath.toStdWString().c_str());
            if(is_or_not_verified) verify_result = "Verified";
            else verify_result = "Not Verified";

            QString entry = QString::fromStdString(service_subkey_name);

            write_item_to_table(row_index,entry,description,verify_result,imagepath);
            row_index++;
            ui->autoruns_table->setRowCount(row_index + 1);

        }

        /*
        if (type >= 16)//用==有错
        {
            //cout<<"16/32"<<endl;
            LPBYTE description, imagepath;

            QString description_qstr;
            description = read_service_subkey_description(root_key,service_subkey);
            description_qstr = LPBYTE_to_QString(description);
            //dll
            if(description_qstr.startsWith("@"))
            {
                bool dllDesSuc = 0;
                PWSTR description_PWSTR;
                if(S_OK == SHLoadIndirectString(description_qstr.toStdWString().c_str(),description_PWSTR,1024, NULL))
                    dllDesSuc = 1;
                else
                    dllDesSuc = 0;
                cout << *description_PWSTR;
                //description_qstr = (dllDesSuc) ? QString::fromLocal8Bit(description_PWSTR) : "----------";
            }

            QString imagepath_qstr;
            imagepath = read_service_subkey_imagepath(root_key, service_subkey);
            imagepath_qstr = LPBYTE_to_QString((LPBYTE)map_int_subkey_name.begin()->second);
            //标准化可执行文件路径
            format_imagepath(&imagepath_qstr);

            //签名验证结果
            //只能用imagepath.toStdWString().c_str()，用QString_to_LPCWSTR()无效
            //参数类型：LPCWSTR
            bool is_or_not_verified = VerifyEmbeddedSignature(imagepath_qstr.toStdWString().c_str());
            QString verify_result;
            if(is_or_not_verified) verify_result = "Verified";
            else verify_result = "Not Verified";



            QString entry = QString::fromStdString(service_subkey_name);
            qDebug()<<entry;

            //cout<<"in row "<< row_index;
            write_item_to_table(row_index,entry,"",verify_result,imagepath_qstr);
            row_index++;
            ui->autoruns_table->setRowCount(row_index + 1);
            cout<<"end"<<endl;
        }
        */
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
    ui->autoruns_table->clearContents();
    ui->autoruns_table->setRowCount(0);
    set_logon_table();
}

void MainWindow::on_Services_clicked()
{
    ui->autoruns_table->clearContents();
    ui->autoruns_table->setRowCount(0);
    set_services_table();
}
