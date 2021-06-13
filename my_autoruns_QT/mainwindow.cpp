#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "read_register.h"
#include "str_convert.h"
#include "check_PE_signature.h"
#include "get_PE_publisher.h"
#include "read_schedule_tasks.h"
#include "format_something.h"
//#include "read_register_service_drivers.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QDateTime>
#include <stdlib.h>

//#include <CFileFind>


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
    ui->autoruns_table->setColumnWidth(3, 130);
    ui->autoruns_table->setColumnWidth(4, 700);
    ui->autoruns_table->setColumnWidth(5, 180);

    //logon自启动项
    set_logon_table();

    //时间测试
    //LPCWSTR s = (LPCWSTR)"c:\\program files (x86)\\common files\\adobe\\adobe desktop common\\elevationmanager\\adobeupdateservice.exe";
    //char* time = get_timestamp(s);
    //qDebug()<<time;

    //set_services_table();
    //set_drivers_table();
    //set_schedule_task_table();
    //set_dlls_table();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//和set_logon_table()相近
void MainWindow::set_dlls_table()
{
    HKEY HKLM_root_key = HKEY_LOCAL_MACHINE;//HKLM
    LPCSTR sub_key = "System\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs";

    int row_index = ui->autoruns_table->rowCount();
    ui->autoruns_table->setRowCount(row_index + 1);

    map<char*, LPBYTE> map_value_data = read_value_data(HKLM_root_key, sub_key);

    //画表：注册表子键头部
    if(!map_value_data.empty())
    {
        QString root_key_str;
        root_key_str = "HKLM";
        write_header_to_table(row_index,root_key_str,sub_key);
        row_index++;
        ui->autoruns_table->setRowCount(row_index + 1);
    }

    //一行行读出每个值
    while (!map_value_data.empty())
    {
        QString entry,dll_name;
        entry = charstr_to_QString(map_value_data.begin()->first);
        dll_name = LPBYTE_to_QString(map_value_data.begin()->second);//.dll名字

        //2路径imagepath：C:\Windows\SysWOW64、C:\Windows\system32
        QString imagepath1,imagepath2;
        imagepath1 = "C:\\Windows\\SysWOW64\\"+dll_name;
        imagepath2 = "C:\\Windows\\system32\\"+dll_name;

        QString verify_result;

        std:string pub;
        get_publisher(imagepath1.toStdWString().c_str(),&verify_result,&pub);
        QString publisher_qstr;
        publisher_qstr = QString::fromLocal8Bit(QByteArray::fromRawData(pub.c_str(), pub.size()));

        if(verify_result!="") verify_result = "("+verify_result+")";

        write_item_to_table(row_index,entry,"",verify_result+publisher_qstr,imagepath1);
        row_index++;
        ui->autoruns_table->setRowCount(row_index + 1);

        get_publisher(imagepath2.toStdWString().c_str(),&verify_result,&pub);

        publisher_qstr = QString::fromLocal8Bit(QByteArray::fromRawData(pub.c_str(), pub.size()));

        if(verify_result!="") verify_result = "("+verify_result+")";

        write_item_to_table(row_index,entry,"",verify_result+publisher_qstr,imagepath2);
        row_index++;
        ui->autoruns_table->setRowCount(row_index + 1);

        //删除第一个子键的<子键名，value>
        map_value_data.erase(map_value_data.begin());
    }

}



