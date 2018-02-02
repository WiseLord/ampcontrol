#ifndef TDA7439_H
#define TDA7439_H

#include <inttypes.h>

#define TDA7439_ADDR            0b10001000

#define CHAN_CNT                4

// TDA7439 I2C function selection
#define TDA7439_INPUT_SELECT    0x00
#define TDA7439_INPUT_GAIN      0x01
#define TDA7439_PREAMP          0x02
#define TDA7439_BASS            0x03
#define TDA7439_MIDDLE          0x04
#define TDA7439_TREBLE          0x05
#define TDA7439_VOLUME_RIGHT    0x06
#define TDA7439_VOLUME_LEFT     0x07
// I2c autoincrement flag
#define TDA7439_AUTO_INC        0x10

#define MUTE_ON                 1
#define MUTE_OFF                0

typedef struct {
    int8_t value;
    int8_t min;
    int8_t max;
    uint8_t step;
    uint8_t *label;
    void (*set)(int8_t value);
} sndParam;

enum {
    SND_VOLUME,
    SND_BASS,
    SND_MIDDLE,
    SND_TREBLE,
    SND_PREAMP,
    SND_BALANCE,
    SND_GAIN0,
    SND_GAIN1,
    SND_GAIN2,
    SND_GAIN3
};

#define SND_PARAM_COUNT         10

sndParam *sndParAddr(uint8_t index);

uint8_t getChan(void);
uint8_t getMute(void);

void changeParam(sndParam *sndPar, int8_t diff);

void setChan(uint8_t ch);
void nextChan(void);

void muteVolume(void);
void unmuteVolume(void);

void switchMute(void);

void loadAudioParams(uint8_t **txtLabels);
void setAudioParams(void);
void saveAudioParams(void);

#endif // TDA7439_H
