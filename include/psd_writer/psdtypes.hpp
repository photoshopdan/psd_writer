#ifndef PSDTYPES_H
#define PSDTYPES_H

#include <cstdint>

// User accessible types.
namespace psdw
{
	struct PSDColour
	{
		uint8_t r{}, g{}, b{};
	};

	struct PSDRect
	{
		int x{}, y{}, w{}, h{};
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

// Internal types.
namespace psdimpl
{
	enum class ChannelOrder
	{
		RGBA,
		BGRA,
		RGB
	};
}

#endif
