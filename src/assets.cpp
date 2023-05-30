#define STB_IMAGE_IMPLEMENTATION
#include "assets.h"
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <sstream>

namespace game::assets
{
	namespace
	{
        //m_subdir is either 'shaders' or 'textures' and m_ext is the extension
		struct AssetType
		{
			std::string m_subdir, m_ext;
		};

		std::unordered_map<std::string, AssetType> s_assetTypes;
	}

	void addAssetType(const std::string &id, std::string subdir, std::string ext)
	{
		s_assetTypes.insert({ id, { std::move(subdir), std::move(ext) }});
	}


	std::optional<std::string> loadText(const std::string &assetType, const std::string &id)
	{
	    //tries to find the asset type
		if(const auto iter = s_assetTypes.find(assetType); iter != s_assetTypes.end())
		{
			const auto &type = iter->second;

			std::ostringstream filename; // stringbuilder for filename
			filename << "assets/" << type.m_subdir << '/' << id << '.' << type.m_ext;

			std::ifstream in(filename.str(), std::ios::in | std::ios::binary);

			if(in)
			{
				std::string data;

				in.seekg(0, std::ios::end); //moves the stream position to the end of the file
				data.resize(in.tellg()); //gets the position of the file so it resizes the data to the size of the file
				in.seekg(0, std::ios::beg); //moves the stream back to the beginning to read

				in.read(data.data(), static_cast<std::streamsize>(data.size())); //reads all of the stream into data

				return { std::move(data) }; //returns data
			}
			else
			{
				std::cerr << "Failed to load " << assetType << " asset \"" << id << '\"' << std::endl;
				return {};
			}
		}
		else
		{
			std::cerr << "No such asset type '" << assetType << "'" << std::endl;
			return {};

		}
	}

	//loads a texture and returns an imagedata struct defined in assets.h
	std::optional<ImageData> loadImage(const std::string &id)
	{
		std::ostringstream filename;
		filename << "assets/textures/" << id << ".png";

		i32 width, height, channels;
		u8 *image = stbi_load(filename.str().c_str(), &width, &height, &channels, 0);

		if(image) return std::optional<ImageData>(std::in_place, image, static_cast<u32>(width), static_cast<u32>(height), static_cast<u32>(channels));
		else
		{
			std::cerr << "Failed to load texture asset \"" << id << '\"' << std::endl;
			return {};
		}
	}
}
