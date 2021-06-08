#include <qDebug.h>


void main()
{
    const char *c1 = "Hello World 1!";
    QString s1(c1);
    qDebug() << c1 << s1;
}