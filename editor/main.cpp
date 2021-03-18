#include "mainwindow.h"
#include <QApplication>
#include "defines.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
