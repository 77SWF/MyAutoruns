#ifndef STR_CONVERT_H
#define STR_CONVERT_H

//char * -> QString
QString charstr_to_QString(char * char_str)
{
    QString q_str;
    q_str = QString(QLatin1String(char_str));
    return q_str;
}

QString LPBYTE_to_QString(LPBYTE LPBYTE_str)
{
    char* char_str = (char*)LPBYTE_str;

    QString q_str;
    q_str = charstr_to_QString(char_str);
    return q_str;

}

//无效？不知道原因
LPCWSTR QString_to_LPCWSTR(QString q_str)
{
    return q_str.toStdWString().c_str();
}

//多字节下char *=TCHAR*
char * QString_to_char_pt(QString q_str)
{
    //QByteArray ba = q_str.toLatin1(); // must
    QByteArray ba = q_str.toLocal8Bit(); // must
    char* char_pt_str;
    char_pt_str=ba.data();

    return char_pt_str;
}
/*
TCHAR* CharToTCHAR(const char* pChar)
{
    TCHAR* pTchar = NULL;
    int nLen = strlen(pChar) + 1;
#ifdef UNICODE
    pTchar = new wchar_t[nLen];
    MultiByteToWideChar(CP_ACP, 0, pChar, nLen, pTchar, nLen);
#else
    pTchar = new char[nLen];
    wcsncpy(pTchar, pChar, nLen * sizeof(char));
#endif
    return pTchar;
}
*/
/*
TCHAR* char2TCHAR(const char* str)
{
    int size = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);

    TCHAR* retStr = new TCHAR[size * sizeof(TCHAR)];

    MultiByteToWideChar(CP_ACP, 0, str, -1, retStr, size);

    return retStr;
}

char* QString2char(QString qstr)
{
    char* ch = new char;
    QByteArray byte = qstr.toLocal8Bit();
    ch = byte.data();
    return ch;
}
*/

TCHAR *CharToWchar(const QString &str)

{
    QByteArray ba = str.toUtf8();

    char *data = ba.data(); //以上两步不能直接简化为“char *data = str.toUtf8().data();”

    int charLen = strlen(data);

    int len = MultiByteToWideChar(CP_ACP, 0, data, charLen, NULL, 0);

    TCHAR *buf = new TCHAR[len + 1];

    //MultiByteToWideChar(CP_ACP, 0, data, charLen, buf, len);
    //!!!!!!!一定要注释掉！！我用的就是多字节编码！！！！！！！！！！！

    buf[len] = '\0';

    return buf;

}

#endif // STR_CONVERT_H
