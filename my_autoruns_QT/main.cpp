#include "mainwindow.h"
#include <iostream>
using namespace std;
#include <QApplication>
#include <qlogging.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
