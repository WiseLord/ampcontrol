#include "mainwindow.h"

#include <QDebug>
#include <QtWidgets>
#include <QStringList>

#include "defines.h"
#include "aboutdialog.h"

#include "irseq.h"

#include "audio/audio.h"
#include "eeprom.h"
#include "tuner/tuner.h"
#include "display.h"
#include "remote.h"

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

    QActionGroup *langGroup = new QActionGroup(this);
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
        wgtHexTable->setVerticalHeaderItem(y, new QTableWidgetItem(QString("%1").arg(y * 16, 4, 16,
                                                                                     QChar('0')).toUpper()));
        for (int x = 0; x < 16; x++)
            wgtHexTable->setItem(y, x, new QTableWidgetItem());
    }
    for (int x = 0; x < 16; x++)
        wgtHexTable->setHorizontalHeaderItem(x, new QTableWidgetItem(QString("%1").arg(x, 0,
                                                                                       16).toUpper()));

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
        item->setForeground(QBrush(Qt::gray));
    else if (item->text() == "00" && (pos) >= EEPROM_LABELS_ADDR)
        item->setForeground(QBrush(Qt::blue));
    else
        item->setForeground(QBrush(Qt::black));
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
    int rcType = eep[EEPROM_RC_TYPE];
    if (rcType + 1 >= cbxRemoteType->count())
        rcType = 0;
    setRemote(rcType);

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

void MainWindow::saveAmsrFile(QString name)
{
    QFile file(name);
    if (!file.open(QIODevice::WriteOnly)) {
        Ui_MainWindow::statusBar->showMessage(tr("Can't save") + " " + name);
        return;
    }
    this->fillAmsr();
    file.write(this->amsr.toLocal8Bit());
    file.close();
    Ui_MainWindow::statusBar->showMessage(tr("AMSR saved as") + " " + name);
}

