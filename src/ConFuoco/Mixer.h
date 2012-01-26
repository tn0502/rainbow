#ifndef CONFUOCO_MIXER_H_
#define CONFUOCO_MIXER_H_

#include "Platform.h"
#if defined(RAINBOW_ANDROID)
#	include "ConFuoco/impl/Mixer_Android.h"
#elif defined(RAINBOW_IOS) || defined(RAINBOW_SDL)
#	include "ConFuoco/impl/Mixer_AL.h"
#else
#	error "Unknown platform"
#endif

#endif
