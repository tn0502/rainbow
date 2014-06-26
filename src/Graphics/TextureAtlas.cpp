// Copyright (c) 2010-14 Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#include "Graphics/TextureAtlas.h"

#include "Graphics/Image.h"

namespace
{
#if 0  // Currently unused.
	/// Return \c true if the integer is greater than 64 and divisible by 4.
	bool is_valid(const unsigned int i)
	{
		return (i < 64) ? false : (i & 0x03) == 0;
	}
#endif
}

TextureAtlas::TextureAtlas(const DataMap &img) : name_(0), width_(0), height_(0)
{
	R_ASSERT(img, "Failed to load texture");

	const Rainbow::Image &image = Rainbow::Image::decode(img);
	if (!image.data)
		return;

#ifndef NDEBUG
	// Ensure texture dimension is supported by the hardware
	int max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
	R_ASSERT(image.width <= static_cast<unsigned int>(max_texture_size)
	         && image.height <= static_cast<unsigned int>(max_texture_size),
	         "Texture dimension exceeds max texture size supported by hardware");
	//R_ASSERT(is_valid(image.width) && is_valid(image.height),
	//         "Texture dimension must be greater than 64 and divisible by 4");
#endif

	width_ = image.width;
	height_ = image.height;

	switch (image.format)
	{
	#ifdef GL_OES_compressed_ETC1_RGB8_texture
		case Rainbow::Image::Format::ETC1:
			name_ = TextureManager::Instance->create_compressed(
			    GL_ETC1_RGB8_OES, image.width, image.height, image.size,
			    image.data);
			break;
	#endif  // ETC1
	#ifdef GL_IMG_texture_compression_pvrtc
		case Rainbow::Image::Format::PVRTC: {
			R_ASSERT(image.depth == 2 || image.depth == 4,
			         "Invalid colour depth");
			R_ASSERT(image.channels == 3 || image.channels == 4,
			         "Invalid number of colour channels");
			GLint internal = 0;
			if (image.channels == 3)
			{
				internal = (image.depth == 2)
				    ? GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
				    : GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
			}
			else
			{
				internal = (image.depth == 2)
				    ? GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
				    : GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
			}
			name_ = TextureManager::Instance->create_compressed(
			    internal, image.width, image.height, image.size, image.data);
			break;
		}
	#endif  // PVRTC
		default: {
			GLint format = 0;
			GLint internal = 0;
			switch (image.channels)
			{
				case 1:
					R_ASSERT(image.depth == 8, "Invalid colour depth");
					format = GL_LUMINANCE;
					internal = GL_LUMINANCE;
					break;
				case 2:
					R_ASSERT(image.depth == 16, "Invalid colour depth");
					format = GL_LUMINANCE_ALPHA;
					internal = GL_LUMINANCE_ALPHA;
					break;
				case 3:
					R_ASSERT(image.depth == 16 || image.depth == 24,
					         "Invalid colour depth");
					format = GL_RGB;
					internal = (image.depth == 16) ? GL_RGBA4 : GL_RGBA8;
					break;
				case 4:
					R_ASSERT(image.depth == 16 || image.depth == 32,
					         "Invalid colour depth");
					format = GL_RGBA;
					internal = (image.depth == 16) ? GL_RGBA4 : GL_RGBA8;
					break;
			}
			name_ = TextureManager::Instance->create(
			    internal, width_, height_, format, image.data);
			break;
		}
	}
	Rainbow::Image::release(image);
}

unsigned int TextureAtlas::define(const Vec2i &origin, const int w, const int h)
{
	R_ASSERT(origin.x >= 0 && (origin.x + w) <= width_ &&
	         origin.y >= 0 && (origin.y + h) <= height_,
	         "Invalid dimensions");

	const Vec2f v0(origin.x / static_cast<float>(width_),
	               origin.y / static_cast<float>(height_));
	const Vec2f v1((origin.x + w) / static_cast<float>(width_),
	               (origin.y + h) / static_cast<float>(height_));

	const size_t i = regions_.size();
	regions_.emplace_back(v0, v1);
	regions_[i].atlas = name_;
	return i;
}
