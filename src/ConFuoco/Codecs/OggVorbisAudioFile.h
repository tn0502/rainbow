// Copyright (c) 2010-14 Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#ifndef CONFUOCO_CODECS_OGGVORBISAUDIOFILE_H_
#define CONFUOCO_CODECS_OGGVORBISAUDIOFILE_H_

#include "Platform/Macros.h"
#define USE_OGGVORBISAUDIOFILE \
    !defined(RAINBOW_JS) && defined(RAINBOW_SDL) && !defined(USE_FMOD_STUDIO)
#if USE_OGGVORBISAUDIOFILE

#ifdef RAINBOW_OS_MACOS
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#include <vorbis/vorbisfile.h>
#ifdef RAINBOW_OS_MACOS
#	pragma GCC diagnostic pop
#endif

#include "ConFuoco/AudioFile.h"

namespace ConFuoco
{
	/// Ogg Vorbis implementation of AudioFile.
	class OggVorbisAudioFile final : public AudioFile
	{
	public:
		OggVorbisAudioFile(File &&file);
		virtual ~OggVorbisAudioFile();

	private:
		OggVorbis_File vf;
		vorbis_info *vi;

		/* Implement AudioFile */

		virtual int channels_impl() const override;
		virtual int rate_impl() const override;
		virtual size_t read_impl(char **dst) override;
		virtual size_t read_impl(char *dst, const size_t size) override;
		virtual void rewind_impl() override;
	};
}

#endif  // USE_OGGVORBISAUDIOFILE
#endif  // CONFUOCO_OGGVORBISAUDIOFILE_H_
