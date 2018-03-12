#include "mainwindow.h"

#include <QDebug>
#include <QtWidgets>

#include "defines.h"
#include "aboutdialog.h"

#include "../audio/audio.h"
#include "../eeprom.h"
#include "../tuner/tuner.h"
#include "../tuner/tea5767.h"
#include "../display.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);

    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

    QString lang = settings.value(SETTINGS_GENERAL_LANGUAGE, "auto").toString();
    actionLangAuto->setChecked(lang.compare("auto") == 0);
    actionLangBelarusian->setChecked(lang.compare("by") == 0);
    actionLangEnglish->setChecked(lang.compare("en") == 0);
    actionLangRussian->setChecked(lang.compare("ru") == 0);

    retranslate(lang);

    QActionGroup* langGroup = new QActionGroup(this);
    actionLangAuto->setActionGroup(langGroup);
    actionLangBelarusian->setActionGroup(langGroup);
    actionLangEnglish->setActionGroup(langGroup);
    actionLangRussian->setActionGroup(langGroup);

    QFontDatabase::addApplicationFont("://fonts/LiberationMono-Bold.ttf");
    fontHex = QFont("Liberation Mono", 9, QFont::Bold);

    lc = new LcdConverter();

    /* Create hex table */
    wgtHexTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wgtHexTable->setFont(fontHex);
    wgtHexTable->verticalHeader()->setFont(fontHex);
    wgtHexTable->horizontalHeader()->setFont(fontHex);
    for (int y = 0; y < wgtHexTable->rowCount(); y++) {
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
    if (item->text() == "FF" && pos >= MODE_SND_END)
        item->setTextColor(Qt::gray);
    else if (item->text() == "00" && (pos) >= EEPROM_LABELS_ADDR)
        item->setTextColor(Qt::blue);
    else
        item->setTextColor(Qt::black);
}

void MainWindow::updateHexTable()
{
    for (int pos = 0; pos < wgtHexTable->rowCount() * 16; pos++)
        updateHexTable(pos);
}

void MainWindow::readEepromFile(QString name)
{
    /* Reading file to QByteArray buffer */

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

    // Processing translations

    QBuffer buffer(&eep);
    char ch;
    int pos, len;

    /* Load text labels */
    wgtTranslations->blockSignals(true);

    buffer.open(QIODevice::ReadOnly);
    buffer.seek(EEPROM_LABELS_ADDR);

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
        wgtTranslations->item(i, 0)->setText(lc->decode(eep.mid(pos, len), LcdConverter::MAP_KS0066_RU));
    }
    wgtTranslations->blockSignals(false);

    buffer.close();

    // Processing audioprocessor

    int proc = eep[EEPROM_AUDIOPROC];
    if (proc >= AUDIOPROC_END)
        proc = AUDIOPROC_TDA7439;
    setAudioproc(proc);

    // Processing tuner
    int tuner = eep[EEPROM_FM_TUNER];
    if (tuner >= TUNER_END)
        tuner = TUNER_TEA5767;
    setTuner(tuner);

    // Processing remote
    // int rcType = eep[EEPROM_RC_TYPE];
    // Hardcode RC5 if (rcType + 1 >= cbxRemoteType->count())
    //    rcType = 0;
    sbxRemoteAddr->setValue(eep[EEPROM_RC_ADDR]);
    lwCommands->setCurrentRow(0);

    // Processing other functions
    setOther();
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

