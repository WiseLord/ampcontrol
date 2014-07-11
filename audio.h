#ifndef AUDIO_H
#define AUDIO_H

/* Integral circuits definitions */
#if !defined(TDA7313) && !defined(TDA7318) && !defined(TDA7439)
#define TDA7439
#endif

#if defined(TDA7313)
#include "audio/tda7313.h"
#elif defined(TDA7318)
#include "audio/tda7318.h"
#elif defined(TDA7439)
#include "audio/tda7439.h"
#endif

#endif /* AUDIO_H */
