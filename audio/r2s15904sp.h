#ifndef R2S15904SP_H
#define R2S15904SP_H

#define R2S15904SP_IN_CNT 4

#define R2S15904SP_INCTRL       0x0000
#define R2S15904SP_BTCTRL       0x0001
#define R2S15904SP_VOLCTRL      0x0002

#define R2S15904SP_MUTE         0xFFFC

#define R2S15904SP_INPUT        0xE000
#define R2S15904SP_TONE_EN      0x0200

void r2s15904spSetSpeakers();
void r2s15904spSetBT();
void r2s15904spSetInput();
void r2s15904spSetMute();

#endif // R2S15904SP_H
