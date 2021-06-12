#ifndef FORMAT_SOMETHING_H
#define FORMAT_SOMETHING_H

#include <shlwapi.h>
#include <QDebug>
#include "str_convert.h"

void format_description_path(QString* value)
{
    if(value->contains("%systemroot%", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("%systemroot%", 0, Qt::CaseInsensitive), 12, "C:\\Windows");
    if(value->contains("%localappdata%", Qt::CaseInsensitive))
    {
        char* env_real;
        env_real = getenv("localappdata");//和登录用户有关，不可手动设定
        *value = value->replace(value->indexOf("%localappdata%", 0, Qt::CaseInsensitive), 14, env_real);
    }
    if(value->contains("%programfiles%", Qt::CaseInsensitive))
    {
        char* env_real;
        env_real = getenv("programfiles");//和登录用户有关，不可手动设定
        *value = value->replace(value->indexOf("%programfiles%", 0, Qt::CaseInsensitive), 14, env_real);
    }
    if(value->contains("\\systemroot", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("\\systemroot", 0, Qt::CaseInsensitive), 11, "C:\\Windows");
    if(value->contains("%windir%", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("%windir%", 0, Qt::CaseInsensitive), 8, "C:\\Windows");
}

/*
    注册表中的value imagepath 可能会带有命令参数、系统变量等
    将其标准化为一个文件路径（QString）
    */
void format_imagepath(QString* value)
{

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
    if(value->contains("%localappdata%", Qt::CaseInsensitive))
    {
        char* env_real;
        env_real = getenv("localappdata");//和登录用户有关，不可手动设定
        *value = value->replace(value->indexOf("%localappdata%", 0, Qt::CaseInsensitive), 14, env_real);
    }
    if(value->contains("%programfiles%", Qt::CaseInsensitive))
    {
        char* env_real;
        env_real = getenv("programfiles");//和登录用户有关，不可手动设定
        *value = value->replace(value->indexOf("%programfiles%", 0, Qt::CaseInsensitive), 14, env_real);
    }
    if(value->contains("\\systemroot", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("\\systemroot", 0, Qt::CaseInsensitive), 11, "C:\\Windows");
    if(value->contains("%windir%", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("%windir%", 0, Qt::CaseInsensitive), 8, "C:\\Windows");
    if(value->contains("\\??\\", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("\\??\\"), 4, "");
    if(!value->contains(":"))
    {
        if(value->contains("system32", Qt::CaseInsensitive))
            *value = value->replace(value->indexOf("system32", 0, Qt::CaseInsensitive), 8, "C:\\Windows\\System32");
        if(value->contains("syswow64", Qt::CaseInsensitive))
            *value = value->replace(value->indexOf("syswow64", 0, Qt::CaseInsensitive), 8, "C:\\Windows\\SysWOW64");
    }
}


//引用传值
void format_description(QString path,QString *content)
{
    PWSTR pszOutBuf = new WCHAR[1024];

    //参数 PCWSTR PWSTR UINT NULL
    //获取到了，PWSTR转换QString有问题
    if( SHLoadIndirectString(path.toStdWString().c_str(),pszOutBuf,1024,NULL) == S_OK)
    {
        //qDebug()<<pszOutBuf;//报错内存泄露
        *content = QString::fromStdWString(pszOutBuf);
        /*
        int len = WideCharToMultiByte(CP_ACP, 0, pszOutBuf, wcslen(pszOutBuf), NULL, 0, NULL, NULL);
        char	*m_char = new char[len + 1];
        WideCharToMultiByte(CP_ACP, 0, pszOutBuf, wcslen(pszOutBuf), m_char, len, NULL, NULL);
        m_char[len] = '\0';
        *content = charstr_to_QString(m_char);
        */
    }
    else
        *content = *content;

    delete[] pszOutBuf;

}


#endif // FORMAT_SOMETHING_H
