#include "mainwindow.h"
#include "setupdialog.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);

    QApplication a(argc, argv);

    QTranslator translator;
    translator.load(":/ts/ts/remote_" + QLocale::system().bcp47Name());
    a.installTranslator(&translator);

    MainWindow w;

    return a.exec();
}
