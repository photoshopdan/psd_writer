// Copyright (c) 2024 Dan Kemp. All rights reserved.
// This source code is licensed under the MIT license found in the 
// LICENSE file in the root directory of this source tree.

#ifndef PSDOCUMENT_H
#define PSDOCUMENT_H

#define DllExport __declspec(dllexport)

#include "psdtypes.hpp"

#include <cstdint>
#include <string>
#include <filesystem>

namespace psdw
{
	class DllExport PSDocument
	{
	public:
		/* Initialises a blank, RGB, 8BPC document. 
		doc_width and doc_height must be between 1 and 30,000 pixels. */
		PSDocument(int doc_width, int doc_height,
			const PSDColour doc_background_rgb={ 255, 255, 255 });

		~PSDocument();
		PSDocument(PSDocument&&) noexcept;
		PSDocument(const PSDocument&) = delete;
		PSDocument& operator=(PSDocument&&) noexcept;
		PSDocument& operator=(const PSDocument&) = delete;

		/* Set document resolution in pixels per inch. ppi must be between 1 
		and 29,999. */
		PSDStatus set_resolution(double ppi);

		PSDStatus set_profile(std::filesystem::path icc_profile);

		PSDStatus add_guide(int position, PSDOrientation orientation);

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
			bool visible=true,
			PSDChannelOrder channel_order=PSDChannelOrder::BGRA,
			PSDCompression compression=PSDCompression::RLE);

		PSDStatus save(const std::filesystem::path& filename,
			bool overwrite=false);

		PSDStatus status() const;

	private:
		// pImpl to simplify DLL interface.
		class PSDocumentImpl;
		PSDocumentImpl* m_psdocument;
	};
}

#endif
