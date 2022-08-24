#ifndef PSTYPES_H
#define PSTYPES_H

#include <cstdint>
#include <vector>


namespace psdw
{
	struct PSDColour
	{
		uint8_t r{};
		uint8_t g{};
		uint8_t b{};
	};

	struct PSDRect
	{
		int x{};
		int y{};
		int w{};
		int h{};
	};

	enum class PSDStatus
	{
		Success,
		FileExistsError,
		FileWriteError,
		NoProfileError,
		InvalidArgument
	};

	enum class PSDChannelOrder
	{
		RGBA,
		BGRA
	};

	enum class PSDCompression
	{
		None,
		RLE
	};
}

namespace psdimpl
{
	struct PSDChannel
	{
		uint16_t compression{};
		std::vector<uint8_t> image_data{};
		std::vector<uint16_t> bytecounts{};
	};

	enum class PSDChannelOrder
	{
		RGBA,
		BGRA,
		RGB
	};
}

#endif