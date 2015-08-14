#include "mainwindow.h"

#include <QDebug>
#include <QtWidgets>

#include "../eeprom.h"
#include "../audio/audioproc.h"

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
    wgtHexTable->setFont(QFont("Liberation Mono", 9, QFont::Bold));

    eep.fill(0xFF, 1024);

    updateTranslation(0, 0);
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
    /* Open eeprom binary file */
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

    file.close();

    updateHexTable();

    readEep();
}

void MainWindow::readEep()
{
    QBuffer buffer(&eep);
    char ch;
    int pos, len;

    /* Load text labels */
    wgtTranslations->blockSignals(true);

    buffer.open(QIODevice::ReadOnly);
    buffer.seek(eepromLabelsAddr);

    pos = buffer.pos();
    len = 0;
    buffer.getChar(&ch);

    for (int i = 0; i < LABEL_END && buffer.pos() < EEPROM_SIZE; i++) {
        while (ch == 0x00 && buffer.pos() < EEPROM_SIZE) {
            pos = buffer.pos();
            len = 0;
            buffer.getChar(&ch);
        }
        while (ch != 0x00 && buffer.pos() < EEPROM_SIZE) {
            buffer.getChar(&ch);
            len++;
        }
        wgtTranslations->item(i, 0)->setText(lc->decode(eep.mid(pos, len)));
    }
    wgtTranslations->blockSignals(false);

    buffer.close();
}

void MainWindow::updateTranslation(int row, int column)
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

void MainWindow::setAudioproc(int proc)
{
    switch (proc) {
    case value:

        break;
    default:
        break;
    }

    eep[eepromAudioproc] = proc;

    updateHexTable();
}
