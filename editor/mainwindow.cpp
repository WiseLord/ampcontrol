#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent)
{
  setupUi(this);

  lc = new LcdConverter();
}

void MainWindow::on_pushButton_clicked()
{
  QString text;

  text = this->lineEdit->text ();

  this->listWidget->clear ();

  QByteArray ba = lc->encode(text);

  this->listWidget->addItem(ba.toHex());

}
