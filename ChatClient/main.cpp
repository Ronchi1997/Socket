#include <QApplication>
#include "login.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Login w;
    w.show();
    w.setWindowTitle("Login");
    return a.exec();
}