/*
使用的API：https://docs.microsoft.com/zh-cn/windows/win32/api/taskschd/nn-taskschd-itaskfolder
使用范例：https://docs.microsoft.com/zh-cn/search/?terms=ITaskService&scope=Desktop
    为任务文件夹中的所有任务显示任务名称和状态：
        1 初始化 COM 并设置常规 COM 安全性。
        2 创建 ITaskService 对象：此对象允许连接到任务计划程序服务并访问特定的任务文件夹
        3 获取包含所需信息的任务文件夹：使用 ITaskService：： GetFolder 方法获取文件夹。
        4 获取文件夹中的任务集合：使用 ITaskFolder：： taskcontroller 方法获取任务 (IRegisteredTaskCollection) 的集合。
        5 获取集合中的任务数，并枚举集合中的每个任务：使用 IRegisteredTaskCollection 的 Item 属性 获取 IRegisteredTask 实例。 每个实例都将包含集合中的任务。 然后，可以从每个已注册任务) (属性值显示信息。
*/
void MainWindow::set_schedule_task_table()
{
    map<char*, char*> map_taskpath_imagepath;
    /*
    HRESULT hr = CoInitialize(NULL);
    if( FAILED(hr) )
    {
        printf("\nCoInitializeEx failed: %x", hr );
        return;
    }
    */

    int row_index = ui->autoruns_table->rowCount();
    ui->autoruns_table->setRowCount(row_index + 1);

    read_schedule_task_folder(&map_taskpath_imagepath);
    while (!map_taskpath_imagepath.empty())
    {
        QString entry,imagepath;
        entry = charstr_to_QString(map_taskpath_imagepath.begin()->first);
        imagepath = charstr_to_QString(map_taskpath_imagepath.begin()->second);

        //标准化可执行文件路径
        format_imagepath(&imagepath);

        QString verify_result;

        std:string pub;
        get_publisher(imagepath.toStdWString().c_str(),&verify_result,&pub);
        QString publisher_qstr;
        publisher_qstr = QString::fromLocal8Bit(QByteArray::fromRawData(pub.c_str(), pub.size()));

        if (imagepath.endsWith("exe") && verify_result!="Verified" )//验证不通过的exe在用这个函数验证一遍
        {
            bool is_or_not_verified = VerifyEmbeddedSignature(imagepath.toStdWString().c_str());
            if(is_or_not_verified) verify_result = "Verified";
            //else verify_result = "Not Verified";
        }
        if(verify_result!="") verify_result = "("+verify_result+")";

        write_item_to_table(row_index,entry,"",verify_result+publisher_qstr,imagepath);
        row_index++;
        ui->autoruns_table->setRowCount(row_index + 1);

        //删除第一个子键的<子键名，value>
        map_taskpath_imagepath.erase(map_taskpath_imagepath.begin());
    }

    //取消COM初始化
    CoUninitialize();
}