void MainWindow::setAudioParam(QDoubleSpinBox *spb, double min, double max, double step, int param)
{
    spb->setRange(min, max);
    spb->setSingleStep(step);
    spb->setValue(eep[EEPROM_VOLUME + param] * step);
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
    buffer.seek(EEPROM_LABELS_ADDR);

    for (int i = 0; i < LABEL_END; i++) {
        QString str = wgtTranslations->item(i, 0)->text();
        if (str.isEmpty())
            str = " ";
        buffer.write(lc->encode(str, LcdConverter::MAP_KS0066_RU));
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
    wgtTreble->hide();
    wgtBalance->hide();
    wgtLoudness->hide();
    wgtSurround->hide();
    wgtEffect3d->hide();
    wgtToneDefeat->hide();
    wgtInput->hide();
    cbxInput->clear();

    /* Handle loudness/surround/effect3d/tone_defeat*/
    switch (proc) {
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_TDA7314:
    case AUDIOPROC_TDA7315:
    case AUDIOPROC_PT2314:
    case AUDIOPROC_RDA580X:
        wgtLoudness->show();
        cbxLoudness->setCurrentIndex(!!(eep[EEPROM_APROC_EXTRA] & APROC_EXTRA_LOUDNESS));
        break;
    case AUDIOPROC_PT232X:
        wgtSurround->show();
        cbxSurround->setCurrentIndex(!!(eep[EEPROM_APROC_EXTRA] & APROC_EXTRA_SURROUND));
        wgtEffect3d->show();
        cbxEffect3d->setCurrentIndex(!!(eep[EEPROM_APROC_EXTRA] & APROC_EXTRA_EFFECT3D));
        wgtToneDefeat->show();
        cbxToneDefeat->setCurrentIndex(!!(eep[EEPROM_APROC_EXTRA] & APROC_EXTRA_TONEDEFEAT));
        break;
    }

    /* Handle maximum inputs */
    switch (proc) {
    case AUDIOPROC_NO:
        break;
    case AUDIOPROC_PT232X:
        cbxInput->insertItem(0, "Input 5");
    case AUDIOPROC_TDA7439:
    case AUDIOPROC_TDA7312:
    case AUDIOPROC_TDA7318:
    case AUDIOPROC_PT2314:
        cbxInput->insertItem(0, "Input 4");
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_TEA6300:
        cbxInput->insertItem(0, "Input 3");
        cbxInput->insertItem(0, "Input 2");
    default:
        cbxInput->insertItem(0, "Input 1");
        wgtInput->show();
        setInput(eep[EEPROM_INPUT]);
        cbxInput->setCurrentIndex(eep[EEPROM_INPUT]);
        break;
    }

    /* Common TDA73x1 audio parameters */
    switch (proc) {
    case AUDIOPROC_TDA7439:
        wgtVolume->show();
        setAudioParam(dsbVolume, -79, 0, 1, MODE_SND_VOLUME);
        wgtBass->show();
        setAudioParam(dsbBass, -14, 14, 2, MODE_SND_BASS);
        wgtTreble->show();
        setAudioParam(dsbTreble, -14, 14, 2, MODE_SND_TREBLE);
        wgtBalance->show();
        setAudioParam(dsbBalance, -15, 15, 1, MODE_SND_BALANCE);
        break;
    case AUDIOPROC_TDA7312:
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_TDA7314:
    case AUDIOPROC_TDA7315:
    case AUDIOPROC_TDA7318:
    case AUDIOPROC_PT2314:
        wgtVolume->show();
        setAudioParam(dsbVolume, -78.5, 0, 1.25, MODE_SND_VOLUME);
        wgtBass->show();
        setAudioParam(dsbBass, -14, 14, 2, MODE_SND_BASS);
        wgtTreble->show();
        setAudioParam(dsbTreble, -14, 14, 2, MODE_SND_TREBLE);
        wgtBalance->show();
        setAudioParam(dsbBalance, -18.75, 18.75, 1.25, MODE_SND_BALANCE);
        break;
    case AUDIOPROC_TDA7448:
        wgtVolume->show();
        setAudioParam(dsbVolume, -79, 0, 1, MODE_SND_VOLUME);
        wgtBalance->show();
        setAudioParam(dsbBalance, -7, 7, 1, MODE_SND_BALANCE);
        break;
    case AUDIOPROC_PT232X:
        wgtVolume->show();
        setAudioParam(dsbVolume, -79, 0, 1, MODE_SND_VOLUME);
        wgtBass->show();
        setAudioParam(dsbBass, -14, 14, 2, MODE_SND_BASS);
        wgtTreble->show();
        setAudioParam(dsbTreble, -14, 14, 2, MODE_SND_TREBLE);
        wgtBalance->show();
        setAudioParam(dsbBalance, -7, 7, 1, MODE_SND_BALANCE);
        break;
    case AUDIOPROC_TEA6300:
    case AUDIOPROC_TEA6330:
        wgtVolume->show();
        setAudioParam(dsbVolume, -66, 20, 2, MODE_SND_VOLUME);
        wgtBass->show();
        setAudioParam(dsbBass, -12, 15, 3, MODE_SND_BASS);
        wgtTreble->show();
        setAudioParam(dsbTreble, -12, 12, 3, MODE_SND_TREBLE);
        wgtBalance->show();
        setAudioParam(dsbBalance, -14, 14, 2, MODE_SND_BALANCE);
        break;
        break;
    case AUDIOPROC_PGA2310:
        wgtVolume->show();
        setAudioParam(dsbVolume, -96, 31, 1, MODE_SND_VOLUME);
        wgtBalance->show();
        setAudioParam(dsbBalance, -15, 15, 1, MODE_SND_BALANCE);
        break;
    case AUDIOPROC_RDA580X:
        wgtVolume->show();
        setAudioParam(dsbVolume, 0, 15, 1, MODE_SND_VOLUME);
        break;
    }

    eep[EEPROM_AUDIOPROC] = proc;
    updateHexTable(EEPROM_AUDIOPROC);
}

void MainWindow::setVolume(double value)
{
    eep[EEPROM_VOLUME + MODE_SND_VOLUME] = static_cast<char>(value / dsbVolume->singleStep());
    updateHexTable(EEPROM_VOLUME + MODE_SND_VOLUME);
}

void MainWindow::setBass(double value)
{
    eep[EEPROM_VOLUME + MODE_SND_BASS] = static_cast<char>(value / dsbBass->singleStep());
    updateHexTable(EEPROM_VOLUME + MODE_SND_BASS);
}

void MainWindow::setTreble(double value)
{
    eep[EEPROM_VOLUME + MODE_SND_TREBLE] = static_cast<char>(value / dsbTreble->singleStep());
    updateHexTable(EEPROM_VOLUME + MODE_SND_TREBLE);
}

void MainWindow::setBalance(double value)
{
    eep[EEPROM_VOLUME + MODE_SND_BALANCE] = static_cast<char>(value / dsbBalance->singleStep());
    updateHexTable(EEPROM_VOLUME + MODE_SND_BALANCE);
}

void MainWindow::setInput(int value)
{
    if (value >= cbxInput->count())
        value = cbxInput->count() - 1;
    eep[EEPROM_INPUT] = (char)value;
    updateHexTable(EEPROM_INPUT);
}

void MainWindow::setLoudness(int value)
{
    if (value)
        eep[EEPROM_APROC_EXTRA] = eep[EEPROM_APROC_EXTRA] | APROC_EXTRA_LOUDNESS;
    else
        eep[EEPROM_APROC_EXTRA] = eep[EEPROM_APROC_EXTRA] & ~APROC_EXTRA_LOUDNESS;
    updateHexTable(EEPROM_APROC_EXTRA);
}

void MainWindow::setSurround(int value)
{
    if (value)
        eep[EEPROM_APROC_EXTRA] = eep[EEPROM_APROC_EXTRA] | APROC_EXTRA_SURROUND;
    else
        eep[EEPROM_APROC_EXTRA] = eep[EEPROM_APROC_EXTRA] & ~APROC_EXTRA_SURROUND;
    updateHexTable(EEPROM_APROC_EXTRA);
}

void MainWindow::setEffect3d(int value)
{
    if (value)
        eep[EEPROM_APROC_EXTRA] = eep[EEPROM_APROC_EXTRA] | APROC_EXTRA_EFFECT3D;
    else
        eep[EEPROM_APROC_EXTRA] = eep[EEPROM_APROC_EXTRA] & ~APROC_EXTRA_EFFECT3D;
    updateHexTable(EEPROM_APROC_EXTRA);
}

void MainWindow::setToneDefeat(int value)
{
    if (value)
        eep[EEPROM_APROC_EXTRA] = eep[EEPROM_APROC_EXTRA] | APROC_EXTRA_TONEDEFEAT;
    else
        eep[EEPROM_APROC_EXTRA] = eep[EEPROM_APROC_EXTRA] & ~APROC_EXTRA_TONEDEFEAT;
    updateHexTable(EEPROM_APROC_EXTRA);
}

double MainWindow::getFreq(int pos)
{
    double freq;

    freq = (unsigned char)eep[pos];
    freq += (unsigned char)eep[pos + 1] * 256;
    freq /= 100;

    return freq;
}

void MainWindow::setFreq(double value, int pos)
{
    int freq = value * 100;

    eep[pos] = (char)(freq & 0x00FF);
    eep[pos + 1] = (char)((freq & 0xFF00) >> 8);

    updateHexTable(pos);
    updateHexTable(pos + 1);
}

int MainWindow::fmStepEep2Index(uint8_t value)
{
    if (value < 2)
        return 0;
    else if (value < 5)
        return 1;
    else if (value < 10)
        return 2;
    else if (value < 20)
        return 3;
    else
        return 4;
}

uint8_t MainWindow::fmStepIndex2Step(uint8_t index)
{
    switch (index) {
    case 1:
        return 2;
    case 2:
        return 5;
    case 3:
        return 10;
    case 4:
        return 20;
    default:
        return 1;
    }
}

void MainWindow::about()
{
    AboutDialog dlg;

    dlg.exec();
}

void MainWindow::aboutQt()
{
    qApp->aboutQt();
}

void MainWindow::setTuner(int tuner)
{
    cbxTuner->setCurrentIndex(tuner);
    wgtFmfreq->hide();
    wgtFmMin->hide();
    wgtFmMax->hide();
    wgtFmstep1->hide();
    wgtFmstep2->hide();
    wgtFmmono->hide();
    wgtFmRDS->hide();
    wgtFmctrl->hide();

    switch (tuner) {
    case TUNER_TEA5767:
        wgtFmctrl->show();
        cbxFmctrlHcc->setChecked(eep[EEPROM_FM_CTRL] & TEA5767_HCC);
        cbxFmctrlSnc->setChecked(eep[EEPROM_FM_CTRL] & TEA5767_SNC);
        cbxFmctrlSm->setChecked(eep[EEPROM_FM_CTRL] & TEA5767_SMUTE);
        cbxFmctrlDtc->setChecked(eep[EEPROM_FM_CTRL] & TEA5767_DTC);
        cbxFmctrlBl->setChecked(eep[EEPROM_FM_CTRL] & TEA5767_BL);
        cbxFmctrlPllref->setChecked(eep[EEPROM_FM_CTRL] & TEA5767_PLLREF);
        cbxFmctrlXtal->setChecked(eep[EEPROM_FM_CTRL] & TEA5767_XTAL);
    case TUNER_RDA5807:
    case TUNER_RDA5807_DF:
        if (tuner != TUNER_TEA5767)
            wgtFmRDS->show();
        setFmRds(eep[EEPROM_FM_RDS]);
        cbxFmRDS->setCurrentIndex(eep[EEPROM_FM_RDS]);
    case TUNER_RDA5802:
        wgtFmmono->show();
        setFmmono(eep[EEPROM_FM_MONO]);
        cbxFmmono->setCurrentIndex(eep[EEPROM_FM_MONO]);
    case TUNER_TUX032:
    case TUNER_LM7001:
    case TUNER_LC72131:
        wgtFmfreq->show();
        wgtFmMin->show();
        wgtFmMax->show();
        dsbFmfreq->setValue(getFreq(EEPROM_FM_FREQ));
        dsbFmMin->setValue(getFreq(EEPROM_FM_FREQ_MIN));
        dsbFmMax->setValue(getFreq(EEPROM_FM_FREQ_MAX));
        if (dsbFmfreq->value() < 76)
            dsbFmfreq->setSingleStep((double)eep[EEPROM_FM_STEP1] / 100);
        else
            dsbFmfreq->setSingleStep((double)eep[EEPROM_FM_STEP2] / 100);
        wgtFmstep1->show();
        wgtFmstep2->show();
        cbxFmstep1->setCurrentIndex(fmStepEep2Index(eep[EEPROM_FM_STEP1]));
        cbxFmstep2->setCurrentIndex(fmStepEep2Index(eep[EEPROM_FM_STEP2]));
        break;
    }

    eep[EEPROM_FM_TUNER] = tuner;
    updateHexTable(EEPROM_FM_TUNER);
}

void MainWindow::setFmfreq(double value)
{
    if (value < 76)
        dsbFmfreq->setSingleStep((double)fmStepIndex2Step(cbxFmstep1->currentIndex()) / 100);
    else
        dsbFmfreq->setSingleStep((double)fmStepIndex2Step(cbxFmstep2->currentIndex()) / 100);
    setFreq(value, EEPROM_FM_FREQ);
}

void MainWindow::setFmMin(double value)
{
    if (value < 76)
        dsbFmMin->setSingleStep((double)fmStepIndex2Step(cbxFmstep1->currentIndex()) / 100);
    else
        dsbFmMin->setSingleStep((double)fmStepIndex2Step(cbxFmstep2->currentIndex()) / 100);
    setFreq(value, EEPROM_FM_FREQ_MIN);
}

void MainWindow::setFmMax(double value)
{
    if (value < 76)
        dsbFmMax->setSingleStep((double)fmStepIndex2Step(cbxFmstep1->currentIndex()) / 100);
    else
        dsbFmMax->setSingleStep((double)fmStepIndex2Step(cbxFmstep2->currentIndex()) / 100);
    setFreq(value, EEPROM_FM_FREQ_MAX);
}

void MainWindow::setFmstep1(int value)
{
    eep[EEPROM_FM_STEP1] = fmStepIndex2Step(value);
    updateHexTable(EEPROM_FM_STEP1);
}

void MainWindow::setFmstep2(int value)
{
    eep[EEPROM_FM_STEP2] = fmStepIndex2Step(value);
    updateHexTable(EEPROM_FM_STEP2);
}

void MainWindow::setFmmono(int value)
{
    if (value)
        eep[EEPROM_FM_MONO] = 0x01;
    else
        eep[EEPROM_FM_MONO] = 0x00;
    updateHexTable(EEPROM_FM_MONO);
}

void MainWindow::setFmRds(int value)
{
    if (value)
        eep[EEPROM_FM_RDS] = 0x01;
    else
        eep[EEPROM_FM_RDS] = 0x00;
    updateHexTable(EEPROM_FM_RDS);
}

void MainWindow::setFmctrl()
{
    char ctrl = 0;

    if (cbxFmctrlHcc->isChecked()) ctrl |= TEA5767_HCC;
    if (cbxFmctrlSnc->isChecked()) ctrl |= TEA5767_SNC;
    if (cbxFmctrlSm->isChecked()) ctrl |= TEA5767_SMUTE;
    if (cbxFmctrlDtc->isChecked()) ctrl |= TEA5767_DTC;
    if (cbxFmctrlBl->isChecked()) ctrl |= TEA5767_BL;
    if (cbxFmctrlPllref->isChecked()) ctrl |= TEA5767_PLLREF;
    if (cbxFmctrlXtal->isChecked()) ctrl |= TEA5767_XTAL;

    eep[EEPROM_FM_CTRL] = ctrl;
    updateHexTable(EEPROM_FM_CTRL);
}

void MainWindow::setRemoteType(int type)
{
    eep[EEPROM_RC_TYPE] = type;
    updateHexTable(EEPROM_RC_TYPE);
}

void MainWindow::setRemoteAddr(int addr)
{
    eep[EEPROM_RC_ADDR] = addr;
    updateHexTable(EEPROM_RC_ADDR);
}

void MainWindow::setRemoteCmd(int cmd)
{
    eep[EEPROM_RC_CMD + lwCommands->currentRow()] = cmd;
    updateHexTable(EEPROM_RC_CMD + lwCommands->currentRow());
}

void MainWindow::setRemoteIndex(int index)
{
    sbxRemoteCmd->setValue(eep[EEPROM_RC_CMD + index]);
}

void MainWindow::setOther()
{
    setBrstby(eep[EEPROM_BR_STBY]);
    sbxBrstby->setValue(eep[EEPROM_BR_STBY]);
}

void MainWindow::setBrstby(int value)
{
    if (value > sbxBrstby->maximum())
        value = 1;
    eep[EEPROM_BR_STBY] = (char)value;
    updateHexTable(EEPROM_BR_STBY);
}

void MainWindow::setLanguage()
{
    QString lang;

    if (actionLangBelarusian->isChecked()) {
        lang = "by";
    } else if (actionLangEnglish->isChecked()) {
        lang = "en";
    } else if (actionLangRussian->isChecked()) {
        lang = "ru";
    } else {
        lang = "auto";
    }

    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

    settings.setValue(SETTINGS_GENERAL_LANGUAGE, lang);

    retranslate(lang);
}

void MainWindow::retranslate(QString lang)
{
    if (lang.compare("auto") == 0) {
        lang = QLocale::system().bcp47Name().remove(QRegExp("-.*"));
    }
    translator.load(":/ts/editor_" + lang);
    qApp->removeTranslator(&translator);
    qApp->installTranslator(&translator);
    retranslateUi(this);
}
