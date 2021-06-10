#ifndef READ_REGISTER_H
#define READ_REGISTER_H

#pragma once
#include <iostream>
#include <windows.h> //MultiByteToWideChar等
#include <tchar.h>
#include <map>
#include <winreg.h>
#include <Wincrypt.h>
#include <stdlib.h>
#include <Softpub.h>

#include <vector>

#include <comdef.h>
#include <taskschd.h>
#include <winreg.h>

#pragma comment(lib, "version.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Wintrust.lib")
#pragma comment(lib, "taskschd.lib")

using namespace std;

//msdn reg函数范例：https://docs.microsoft.com/zh-cn/windows/win32/api/winreg/nf-winreg-regopenkeyexa
//读某键下的所有value，返回map<值名，数据>
map<char*, LPBYTE> read_value_data(HKEY root_key, LPCUTSTR sub_key)
{
    HKEY key_handle;

    map<char*, LPBYTE> map_value_data;

    if (ERROR_SUCCESS == RegOpenKeyEx(root_key, sub_key, 0, KEY_READ|KEY_WOW64_64KEY, &key_handle))
    {
        DWORD dwIndex = 0, NameSize, NameCnt, NameMaxLen, Type;
        DWORD KeySize, KeyCnt, KeyMaxLen, DateSize, MaxDateLen;
        if (ERROR_SUCCESS == RegQueryInfoKey(key_handle, NULL, NULL, 0, &KeyCnt, &KeyMaxLen, NULL, &NameCnt, &NameMaxLen, &MaxDateLen, NULL, NULL))
        {
            //成功查到这个自启动的键，遍历下面所有value，用NameCNt
            for (DWORD dwIndex = 0; dwIndex < NameCnt; dwIndex++)
            {
                DateSize = MaxDateLen + 1;//数据最大长度
                NameSize = NameMaxLen + 1;//值名最大长度
                //值名，数据
                char* value_name = (char*)malloc(NameSize);
                LPBYTE value_data = (LPBYTE)malloc(DateSize);//LPBYTE也是char *

                //读到的东西(打开句柄，找第i个下面的子键)
                //取值名字、长度、类型、数据和数据长度存到变量里
                RegEnumValue(key_handle, dwIndex, value_name, &NameSize, NULL, &Type, value_data, &DateSize);

                //值名：数据
                map_value_data[value_name] = value_data;

            }

        }
        else
        {
            cout << "RegQueryInfoKey fail！" << endl;
        }
    }
    else {
        cout << "RegOpenKeyEx fail!" << endl;
    }

    return map_value_data;
}

//读子键下所有子键，返回map<数字, char*子键名>
map<int, char*> read_subkey_name(HKEY root_key, LPCTSTR sub_key)
{
    //打开
    HKEY key_handle;
    map<int, char*> subitem;//0:子键名 1:子键名？存这个主键/可能自启动路径下的所有子键名？
    int len = 0;
    cout<< "begin read"<<endl;

    //如果读主键root_key下的子键sub_key打开成功
    if (ERROR_SUCCESS == RegOpenKeyEx(
        root_key, 
        sub_key, 
        0, 
        KEY_READ, 
        &key_handle))
    {
        cout<<"read reg"<<endl;
        DWORD dwIndex = 0, NameSize, NameCnt, NameMaxLen, Type;
        DWORD KeySize, KeyCnt, KeyMaxLen, DateSize, MaxDateLen;

        //如果成功查询打开的子键，下的数量信息
        if (ERROR_SUCCESS == RegQueryInfoKey(
            key_handle, 
            NULL, 
            NULL, 
            0, 
            &KeyCnt, 
            &KeyMaxLen, 
            NULL, 
            &NameCnt, 
            &NameMaxLen, 
            &MaxDateLen, 
            NULL, 
            NULL)) 
        {
            //遍历子键下所有子键
            for (DWORD dwIndex = 0; dwIndex < KeyCnt; dwIndex++)
            {
                KeySize = KeyMaxLen + 1;//子键名称最大长度+1
                char* szKeyName = (char*)malloc(KeySize);//子键名字符串 开一个位置

                //打开的那6个注册表之一的句柄，句柄对应的是这个键下的一列子键，第dwIndex个子键(0开始)
                //szKeyName存第dwIndex个子键的名字，子键名长度KeySize 为什么加&？
                RegEnumKeyEx(
                    key_handle, 
                    dwIndex, 
                    szKeyName, 
                    &KeySize, 
                    NULL, 
                    NULL, 
                    NULL, 
                    NULL);

                subitem[len] = szKeyName;
                len++;
            }
        }
        //读不到子键
        else {
            cout << "读取子键失败！" << endl;
        }
    }
    else {
        cout << "打开注册表失败！" << endl;
    }
    RegCloseKey(key_handle);//关闭句柄

    return subitem;
}


//refer
void format_imagepath(QString* value)
{
    /*
        注册表中的value可能会带有命令参数、系统变量等，该函数将其标准化为一个文件路径（QString）
        */
    if (value->indexOf("@") == 0)
    {
        *value = value->split("@")[1];
        *value = value->split(",-")[0];
    }
    if (value->indexOf(" -") >= 0)    //部分ImagePath后跟有 -[command],需要剔除
        *value = value->split(" -")[0];
    if (value->indexOf(" /") >= 0)    //部分ImagePath后跟有 /[command],需要剔除
        *value = value->split(" /")[0];
    if(value->indexOf("\"") >= 0)     //部分ImagePath已被""包括
        *value = value->split("\"")[1];
    if(value->contains("%systemroot%", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("%systemroot%", 0, Qt::CaseInsensitive), 12, "C:\\Windows");
    if(value->contains("\\systemroot", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("\\systemroot", 0, Qt::CaseInsensitive), 11, "C:\\Windows");
    if(value->contains("%windir%", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("%windir%", 0, Qt::CaseInsensitive), 8, "C:\\Windows");
    if(value->contains("\\??\\", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("\\??\\"), 4, "");
    if(!value->contains(":"))
    {
        if(value->contains("system32", Qt::CaseInsensitive))
            *value = value->replace(value->indexOf("system32", 0, Qt::CaseInsensitive), 8, "c:\\Windows\\System32");
        if(value->contains("syswow64", Qt::CaseInsensitive))
            *value = value->replace(value->indexOf("syswow64", 0, Qt::CaseInsensitive), 8, "c:\\Windows\\SysWOW64");
    }
}




#endif // READ_REGISTER_H
