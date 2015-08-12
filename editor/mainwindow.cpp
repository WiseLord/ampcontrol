#include "mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent)
{
  setupUi(this);

  lc = new LcdConverter();

}

void MainWindow::openEeprom()
{
    fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open eeprom binary"),
            "../eeprom/",
            tr("EEPROM files (eeprom_*.bin);;All files (*.*)")
    );

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    eeprom = file.readAll();
}

void MainWindow::on_pushButton_clicked()
{
  QString text;

  text = this->lineEdit->text ();

  this->listWidget->clear ();

  QByteArray ba = lc->encode(text);

  this->listWidget->addItem(ba.toHex());
}