void MainWindow::fillAmsr()
{
    this->amsr.clear();

    this->amsr.append("{\n");
    this->amsr.append("  \"name\":\"AmpControl\",\n");
    this->amsr.append("  \"all_codes\":\n");
    this->amsr.append("  [\n");
    for (int i = 0; i < lwCommands->count(); i++) {
        QString itemName = lwCommands->item(i)->text();

        IrSeq irseq(sbxIrFreq->value(), eep[EEPROM_RC_TYPE]);
        QString code1 = irseq.getSequence(eep[EEPROM_RC_ADDR], eep[EEPROM_RC_CMD + i], 1);
        QString code2 = irseq.getSequence(eep[EEPROM_RC_ADDR], eep[EEPROM_RC_CMD + i], 2);

        this->amsr.append("    {\n");
        this->amsr.append("      \"function\":\"" + itemName + "\",\n");
        this->amsr.append("      \"code1\":\"" + code1 + "\"");
        if ((int)eep[EEPROM_RC_TYPE] == IR_TYPE_RC5) {
            this->amsr.append(",\n");
            this->amsr.append("      \"code2\":\"" + code2 + "\"\n");
        } else {
            this->amsr.append("\n");
        }
        if (i == lwCommands->count() - 1)
            this->amsr.append("    }\n");
        else
            this->amsr.append("    },\n");
    }
    this->amsr.append("  ]\n");
    this->amsr.append("}\n");
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

void MainWindow::saveAmsr()
{
    QString name = QFileDialog::getSaveFileName(this,
                                                tr("Save AMSR file"),
                                                "./AmpControl.amsr",
                                                tr("AMSR files (*.amsr)"));
    if (name.isEmpty())
        return;

    saveAmsrFile(name);
}

void MainWindow::updateTranslation(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

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
    wgtToneBypass->hide();
    wgtInput->hide();
    wgtInputIcon->hide();
    cbxInput->clear();

    /* Handle loudness/surround/effect3d/tone_bypass*/
    switch (proc) {
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_TDA7314:
    case AUDIOPROC_TDA7315:
    case AUDIOPROC_PT2314:
    case AUDIOPROC_TUNER_IC:
        wgtLoudness->show();
        cbxLoudness->setCurrentIndex(!!(eep[EEPROM_APROC_EXTRA] & APROC_EXTRA_LOUDNESS));
        break;
    case AUDIOPROC_PT232X:
        wgtSurround->show();
        cbxSurround->setCurrentIndex(!!(eep[EEPROM_APROC_EXTRA] & APROC_EXTRA_SURROUND));
        wgtEffect3d->show();
        cbxEffect3d->setCurrentIndex(!!(eep[EEPROM_APROC_EXTRA] & APROC_EXTRA_EFFECT3D));
    case AUDIOPROC_R2S15904SP:
        wgtToneBypass->show();
        cbxToneBypass->setCurrentIndex(!!(eep[EEPROM_APROC_EXTRA] & APROC_EXTRA_TONE_BYPASS));
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
    case AUDIOPROC_R2S15904SP:
        cbxInput->insertItem(0, "Input 4");
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_TEA6300:
        cbxInput->insertItem(0, "Input 3");
        cbxInput->insertItem(0, "Input 2");
    default:
        cbxInput->insertItem(0, "Input 1");
        wgtInput->show();
        wgtInputIcon->show();
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
    case AUDIOPROC_TUNER_IC:
        wgtVolume->show();
        setAudioParam(dsbVolume, 0, 16, 1, MODE_SND_VOLUME);
        break;
    case AUDIOPROC_R2S15904SP:
        wgtVolume->show();
        setAudioParam(dsbVolume, -79, 0, 1, MODE_SND_VOLUME);
        wgtBass->show();
        setAudioParam(dsbBass, -16, 16, 2, MODE_SND_BASS);
        wgtTreble->show();
        setAudioParam(dsbTreble, -16, 16, 2, MODE_SND_TREBLE);
        wgtBalance->show();
        setAudioParam(dsbBalance, -15, 15, 1, MODE_SND_BALANCE);
        break;
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
    cbxInputIcon->setCurrentIndex(eep[EEPROM_INPUT_ICONS + eep[EEPROM_INPUT]]);
}

void MainWindow::setInputIcon(int value)
{
    if (value >= cbxInputIcon->count())
        value = cbxInputIcon->count() - 1;
    eep[EEPROM_INPUT_ICONS + eep[EEPROM_INPUT]] = (char)value;
    updateHexTable(EEPROM_INPUT_ICONS + eep[EEPROM_INPUT]);
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

void MainWindow::setToneBypass(int value)
{
    if (value)
        eep[EEPROM_APROC_EXTRA] = eep[EEPROM_APROC_EXTRA] | APROC_EXTRA_TONE_BYPASS;
    else
        eep[EEPROM_APROC_EXTRA] = eep[EEPROM_APROC_EXTRA] & ~APROC_EXTRA_TONE_BYPASS;
    updateHexTable(EEPROM_APROC_EXTRA);
}

// Save/delete station from eeprom
void MainWindow::stationAddRemove()
{
    uint8_t i, j;
    uint16_t freqCell;
    uint16_t freq;

    freq = (dsbCurfreq->value() + 0.001) * 100;

    for (i = 0; i < FM_COUNT; i++) {
        freqCell = (uint8_t)eep[EEPROM_STATIONS + 2 * i] | ((uint8_t)eep[EEPROM_STATIONS + 2 * i + 1] << 8);
        if (freqCell < freq)
            continue;
        if (freqCell == freq) {
            for (j = i; j < FM_COUNT; j++) {
                if (j == FM_COUNT - 1) {
                    freqCell = 0xFFFF;
                } else {
                    freqCell = (uint8_t)eep[EEPROM_STATIONS + 2 * (j + 1)] | ((uint8_t)eep[EEPROM_STATIONS + 2 *
                                                                                           (j + 1) + 1] << 8);
                }
                eep[EEPROM_STATIONS + 2 * j] = (char)(freqCell & 0x00FF);
                eep[EEPROM_STATIONS + 2 * j + 1] = (char)(freqCell >> 8);
            }
            break;
        } else {
            for (j = i; j < FM_COUNT; j++) {
                freqCell = (uint8_t)eep[EEPROM_STATIONS + 2 * j] | ((uint8_t)eep[EEPROM_STATIONS + 2 * j + 1] << 8);
                eep[EEPROM_STATIONS + 2 * j] = (char)(freq & 0x00FF);
                eep[EEPROM_STATIONS + 2 * j + 1] = (char)(freq >> 8);
                freq = freqCell;
            }
            break;
        }
    }
    updateHexTable();
    fillStations();
    setCurFreq(dsbCurfreq->value());
}

void MainWindow::stationRemoveAll()
{
    for (uint8_t i = 0; i < FM_COUNT; i++) {
        eep[EEPROM_STATIONS + 2 * i] = 0xFF;
        eep[EEPROM_STATIONS + 2 * i + 1] = 0xFF;
    }
    updateHexTable();
    fillStations();
    setCurFreq(dsbCurfreq->value());
}

void MainWindow::setStationIndex(int index)
{
    if (lwStations->item(index)) {
        double freq = lwStations->item(index)->text().toDouble();
        dsbCurfreq->setValue(freq);
    }
}

void MainWindow::setCurFreq(double value)
{
    if (value < 76)
        dsbCurfreq->setSingleStep((double)fmStepIndex2Step(cbxFmstep1->currentIndex()) / 100);
    else
        dsbCurfreq->setSingleStep((double)fmStepIndex2Step(cbxFmstep2->currentIndex()) / 100);
    curFreq = value;

    for (int i = 0; i < lwStations->count(); i++) {
        if (lwStations->item(i)->text() == QString::number(curFreq, 'f', 2)) {
            lwStations->setCurrentRow(i);
            pbStationAddRemove->setText(tr("Remove station"));
            return;
        }
    }

    lwStations->clearSelection();
    pbStationAddRemove->setText(tr("Add station"));
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
    int freq = (value + 0.001) * 100;

    eep[pos] = (char)(freq & 0x00FF);
    eep[pos + 1] = (char)((freq & 0xFF00) >> 8);

    updateHexTable(pos);
    updateHexTable(pos + 1);
}

void MainWindow::fillStations()
{
    lwStations->clear();
    for (int i = 0; i < FM_COUNT; i++) {
        double freq = getFreq(EEPROM_STATIONS + i * 2);
        if (freq < 655) { // 0xFFFF
            lwStations->addItem(QString::number(freq, 'f', 2));
        }
    }
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
    wgtFmBass->hide();

    cbxFmctrlDfreq->setChecked(eep[EEPROM_FM_CTRL] & TUNER_DFREQ);
    cbxFmctrlSnc->setChecked(eep[EEPROM_FM_CTRL] & TUNER_SNC);
    cbxFmctrlHcc->setChecked(eep[EEPROM_FM_CTRL] & TUNER_HCC);
    cbxFmctrlSm->setChecked(eep[EEPROM_FM_CTRL] & TUNER_SMUTE);
    cbxFmctrlXtal->setChecked(eep[EEPROM_FM_CTRL] & TUNER_XTAL);
    cbxFmctrlBl->setChecked(eep[EEPROM_FM_CTRL] & TUNER_BL);
    cbxFmctrlDe->setChecked(eep[EEPROM_FM_CTRL] & TUNER_DE);
    cbxFmctrlPllref->setChecked(eep[EEPROM_FM_CTRL] & TUNER_PLLREF);

    wgtFmctrl->hide();
    cbxFmctrlDfreq->hide();
    cbxFmctrlSnc->hide();
    cbxFmctrlHcc->hide();
    cbxFmctrlSm->hide();
    cbxFmctrlXtal->hide();
    cbxFmctrlBl->hide();
    cbxFmctrlDe->hide();
    cbxFmctrlPllref->hide();

    switch (tuner) {
    case TUNER_TEA5767:
        wgtFmctrl->show();
        cbxFmctrlHcc->show();
        cbxFmctrlSnc->show();
        cbxFmctrlSm->show();
        cbxFmctrlDe->show();
        cbxFmctrlBl->show();
        cbxFmctrlPllref->show();
        cbxFmctrlXtal->show();
    case TUNER_RDA5807:
    case TUNER_RDA5802:
    case TUNER_SI470X:
        if (tuner != TUNER_TEA5767) {
            wgtFmctrl->show();
            wgtFmRDS->show();
            setFmRds(eep[EEPROM_FM_RDS]);
            cbxFmRDS->setCurrentIndex(eep[EEPROM_FM_RDS]);
            if (tuner == TUNER_RDA5807) {
                cbxFmctrlDfreq->show();
            }
            cbxFmctrlSm->show();
            cbxFmctrlDe->show();
            cbxFmctrlBl->show();
            if (tuner == TUNER_RDA5802 || tuner == TUNER_RDA5807) {
                wgtFmBass->show();
                setFmBass(eep[EEPROM_FM_BASS]);
                cbxFmBass->setCurrentIndex(eep[EEPROM_FM_BASS]);
            }
        }
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
        dsbCurfreq->setValue(getFreq(EEPROM_FM_FREQ));
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

    fillStations();
    setCurFreq(getFreq(EEPROM_FM_FREQ));
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

void MainWindow::setFmBass(int value)
{
    if (value)
        eep[EEPROM_FM_BASS] = 0x01;
    else
        eep[EEPROM_FM_BASS] = 0x00;
    updateHexTable(EEPROM_FM_BASS);
}

void MainWindow::setFmctrl()
{
    char ctrl = 0;

    if (cbxFmctrlDfreq->isChecked()) ctrl |= TUNER_DFREQ;
    if (cbxFmctrlHcc->isChecked()) ctrl |= TUNER_HCC;
    if (cbxFmctrlSnc->isChecked()) ctrl |= TUNER_SNC;
    if (cbxFmctrlSm->isChecked()) ctrl |= TUNER_SMUTE;
    if (cbxFmctrlDe->isChecked()) ctrl |= TUNER_DE;
    if (cbxFmctrlBl->isChecked()) ctrl |= TUNER_BL;
    if (cbxFmctrlPllref->isChecked()) ctrl |= TUNER_PLLREF;
    if (cbxFmctrlXtal->isChecked()) ctrl |= TUNER_XTAL;

    eep[EEPROM_FM_CTRL] = ctrl;
    updateHexTable(EEPROM_FM_CTRL);
}

void MainWindow::setRemote(int rcType)
{
    setRemoteType(rcType);
    cbxRemoteType->setCurrentIndex(rcType);

    setRemoteAddr(eep[EEPROM_RC_ADDR]);
    sbxRemoteAddr->setValue(eep[EEPROM_RC_ADDR]);

    lwCommands->setCurrentRow(0);
    sbxRemoteCmd->setValue(eep[EEPROM_RC_CMD]);
}

void MainWindow::setRemoteType(int type)
{
    eep[EEPROM_RC_TYPE] = type;
    updateHexTable(EEPROM_RC_TYPE);
    calcRemoteSeq();
}

void MainWindow::setRemoteAddr(int addr)
{
    eep[EEPROM_RC_ADDR] = addr;
    updateHexTable(EEPROM_RC_ADDR);
    calcRemoteSeq();
}

void MainWindow::setRemoteCmd(int cmd)
{
    eep[EEPROM_RC_CMD + lwCommands->currentRow()] = cmd;
    updateHexTable(EEPROM_RC_CMD + lwCommands->currentRow());
    calcRemoteSeq();
}

void MainWindow::setRemoteIndex(int index)
{
    sbxRemoteCmd->setValue(eep[EEPROM_RC_CMD + index]);
}

void MainWindow::calcRemoteSeq()
{
    IrSeq irseq(sbxIrFreq->value(), cbxRemoteType->currentIndex());

    QString out = irseq.getSequence(sbxRemoteAddr->value(), sbxRemoteCmd->value(), 1);
    out.append("\n");
    if ((int)eep[EEPROM_RC_TYPE] == IR_TYPE_RC5) {
        out.append(irseq.getSequence(sbxRemoteAddr->value(), sbxRemoteCmd->value(), 2));
    }

    teIrSeq->setText(out);
}

void MainWindow::setOther()
{
    setSpmode(eep[EEPROM_SP_MODE]);
    cbxSpmode->setCurrentIndex(eep[EEPROM_SP_MODE]);

    setSpspeed(eep[EEPROM_FALL_SPEED]);
    cbxSpspeed->setCurrentIndex(eep[EEPROM_FALL_SPEED]);

    setBrstby(eep[EEPROM_BR_STBY]);
    sbxBrstby->setValue(eep[EEPROM_BR_STBY]);

    setEncres(eep[EEPROM_ENC_RES]);
    sbxEncres->setValue(eep[EEPROM_ENC_RES]);

    setExtfunc(eep[EEPROM_EXT_FUNC]);
    cbxExtfunc->setCurrentIndex(eep[EEPROM_EXT_FUNC]);

    setThreshold(eep[EEPROM_TEMP_TH]);
    sbxThreshold->setValue(eep[EEPROM_TEMP_TH]);

    setSilence((unsigned char)eep[EEPROM_SILENCE_TIMER]);
    sbxSilence->setValue((unsigned char)eep[EEPROM_SILENCE_TIMER]);

    setInitMode(((unsigned char)eep[EEPROM_INIT_MODE] < 2) ? 1 : 0);
    cbxInitMode->setCurrentIndex(((unsigned char)eep[EEPROM_INIT_MODE]) < 2 ? 1 : 0);
}

void MainWindow::setSpmode(int value)
{
    if (value >= SP_MODE_END)
        value = SP_MODE_STEREO;
    eep[EEPROM_SP_MODE] = (char)value;
    updateHexTable(EEPROM_SP_MODE);
}

void MainWindow::setSpspeed(int value)
{
    if (value >= FALL_SPEED_END)
        value = FALL_SPEED_LOW;
    eep[EEPROM_FALL_SPEED] = (char)value;
    updateHexTable(EEPROM_FALL_SPEED);
}

void MainWindow::setBrstby(int value)
{
    if (value > sbxBrstby->maximum())
        value = 1;
    eep[EEPROM_BR_STBY] = (char)value;
    updateHexTable(EEPROM_BR_STBY);
}

void MainWindow::setEncres(int value)
{
    if (value > sbxEncres->maximum())
        value = sbxEncres->maximum();
    if (value < sbxEncres->minimum())
        value = sbxEncres->minimum();
    eep[EEPROM_ENC_RES] = (char)value;
    updateHexTable(EEPROM_ENC_RES);
}

void MainWindow::setExtfunc(int value)
{
    if (value >= cbxExtfunc->count())
        value = 0;
    eep[EEPROM_EXT_FUNC] = (char)value;
    updateHexTable(EEPROM_EXT_FUNC);
}

void MainWindow::setThreshold(int value)
{
    if (value > sbxThreshold->maximum())
        value = sbxThreshold->maximum();
    if (value < sbxThreshold->minimum())
        value = sbxThreshold->minimum();
    eep[EEPROM_TEMP_TH] = (char)value;
    updateHexTable(EEPROM_TEMP_TH);
}

void MainWindow::setSilence(int value)
{
    if (value > sbxSilence->maximum())
        value = sbxSilence->maximum();
    if (value < sbxSilence->minimum())
        value = sbxSilence->minimum();
    eep[EEPROM_SILENCE_TIMER] = (unsigned char)value;
    updateHexTable(EEPROM_SILENCE_TIMER);

}

void MainWindow::setInitMode(int value)
{
    eep[EEPROM_INIT_MODE] = (char)(value ? 0x00 : 0xFF);
    updateHexTable(EEPROM_INIT_MODE);
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

    for (int i = 0; i < lwStations->count(); i++) {
        if (lwStations->item(i)->text() == QString::number(curFreq, 'f', 2)) {
            pbStationAddRemove->setText(tr("Remove station"));
            return;
        }
    }
    pbStationAddRemove->setText(tr("Add station"));
}
