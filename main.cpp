#include "kmatch.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(utf8);
    QApplication a(argc, argv);
    KMatch w;
    w.show();

    return a.exec();
}
