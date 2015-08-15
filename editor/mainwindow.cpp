#include "mainwindow.h"

#include <QDebug>
#include <QtWidgets>

#include "../audio/audio.h"
#include "../audio/audioproc.h"
#include "../eeprom.h"
#include "../tuner/tuner.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);

    lc = new LcdConverter();

    /* Create hex table */
    wgtHexTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wgtHexTable->setFont(QFont(QFontDatabase::systemFont(QFontDatabase::FixedFont).family(), 9, QFont::Bold));
    wgtHexTable->verticalHeader()->setFont(QFont(QFontDatabase::systemFont(QFontDatabase::FixedFont).family(), 9, QFont::Bold));
    wgtHexTable->horizontalHeader()->setFont(QFont(QFontDatabase::systemFont(QFontDatabase::FixedFont).family(), 9, QFont::Bold));
    for (int y = 0; y < 64; y++) {
        wgtHexTable->setVerticalHeaderItem(y, new QTableWidgetItem(QString("%1").arg(y * 16, 4, 16, QChar('0')).toUpper()));
        for (int x = 0; x < 16; x++)
            wgtHexTable->setItem(y, x, new QTableWidgetItem());
    }
    for (int x = 0; x < 16; x++)
        wgtHexTable->setHorizontalHeaderItem(x, new QTableWidgetItem(QString("%1").arg(x, 0, 16).toUpper()));

    /* Create translations table */
    wgtTranslations->blockSignals(true);
    for (int y = 0; y < LABEL_END; y++)
        wgtTranslations->setItem(y, 0, new QTableWidgetItem());
    wgtTranslations->blockSignals(false);

    /* Load default eeprom file */
    readEepromFile(EEPROM_RESOURCE);
}

void MainWindow::updateHexTable(int pos)
{
    QTableWidgetItem *item = wgtHexTable->item(pos / 16, pos % 16);
    item->setText(eep.mid(pos, 1).toHex().toUpper());
    if (item->text() == "FF")
        item->setTextColor(Qt::gray);
    else if (item->text() == "00" && (pos) >= eepromLabelsAddr)
        item->setTextColor(Qt::blue);
    else
        item->setTextColor(Qt::black);
}

void MainWindow::updateHexTable()
{
    for (int pos = 0; pos < 1024; pos++)
        updateHexTable(pos);
}

void MainWindow::setAudioParam(QDoubleSpinBox *spb, double min, double max, double step, int param)
{
    spb->setRange(min, max);
    spb->setSingleStep(step);
    spb->setValue(eep[eepromVolume + param] * step);
}

void MainWindow::readEepromFile(QString name)
{
    QFile file(name);

    if (!file.open(QIODevice::ReadOnly))
        return;
    if (name != EEPROM_RESOURCE) {
        actionSaveEeprom->setEnabled(true);
        fileName = name;
        Ui_MainWindow::statusBar->showMessage(
                    tr("File") + " " + fileName + " " + tr("loaded"));
    } else {
        actionSaveEeprom->setEnabled(false);
        fileName.clear();
        Ui_MainWindow::statusBar->showMessage("Default eeprom loaded");
    }

    eep = file.readAll();
    file.close();
    updateHexTable();

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

    int proc = eep[eepromAudioproc];
    if (proc >= AUDIOPROC_END)
        proc = AUDIOPROC_TDA7439;
    setAudioproc(proc);
}

void MainWindow::saveEepromFile(QString name)
{
    fileName = name;

    QFile file(name);
    if (!file.open(QIODevice::WriteOnly)) {
        Ui_MainWindow::statusBar->showMessage(tr("Can't save") + " " + name);
        return;
    }
    file.write(eep);
    file.close();
    Ui_MainWindow::statusBar->showMessage(tr("Saved as") + " " + name);

}

void MainWindow::openEeprom()
{
    QString name = QFileDialog::getOpenFileName(this,
        tr("Open eeprom binary"),
        "../eeprom/",
        tr("EEPROM files (*.bin);;All files (*.*)"));

    readEepromFile(name);
}

void MainWindow::saveEeprom()
{
    saveEepromFile(fileName);
}

void MainWindow::saveEepromAs()
{
    QString name = QFileDialog::getSaveFileName(this,
        tr("Save eeprom binary"),
        "../eeprom/" + fileName,
        tr("EEPROM files (*.bin)"));

    if (name.isEmpty())
        return;

    saveEepromFile(name);
}

