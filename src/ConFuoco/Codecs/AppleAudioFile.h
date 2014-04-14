// Copyright (c) 2010-14 Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#ifndef CONFUOCO_CODECS_APPLEAUDIOFILE_H_
#define CONFUOCO_CODECS_APPLEAUDIOFILE_H_

#include "Platform/Macros.h"
#define USE_APPLEAUDIOFILE \
    (defined(RAINBOW_OS_IOS) || defined(RAINBOW_OS_MACOS)) \
    && !defined(USE_FMOD_STUDIO)
#if USE_APPLEAUDIOFILE

#include <AudioToolbox/ExtendedAudioFile.h>

#include "ConFuoco/AudioFile.h"

namespace ConFuoco
{
	/// Extended Audio File Services implementation of AudioFile.
	class AppleAudioFile final : public AudioFile
	{
	public:
		AppleAudioFile(const char *const file, const int mode);
		virtual ~AppleAudioFile();

	private:
		AudioStreamBasicDescription format;
		ExtAudioFileRef ref;

		/* Implement AudioFile */

		virtual int channels_impl() const override;
		virtual int rate_impl() const override;
		virtual size_t read_impl(char **dst) override;
		virtual size_t read_impl(char *dst, const size_t size) override;
		virtual void rewind_impl() override;
	};
}

#endif  // USE_APPLEAUDIOFILE
#endif  // CONFUOCO_APPLEAUDIOFILE_H_