//查找logon自启动项，显示到表中
void MainWindow::set_logon_table()
{
    int row_index = ui->autoruns_table->rowCount();
    //cout<<row_index<<endl;
    ui->autoruns_table->setRowCount(row_index + 1);
    //cout << "total" << row_index << "rows" <<endl;


    /*
        查找自启动目录
    */
    char* env1,*env2;
    env1 = getenv("USERPROFILE");
    env2 = getenv("ProgramData");

    QString path1 = charstr_to_QString(env1) + "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
    QString path2= charstr_to_QString(env2) + "\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp";

    QString paths[] = {path1,path2};

    // 获取所有文件名
    for(int i=0;i<2;i++)
    {
        QDir *dir=new QDir(paths[i]);
        if (!dir->exists())
            qDebug()<<"no files";
        dir->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        QFileInfoList list = dir->entryInfoList();

        if(list.size() == 0)
        {
            qDebug()<<"no files";
            break;
        }

        //画表：注册表子键头部
        write_header_to_table(row_index,"",paths[i]);
        row_index++;
        ui->autoruns_table->setRowCount(row_index + 1);

        for(int i = 0; i < list.size(); i++)
        {
            QString entry,imagepath;
            entry = list.at(i).fileName();
            imagepath = list.at(i).filePath();

            QString verify_result;

            std:string pub;
            get_publisher(imagepath.toStdWString().c_str(),&verify_result,&pub);
            QString publisher_qstr;
            publisher_qstr = QString::fromLocal8Bit(QByteArray::fromRawData(pub.c_str(), pub.size()));

            if (imagepath.endsWith("exe") && verify_result!="Verified" )//验证不通过的exe在用这个函数验证一遍
            {
                bool is_or_not_verified = VerifyEmbeddedSignature(imagepath.toStdWString().c_str());
                if(is_or_not_verified) verify_result = "Verified";
                //else verify_result = "Not Verified";
            }

            if(verify_result!="") verify_result = "("+verify_result+")";

            write_item_to_table(row_index,entry,"",verify_result+publisher_qstr,imagepath);
            row_index++;
            ui->autoruns_table->setRowCount(row_index + 1);
        }
    }




    /*和Windows.h头文件冲突
    for(int i = 0;1<2;i++)//两个目录
    {
        CFileFind read_folder_finder;
        CString file_name,file_path;
        bool is_find_success = read_folder_finder.FindFile(paths[i].toStdString().c_str());

        //读到一个文件
        while(is_find_success)
        {
            file_name = read_folder_finder.GetFileName();//文件名
            //隐藏文件 dir /a可见
            if(file_name!="." && file_name!=".." && file_name != "desktop.ini")
            {
                file_path = read_folder_finder.GetFilePath();

                //QString file_name_qstr = CS2QS(file_name);

                write_item_to_table(row_index,file_name,"","",file_path);
                row_index++;
                ui->autoruns_table->setRowCount(row_index + 1);
            }
        }

        read_folder_finder.Close();

    }
    */






    /*
        查找注册表
    */
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
            string pub;
            while (!map_value_data.empty())
            {
                QString entry,imagepath;
                entry = charstr_to_QString(map_value_data.begin()->first);
                imagepath = LPBYTE_to_QString(map_value_data.begin()->second);

                //标准化可执行文件路径
                format_imagepath(&imagepath);
                //qDebug() << imagepath;
                //qDebug()<<entry;

                //签名验证结果 只能验证exe
                //只能用imagepath.toStdWString().c_str()，用QString_to_LPCWSTR()无效
                //参数类型：LPCWSTR
                QString verify_result;


                get_publisher(imagepath.toStdWString().c_str(),&verify_result,&pub);
                QString publisher_qstr;
                publisher_qstr = QString::fromLocal8Bit(QByteArray::fromRawData(pub.c_str(), pub.size()));

                if (imagepath.endsWith("exe") && verify_result!="Verified" )//验证不通过的exe在用这个函数验证一遍
                {
                    bool is_or_not_verified = VerifyEmbeddedSignature(imagepath.toStdWString().c_str());
                    if(is_or_not_verified) verify_result = "Verified";
                    //else verify_result = "Not Verified";
                }
                //else verify_result = "";


                //测试另一种验证 无效
                //bool re = verify(imagepath.toStdWString().c_str());

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
                if(verify_result!="") verify_result = "("+verify_result+")";

                write_item_to_table(row_index,entry,"",verify_result+publisher_qstr,imagepath);
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
    //cout<<"begin"<<endl;
    map<int, char*> map_int_subkey_name = read_subkey_name(root_key,sub_key);
    //cout<<"end"<<endl;

    int row_index = ui->autoruns_table->rowCount();
    //cout<<row_index<<endl;
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
        QString description,imagepath,type;

        //非空，遍历每个服务子键下所有value
        while(!map_service_value_data.empty())
        {
            //查描述
            if(strcmp((char*)map_service_value_data.begin()->first,value_name_Description) == 0)
            {
                LPBYTE data = (LPBYTE)map_service_value_data.begin()->second;
                description = LPBYTE_to_QString(data);
                if(description.startsWith("@"))
                {
                    format_description_path(&description);
                    //description = "@"+description;
                    //QString *content = new QString;
                    format_description(description,&description);

                }
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
            /*
                判断是否共享服务
                判断服务子键下有无 `Parameters` 子键
                若有，说明是共享服务，需要进一步获取真正的 `ImagePath` 代替在原服务子键下的值中取得的
                不由上课讲的 `Type = 32` 是共享服务判断是因为查看注册表后，发现不只有这个类型的服务的可执行文件相同。
            */
            map<int, char*> map_subkeynames_under_servicekey = read_subkey_name(root_key,service_subkey);
            for (map<int, char*>::iterator it = map_subkeynames_under_servicekey.begin(); it != map_subkeynames_under_servicekey.end(); ++it)
            {
                //子键名char*,可直接转string
                string subkey_name,new_subkey,service_subkey_str;
                subkey_name = it->second;
                //LPCTSTR=LPCSTR=char*
                service_subkey_str = service_subkey;
                //是共享服务 更新路径和签名
                if(subkey_name == "Parameters")
                {
                    new_subkey = service_subkey_str + "\\" + subkey_name;
                    LPCTSTR para_subkey;
                    para_subkey = new_subkey.c_str();

                    //parameters子键下所有值-数据
                    map<char*,LPBYTE> map_para_value_data;
                    map_para_value_data = read_value_data(root_key,para_subkey);

                    const char * value_name_ServiceDll = "ServiceDll";
                    while(!map_para_value_data.empty())
                    {
                        //查ServiceDll值，更新路径
                        if(strcmp((char*)map_para_value_data.begin()->first,value_name_ServiceDll) == 0)
                        {
                            LPBYTE data = (LPBYTE)map_para_value_data.begin()->second;
                            imagepath = LPBYTE_to_QString(data);
                            //标准化可执行文件路径
                            format_imagepath(&imagepath);
                            break;
                        }
                        map_para_value_data.erase(map_para_value_data.begin());//删除map第一项

                    }
                    break;
                }

            }

            //签名验证结果
            QString verify_result;

            std:string pub;
            get_publisher(imagepath.toStdWString().c_str(),&verify_result,&pub);
            QString publisher_qstr;
            publisher_qstr = QString::fromLocal8Bit(QByteArray::fromRawData(pub.c_str(), pub.size()));

            if (imagepath.endsWith("exe") && verify_result!="Verified" )//验证不通过的exe在用这个函数验证一遍
            {
                bool is_or_not_verified = VerifyEmbeddedSignature(imagepath.toStdWString().c_str());
                if(is_or_not_verified) verify_result = "Verified";
                //else verify_result = "Not Verified";
            }

            QString entry = QString::fromStdString(service_subkey_name);

            if(verify_result!="") verify_result = "("+verify_result+")";

            write_item_to_table(row_index,entry,"",verify_result+publisher_qstr,imagepath);
            row_index++;
            ui->autoruns_table->setRowCount(row_index + 1);

        }
    }

}

//和services几乎一样，但无共享服务
void MainWindow::set_drivers_table()
{
    HKEY root_key = HKEY_LOCAL_MACHINE;
    LPCSTR sub_key = "SYSTEM\\CurrentControlSet\\Services";
    //cout<<"begin"<<endl;
    map<int, char*> map_int_subkey_name = read_subkey_name(root_key,sub_key);
    //cout<<"end"<<endl;

    int row_index = ui->autoruns_table->rowCount();
    //cout<<row_index<<endl;
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
                if(description.startsWith("@"))
                {
                    format_description_path(&description);
                    format_description(description,&description);
                }
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

        if(type!= "" && type < 16) //内核态驱动
        {

            /*
                判断是否共享服务
                判断服务子键下有无 `Parameters` 子键
                若有，说明是共享服务，需要进一步获取真正的 `ImagePath` 代替在原服务子键下的值中取得的
                不由上课讲的 `Type = 32` 是共享服务判断是因为查看注册表后，发现不只有这个类型的服务的可执行文件相同。
            */
            /*  注释掉，内核态无共享服务
            map<int, char*> map_subkeynames_under_servicekey = read_subkey_name(root_key,service_subkey);
            for (map<int, char*>::iterator it = map_subkeynames_under_servicekey.begin(); it != map_subkeynames_under_servicekey.end(); ++it)
            {
                //子键名char*,可直接转string
                string subkey_name,new_subkey,service_subkey_str;
                subkey_name = it->second;
                //LPCTSTR=LPCSTR=char*
                service_subkey_str = service_subkey;
                //是共享服务 更新路径和签名
                if(subkey_name == "Parameters")
                {
                    new_subkey = service_subkey_str + "\\" + subkey_name;
                    LPCTSTR para_subkey;
                    para_subkey = new_subkey.c_str();

                    //parameters子键下所有值-数据
                    map<char*,LPBYTE> map_para_value_data;
                    map_para_value_data = read_value_data(root_key,para_subkey);

                    const char * value_name_ServiceDll = "ServiceDll";
                    while(!map_para_value_data.empty())
                    {
                        //查ServiceDll值，更新路径
                        if(strcmp((char*)map_para_value_data.begin()->first,value_name_ServiceDll) == 0)
                        {
                            LPBYTE data = (LPBYTE)map_para_value_data.begin()->second;
                            imagepath = LPBYTE_to_QString(data);
                            //标准化可执行文件路径
                            format_imagepath(&imagepath);
                            break;
                        }
                        map_para_value_data.erase(map_para_value_data.begin());//删除map第一项

                    }
                    break;
                }

            }
            */

            //签名验证结果
            std:string pub;
            get_publisher(imagepath.toStdWString().c_str(),&verify_result,&pub);
            QString publisher_qstr;
            publisher_qstr = QString::fromLocal8Bit(QByteArray::fromRawData(pub.c_str(), pub.size()));

            if (imagepath.endsWith("exe") && verify_result!="Verified" )//验证不通过的exe在用这个函数验证一遍
            {
                bool is_or_not_verified = VerifyEmbeddedSignature(imagepath.toStdWString().c_str());
                if(is_or_not_verified) verify_result = "Verified";
                //else verify_result = "Not Verified";
            }

            QString entry = QString::fromStdString(service_subkey_name);

            if(verify_result!="") verify_result = "("+verify_result+")";

            write_item_to_table(row_index,entry,"",verify_result+publisher_qstr,imagepath);
            row_index++;
            ui->autoruns_table->setRowCount(row_index + 1);

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

    //文件创建时间
    QFileInfo file_info(imagepath);
    QDateTime file_create_datetime = file_info.created();
    QString file_create_time = QObject::tr("%1").arg(file_create_datetime.toString("yyyy-MM-dd hh:mm:ss"));
    QTableWidgetItem* timestamp_item = new QTableWidgetItem(file_create_time);
    ui->autoruns_table->setItem(row_index, 5, timestamp_item);

    //图标
    QFileIconProvider file_icon;
    QIcon icon = file_icon.icon(file_info);
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

void MainWindow::on_services_clicked()
{
    ui->autoruns_table->clearContents();
    ui->autoruns_table->setRowCount(0);
    set_services_table();
}

void MainWindow::on_drivers_clicked()
{
    ui->autoruns_table->clearContents();
    ui->autoruns_table->setRowCount(0);
    set_drivers_table();
}

void MainWindow::on_schedule_task_clicked()
{
    ui->autoruns_table->clearContents();
    ui->autoruns_table->setRowCount(0);
    set_schedule_task_table();
}

void MainWindow::on_dlls_clicked()
{
    ui->autoruns_table->clearContents();
    ui->autoruns_table->setRowCount(0);
    set_dlls_table();
}