void MainWindow::loadDefaultEeprom()
{
    readEepromFile(EEPROM_RESOURCE);
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
    cbxAudioproc->setCurrentIndex(proc);

    wgtVolume->hide();
    wgtBass->hide();
    wgtMiddle->hide();
    wgtTreble->hide();
    wgtPreamp->hide();
    wgtFrontrear->hide();
    wgtBalance->hide();
    wgtCenter->hide();
    wgtSubwoofer->hide();
    wgtGain0->hide();
    wgtGain1->hide();
    wgtGain2->hide();
    wgtGain3->hide();
    wgtLoudness->hide();
    wgtInput->hide();
    cbxInput->clear();

    /* Handle loudness */
    switch (proc) {
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_TDA7314:
        wgtLoudness->show();
        setLoudness(0x01);
        cbxLoudness->setCurrentIndex(eep[eepromLoudness]);
        break;
    }

    /* Handle maximum inputs */
    switch (proc) {
    case AUDIOPROC_TDA7439:
        cbxInput->insertItem(0, wgtTranslations->item(MODE_SND_GAIN3, 0)->text());
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_TDA7318:
        cbxInput->insertItem(0, wgtTranslations->item(MODE_SND_GAIN2, 0)->text());
        cbxInput->insertItem(0, wgtTranslations->item(MODE_SND_GAIN1, 0)->text());
        cbxInput->insertItem(0, wgtTranslations->item(MODE_SND_GAIN0, 0)->text());
        wgtInput->show();
        setInput(eep[eepromInput]);
        cbxInput->setCurrentIndex(eep[eepromInput]);
        break;
    }

    /* Common TDA73x1 audio parameters */
    switch (proc) {
    case AUDIOPROC_TDA7312:
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_TDA7314:
    case AUDIOPROC_TDA7318:
        wgtVolume->show();
        setAudioParam(dsbVolume, -78.5, 0, 1.25, MODE_SND_VOLUME);
        wgtBass->show();
        setAudioParam(dsbBass, -14, 14, 2, MODE_SND_BASS);
        wgtTreble->show();
        setAudioParam(dsbTreble, -14, 14, 2, MODE_SND_TREBLE);
        wgtBalance->show();
        setAudioParam(dsbBalance, -18.75, 18.75, 1.25, MODE_SND_BALANCE);
        break;
    }

    switch (proc) {
    case AUDIOPROC_TDA7439:
        wgtVolume->show();
        setAudioParam(dsbVolume, -79, 0, 1, MODE_SND_VOLUME);
        wgtBass->show();
        setAudioParam(dsbBass, -14, 14, 2, MODE_SND_BASS);
        wgtMiddle->show();
        setAudioParam(dsbMiddle, -14, 14, 2, MODE_SND_MIDDLE);
        wgtTreble->show();
        setAudioParam(dsbTreble, -14, 14, 2, MODE_SND_TREBLE);
        wgtPreamp->show();
        setAudioParam(dsbPreamp, -47, 0, 1, MODE_SND_PREAMP);
        wgtBalance->show();
        setAudioParam(dsbBalance, -21, 21, 1, MODE_SND_BALANCE);
        wgtGain0->show();
        setAudioParam(dsbGain0, 0, 30, 2, MODE_SND_GAIN0);
        wgtGain1->show();
        setAudioParam(dsbGain1, 0, 30, 2, MODE_SND_GAIN1);
        wgtGain2->show();
        setAudioParam(dsbGain2, 0, 30, 2, MODE_SND_GAIN2);
        wgtGain3->show();
        setAudioParam(dsbGain3, 0, 30, 2, MODE_SND_GAIN3);
        break;
    case AUDIOPROC_TDA7313:
        wgtFrontrear->show();
        setAudioParam(dsbFrontrear, -18.75, 18.75, 1.25, MODE_SND_FRONTREAR);
        wgtGain0->show();
        setAudioParam(dsbGain0, 0, 11.25, 3.75, MODE_SND_GAIN0);
        wgtGain1->show();
        setAudioParam(dsbGain1, 0, 11.25, 3.75, MODE_SND_GAIN1);
        wgtGain2->show();
        setAudioParam(dsbGain2, 0, 11.25, 3.75, MODE_SND_GAIN2);
        break;
    case AUDIOPROC_TDA7314:
        wgtFrontrear->show();
        setAudioParam(dsbFrontrear, -18.75, 18.75, 1.25, MODE_SND_FRONTREAR);
        wgtGain0->show();
        setAudioParam(dsbGain0, 0, 18.75, 6.25, MODE_SND_GAIN0);
        break;
    case AUDIOPROC_TDA7318:
        wgtFrontrear->show();
        setAudioParam(dsbFrontrear, -18.75, 18.75, 1.25, MODE_SND_FRONTREAR);
        wgtGain0->show();
        setAudioParam(dsbGain0, 0, 18.75, 6.25, MODE_SND_GAIN0);
        wgtGain1->show();
        setAudioParam(dsbGain1, 0, 18.75, 6.25, MODE_SND_GAIN1);
        wgtGain2->show();
        setAudioParam(dsbGain2, 0, 18.75, 6.25, MODE_SND_GAIN2);
        wgtGain3->show();
        setAudioParam(dsbGain3, 0, 18.75, 6.25, MODE_SND_GAIN3);
        break;
    case AUDIOPROC_TDA7448:
        wgtVolume->show();
        setAudioParam(dsbVolume, -79, 0, 1, MODE_SND_VOLUME);
        wgtFrontrear->show();
        setAudioParam(dsbFrontrear, -21, 21, 1, MODE_SND_FRONTREAR);
        wgtBalance->show();
        setAudioParam(dsbBalance, -21, 21, 1, MODE_SND_BALANCE);
        wgtCenter->show();
        setAudioParam(dsbCenter, -21, 0, 1, MODE_SND_CENTER);
        wgtSubwoofer->show();
        setAudioParam(dsbSubwoofer, -21, 0, 1, MODE_SND_SUBWOOFER);
        break;
    case AUDIOPROC_PGA2310:
        wgtVolume->show();
        setAudioParam(dsbVolume, -96, 31, 1, MODE_SND_VOLUME);
        wgtBalance->show();
        setAudioParam(dsbBalance, -21, 21, 1, MODE_SND_BALANCE);
        break;
    }

    eep[eepromAudioproc] = proc;
    updateHexTable(eepromAudioproc);
}

