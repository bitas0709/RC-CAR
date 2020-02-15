#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("RysCorp");
    QCoreApplication::setOrganizationDomain("oc4.pp.ua");
    QCoreApplication::setApplicationName("RC-CAR");
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Resources/Icons/AppIcon.png"));
    MainWindow w;
    w.setWindowIcon(QIcon(":/Resources/Icons/AppIcon.png"));
    w.setWindowTitle("RC-CAR Control App");
    w.show();

    return a.exec();
}
