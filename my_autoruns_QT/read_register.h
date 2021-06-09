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
map<char*, LPBYTE> read_value_data(HKEY root_key, LPCUTSTR sub_key)
{
    HKEY key_handle;

    map<char*, LPBYTE> map_value_data;

    REGSAM samDesired;

    //if(root_key== HKEY_LOCAL_MACHINE && sub_key =="Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run" ) samDseired = KEY_WOW64_64KEY;
    //else 

    if (ERROR_SUCCESS == RegOpenKeyEx(root_key, sub_key, 0, KEY_READ|KEY_WOW64_64KEY, &key_handle)) {
        DWORD dwIndex = 0, NameSize, NameCnt, NameMaxLen, Type;
        DWORD KeySize, KeyCnt, KeyMaxLen, DateSize, MaxDateLen;
        if (ERROR_SUCCESS == RegQueryInfoKey(key_handle, NULL, NULL, 0, &KeyCnt, &KeyMaxLen, NULL, &NameCnt, &NameMaxLen, &MaxDateLen, NULL, NULL)) {
            //成功查到这个自启动的键，遍历下面所有子键，用NameCNt
            for (DWORD dwIndex = 0; dwIndex < NameCnt; dwIndex++)
            {
                DateSize = MaxDateLen + 1;//子键的值最大长度
                NameSize = NameMaxLen + 1;//子键名字最大长度
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
        else {
            cout << "读取子键失败！" << endl;
        }
    }
    else {
        cout << "打开注册表失败！" << endl;
    }

    return map_value_data;
}

//msdn https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryvalueexa
LPBYTE read_description(HKEY root_key, LPCSTR sub_key)
{
	HKEY key_handle;
	DWORD lpType = 0;
	LPBYTE lpData = NULL;
	DWORD lpcbData = 0;

	long dwRet;
	dwRet = RegOpenKeyEx(root_key, 
                        sub_key, 
                        0, 
                        KEY_QUERY_VALUE, 
                        &key_handle);
                        
	if (dwRet == ERROR_SUCCESS)
	{
        RegQueryValueEx(key_handle, (LPCSTR)("Description"), 0, &lpType, lpData, &lpcbData);
		lpData = (LPBYTE)malloc(lpcbData);
        dwRet = RegQueryValueEx(key_handle, (LPCSTR)("Description"), 0, &lpType, lpData, &lpcbData);
		RegCloseKey(key_handle);
	}

	//cout << lpData << endl;
	return lpData;
}


//参考github
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