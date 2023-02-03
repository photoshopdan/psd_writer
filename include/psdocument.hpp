#ifndef PSDOCUMENT_H
#define PSDOCUMENT_H

#include "psdtypes.hpp"
#include "psdwriter.hpp"

#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <memory>

namespace psdw
{
	class PSDocument
	{
	public:
		/* Initialises a blank, RGB, 8BPC document. 
		doc_width and doc_height must be between 1 and 30,000 pixels. */
		PSDocument(int doc_width, int doc_height,
			const PSDColour doc_background_rgb={ 255, 255, 255 });

		/* Set document resolution in pixels per inch. ppi must be between 1 
		and 29,999. */
		PSDStatus set_resolution(double ppi);

		PSDStatus set_profile(std::filesystem::path icc_profile);

		/* img should be a pointer to an 8BPC band-interleaved-by-pixel colour 
		array in RGBA or BGRA format. rect contains the x and y coordinate 
		of the top-left corner of the layer and the actual width and height of 
		the array. Compression can either be turned off with None or set to 
		RLE for PackBits run-length encoding. Using RLE will result in a much 
		smaller file for layers with simple graphics but may inflate file size
		for photographs. */
		PSDStatus add_layer(const unsigned char* img,
			PSDRect rect,
			std::string layer_name,
			PSDChannelOrder channel_order=PSDChannelOrder::BGRA,
			PSDCompression compression=PSDCompression::RLE);

		PSDStatus save(const std::filesystem::path& filename,
			bool overwrite=false);

		PSDStatus status() const { return m_status; }

	private:
		PSDStatus m_status{ PSDStatus::Success };
		psdimpl::PSDData m_data{};
		psdimpl::PSDWriter m_writer{ m_data };
	};
}

#endif
