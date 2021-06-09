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

#endif // STR_CONVERT_H
