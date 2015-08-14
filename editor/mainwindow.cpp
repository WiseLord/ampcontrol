#include "mainwindow.h"

#include <QDebug>
#include <QtWidgets>

#include "../eeprom.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);

    lc = new LcdConverter();

    wgtHexTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 16; x++) {
            QTableWidgetItem *item = new QTableWidgetItem("00");
            wgtHexTable->setItem(y, x, item);
        }
        QTableWidgetItem *item = new QTableWidgetItem(QString("%1").arg(y * 16, 4, 16, QChar('0')).toUpper());
        wgtHexTable->setVerticalHeaderItem(y, item);
    }
    for (int x = 0; x < 16; x++) {
        QTableWidgetItem *item = new QTableWidgetItem(QString("%1").arg(x, 0, 16).toUpper());
        wgtHexTable->setHorizontalHeaderItem(x, item);
    }
    wgtHexTable->setFont(QFont("Liberation Mono"));

    eep.fill(0xFF, 1024);

    updateHexTable();
}

void MainWindow::updateHexTable()
{
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 16; x++) {
            QTableWidgetItem *item = wgtHexTable->item(y, x);
            item->setText(eep.mid(y * 16 + x, 1).toHex().toUpper());
            if (item->text() == "FF")
                item->setTextColor(Qt::gray);
            else if (item->text() == "00" && (y * 16 + x) >= eepromLabelsAddr)
                item->setTextColor(Qt::blue);
            else
                item->setTextColor(Qt::black);
        }
    }
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

    eep = file.readAll();

    updateHexTable();
}

void MainWindow::setAudioproc(int proc)
{
    eep[eepromAudioproc] = proc;
    updateHexTable();
}

void MainWindow::translated(int row, int column)
{
    Q_UNUSED(row); Q_UNUSED(column);

    QBuffer buffer(&eep);
    buffer.open(QIODevice::WriteOnly);

    buffer.seek(eepromLabelsAddr);

    for (int i = 0; i < LABEL_END; i++) {
        buffer.write(lc->encode(wgtTranslations->item(i, 0)->text()));
        buffer.putChar('\0');
    }

    while (buffer.pos() < EEPROM_SIZE) {
        buffer.putChar(0xFF);
    }

    buffer.close();

    updateHexTable();
}

void MainWindow::on_pushButton_clicked()
{
    QString text;

    text = this->lineEdit->text ();

    this->listWidget->clear ();

    QByteArray ba = lc->encode(text);

    this->listWidget->addItem(ba.toHex());
}