void MainWindow::setVolume(double value)
{
    eep[eepromVolume + MODE_SND_VOLUME] = static_cast<char>(value / dsbVolume->singleStep());
    updateHexTable(eepromVolume + MODE_SND_VOLUME);
}

void MainWindow::setBass(double value)
{
    eep[eepromVolume + MODE_SND_BASS] = static_cast<char>(value / dsbBass->singleStep());
    updateHexTable(eepromVolume + MODE_SND_BASS);
}

void MainWindow::setMiddle(double value)
{
    eep[eepromVolume + MODE_SND_MIDDLE] = static_cast<char>(value / dsbMiddle->singleStep());
    updateHexTable(eepromVolume + MODE_SND_MIDDLE);
}

void MainWindow::setTreble(double value)
{
    eep[eepromVolume + MODE_SND_TREBLE] = static_cast<char>(value / dsbTreble->singleStep());
    updateHexTable(eepromVolume + MODE_SND_TREBLE);
}

void MainWindow::setPreamp(double value)
{
    eep[eepromVolume + MODE_SND_PREAMP] = static_cast<char>(value / dsbPreamp->singleStep());
    updateHexTable(eepromVolume + MODE_SND_PREAMP);
}

void MainWindow::setFrontrear(double value)
{
    eep[eepromVolume + MODE_SND_FRONTREAR] = static_cast<char>(value / dsbFrontrear->singleStep());
    updateHexTable(eepromVolume + MODE_SND_FRONTREAR);
}

void MainWindow::setBalance(double value)
{
    eep[eepromVolume + MODE_SND_BALANCE] = static_cast<char>(value / dsbBalance->singleStep());
    updateHexTable(eepromVolume + MODE_SND_BALANCE);
}

void MainWindow::setCenter(double value)
{
    eep[eepromVolume + MODE_SND_CENTER] = static_cast<char>(value / dsbCenter->singleStep());
    updateHexTable(eepromVolume + MODE_SND_CENTER);
}

void MainWindow::setSubwoofer(double value)
{
    eep[eepromVolume + MODE_SND_SUBWOOFER] = static_cast<char>(value / dsbSubwoofer->singleStep());
    updateHexTable(eepromVolume + MODE_SND_SUBWOOFER);
}

void MainWindow::setGain0(double value)
{
    eep[eepromVolume + MODE_SND_GAIN0] = static_cast<char>(value / dsbGain0->singleStep());
    updateHexTable(eepromVolume + MODE_SND_GAIN0);
}

void MainWindow::setGain1(double value)
{
    eep[eepromVolume + MODE_SND_GAIN1] = static_cast<char>(value / dsbGain1->singleStep());
    updateHexTable(eepromVolume + MODE_SND_GAIN1);
}

void MainWindow::setGain2(double value)
{
    eep[eepromVolume + MODE_SND_GAIN2] = static_cast<char>(value / dsbGain2->singleStep());
    updateHexTable(eepromVolume + MODE_SND_GAIN2);
}

void MainWindow::setGain3(double value)
{
    eep[eepromVolume + MODE_SND_GAIN3] = static_cast<char>(value / dsbGain3->singleStep());
    updateHexTable(eepromVolume + MODE_SND_GAIN3);
}

void MainWindow::setInput(int value)
{
    if (value >= cbxInput->count())
        value = cbxInput->count() - 1;
    eep[eepromInput] = (char)value;
    updateHexTable(eepromInput);
}

void MainWindow::setLoudness(int value)
{
    if (value)
        eep[eepromLoudness] = 0x01;
    else
        eep[eepromLoudness] = 0x00;
    updateHexTable(eepromLoudness);
}
