#ifndef AUDIO_H
#define AUDIO_H

/* Integral circuits definitions */
#if !defined(TDA7313) && !defined(TDA7318) && !defined(TDA7439)
#define TDA7313
#endif

#if defined(TDA7313)
#include "tda7313.h"
#elif defined(TDA7318)
#include "tda7318.h"
#elif defined(TDA7439)
#include "tda7439.h"
#endif

#endif /* AUDIO_H */
