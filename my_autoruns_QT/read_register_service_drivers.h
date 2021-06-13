#ifndef READ_REGISTER_SERVICE_DRIVERS_H
#define READ_REGISTER_SERVICE_DRIVERS_H

/*
    服务相关读取函数
    */

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

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

struct KEY_VALUE
{
    TCHAR key[MAX_VALUE_NAME];
    BYTE value[MAX_VALUE_NAME];
    int length = 0;
};

//tchar BYTE
//传入句柄
KEY_VALUE *QueryKey(HKEY hKey)
{
    TCHAR achKey[MAX_KEY_LENGTH];        // buffer for subkey name
    DWORD cbName;                        // size of name string
    TCHAR achClass[MAX_PATH] = TEXT(""); // buffer for class name
    DWORD cchClassName = MAX_PATH;       // size of class string
    DWORD cSubKeys = 0;                  // number of subkeys
    DWORD cbMaxSubKey;                   // longest subkey size
    DWORD cchMaxClass;                   // longest class string
    DWORD cValues;                       // number of values for key
    DWORD cchMaxValue;                   // longest value name
    DWORD cbMaxValueData;                // longest value data
    DWORD cbSecurityDescriptor;          // size of security descriptor
    FILETIME ftLastWriteTime;            // last write time

    DWORD i, retCode;

    TCHAR achValue[MAX_VALUE_NAME];
    BYTE achData[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD cchData = MAX_VALUE_NAME;
    DWORD Type;

    // Get the class name and the value count.
    retCode = RegQueryInfoKey(
        hKey,                  // key handle
        achClass,              // buffer for class name
        &cchClassName,         // size of class string
        NULL,                  // reserved
        &cSubKeys,             // number of subkeys
        &cbMaxSubKey,          // longest subkey size
        &cchMaxClass,          // longest class string
        &cValues,              // number of values for this key
        &cchMaxValue,          // longest value name
        &cbMaxValueData,       // longest value data
        &cbSecurityDescriptor, // security descriptor
        &ftLastWriteTime);     // last write time

    // Enumerate the subkeys, until RegEnumKeyEx fails.

    // Enumerate the key values.
    KEY_VALUE *list = new KEY_VALUE [cValues + 1];
    list[0].length = 0;
    if (cValues)
    {
        //列表第一个用于存放该list的值的个数
        for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
        {
            cchData = MAX_VALUE_NAME;
            cchValue = MAX_VALUE_NAME;
            //填充
            list[i].key[0] = '\0';
            list[i].value[0] = '\0';
            retCode = RegEnumValue(hKey, i, list[i].key, &cchValue, NULL, &Type, list[i].value, &cchData);
            
            if (retCode == ERROR_SUCCESS)
            {
                list[0].length += 1;
            }
        }
    }
    return list;
}

//msdn https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryvalueexa
LPBYTE read_service_subkey_description(HKEY root_key, LPCSTR sub_key)
{
    HKEY key_handle;
    DWORD lpType = 0;
    LPBYTE lpData = NULL;
    DWORD lpcbData = 0;

    if (ERROR_SUCCESS == RegOpenKeyEx(root_key,
                        sub_key,
                        0,
                        KEY_QUERY_VALUE,
                        &key_handle))
    {
        RegQueryValueEx(key_handle, (LPCSTR)("Description"), 0, &lpType, lpData, &lpcbData);

        lpData = (LPBYTE)malloc(lpcbData);

        RegQueryValueEx(key_handle, (LPCSTR)("Description"), 0, &lpType, lpData, &lpcbData);
        RegCloseKey(key_handle);
    }

    return lpData;
}

LPBYTE read_service_subkey_imagepath(HKEY root_key, LPCSTR sub_key)
{
    HKEY key_handle;
    DWORD lpType = 0;
    LPBYTE lpData = NULL;
    DWORD lpcbData = 0;

    if (ERROR_SUCCESS == RegOpenKeyEx(root_key,
                        sub_key,
                        0,
                        KEY_QUERY_VALUE,
                        &key_handle))
    {
        RegQueryValueEx(key_handle, (LPCSTR)("ImagePath"), 0, &lpType, lpData, &lpcbData);

        lpData = (LPBYTE)malloc(lpcbData);

        RegQueryValueEx(key_handle, (LPCSTR)("ImagePath"), 0, &lpType, lpData, &lpcbData);
        RegCloseKey(key_handle);
    }

    return lpData;
}


//这个主键/可能自启动路径下的所有子键名
//第几个，子键名 map<int, char*>形式
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


/*
    读注册表特定值 msdn文档 https://docs.microsoft.com/zh-cn/windows/win32/sysinfo/retrieving-data-from-the-registry
     应用程序通常会调用 RegEnumValue 来确定值名称，然后 RegQueryValueEx 检索名称的数据。
     枚举子项：https://docs.microsoft.com/zh-cn/windows/win32/sysinfo/enumerating-registry-subkeys
    */
   //打开服务子键，获取
KEY_VALUE* read_service_subkey_type(HKEY root_key, LPCSTR sub_key)
{
    HKEY key_handle;
    //DWORD lpData;
    //DWORD lptype = REG_DWORD;//A 32-bit number
    //DWORD lpcbData;


    //打开服务子键
    KEY_VALUE *list = new KEY_VALUE;
    if (ERROR_SUCCESS == RegOpenKeyExA(
                            root_key,
                            sub_key,
                            0,
                            KEY_READ,
                            &key_handle))
    {
        /* 会异常结束
        long ret_value;
        //https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryvalueexa
        ret_value = RegQueryValueEx(
                        key_handle, 
                        (LPCSTR)("Type"),
                        0, 
                        &lptype, 
                        (LPBYTE)&lpData, 
                        &lpcbData);

        if (ret_value == ERROR_SUCCESS)
        RegCloseKey(key_handle);
        */
       list = QueryKey(key_handle);
       
    }
    //返回type这条值的数据(数字)
    //cout<<"read "<<lpData<<endl;
    return list;


}

DWORD read_service_subkey_start(HKEY root_key, LPCSTR sub_key)
{
    HKEY key_handle;
    DWORD lpData;
    DWORD lptype = REG_DWORD;
    DWORD lpcbData;

    long ret_subkey;
    long ret_value;

    //打开服务子键
    ret_subkey = RegOpenKeyEx(root_key,
                            sub_key,
                            0,
                            KEY_READ,
                            &key_handle);

    if (ret_subkey == ERROR_SUCCESS)
        ret_value = RegQueryValueEx(
            key_handle,
            (LPCSTR)"Start",
            0,
            &lptype,
            (LPBYTE)&lpData,
            &lpcbData);

    if (ret_value == ERROR_SUCCESS)
        RegCloseKey(key_handle);

        //返回Start这条值的数据(数字)
        return lpData;
}




#endif // READ_REGISTER_SERVICE_DRIVERS_H
