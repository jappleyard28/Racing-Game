#pragma once

#define STBI_ONLY_PNG
#include "types.h"
#include <string>
#include <optional>
#include "glw.h"
#include <stb_image.h>

namespace game::assets
{
	struct ImageData
	{
		ImageData(u8 *data, u32 width, u32 height, u32 channels)
			: m_width(width), m_height(height),
			  m_channels(channels),
			  m_data(data) {}

		~ImageData()
		{
			stbi_image_free(m_data);
		}

		[[nodiscard]] inline u32 pixelAt(u32 x, u32 y) const
		{
			return reinterpret_cast<u32 *>(m_data)[x + m_width * y];
		}

		ImageData(const ImageData &) = delete;

		ImageData &operator=(const ImageData &) = delete;
		ImageData &operator=(ImageData &&) = delete;

		u32 m_width, m_height, m_channels;
		u8 *m_data;
	};

	void addAssetType(const std::string &id, std::string subdir, std::string ext);
	[[nodiscard]] std::optional<std::string> loadText(const std::string &assetType, const std::string &id);
	[[nodiscard]] std::optional<ImageData> loadImage(const std::string &id);
}
