#ifndef PSDOCUMENT_H
#define PSDOCUMENT_H

#include "psdtypes.hpp"
#include "psdimage.hpp"

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
		doc_width and doc_height should be between 1 and 30,000. */
		PSDocument(int doc_width, int doc_height,
			const PSDColour doc_background_rgb={ 255, 255, 255 });

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
		struct LayerRect
		{
			uint32_t top{}, left{}, bottom{}, right{};
		};

		struct ResolutionInfo
		{
			uint16_t h_res_int{}; // Fixed-point integer component.
			uint16_t h_res_frac{}; // Fixed-point fractional component.
			uint16_t h_res_unit{}; // 1=PPI, 2=PPCM.
			uint16_t width_unit{}; // 1=in, 2=cm, 3=pt, 4=picas, 5=cols.
			uint16_t v_res_int{};
			uint16_t v_res_frac{};
			uint16_t v_res_unit{};
			uint16_t height_unit{};
		};

		struct ChannelInfo
		{
			int16_t channel_id{};
			uint32_t channel_data_length{};
		};

		struct LayerBlendingRanges
		{
			uint8_t src_black_lower{ 0 };
			uint8_t src_black_upper{ 0 };
			uint8_t src_white_lower{ 255 };
			uint8_t src_white_upper{ 255 };
			uint8_t dst_black_lower{ 0 };
			uint8_t dst_black_upper{ 0 };
			uint8_t dst_white_lower{ 255 };
			uint8_t dst_white_upper{ 255 };
		};

		struct AdditionalLayerInfo
		{
			std::string signature{};
			std::string key{};
			uint32_t data_length{};
			std::vector<uint8_t> data{};
		};

		struct LayerRecord
		{
			LayerRecord(bool little_endian, int layer_number,
				std::string layer_name);

			void populate_luni(std::string layer_name);
			void populate_lyid(int layer_number);
			void populate_cust();

			bool m_little_endian{};

			LayerRect layer_content_rect{};
			uint16_t channel_count{};
			std::vector<ChannelInfo> channel_info{};
			std::string blend_mode_signature{ "8BIM" };
			std::string blend_mode_key{ "norm" };
			uint8_t opacity{ 255 };
			uint8_t clipping{ 0 };
			uint8_t flags{ 9 };
			uint8_t filler{ 0 };
			uint32_t extra_data_length{};
			uint32_t layer_mask_data_length{ 0 };
			LayerRect layer_mask_rect{};
			uint8_t layer_mask_colour{};
			uint8_t layer_mask_flags{};
			uint16_t layer_mask_padding{};
			uint32_t blending_range_length{ 40 };
			LayerBlendingRanges grey{};
			LayerBlendingRanges red{};
			LayerBlendingRanges green{};
			LayerBlendingRanges blue{};
			LayerBlendingRanges alpha{};
			std::string layer_name{};
			AdditionalLayerInfo unicode_layer_name{};
			AdditionalLayerInfo layer_id{ "8BIM", "lyid" };
			AdditionalLayerInfo blend_clipping_elements{ "8BIM", "clbl" };
			AdditionalLayerInfo blend_interior_elements{ "8BIM", "infx" };
			AdditionalLayerInfo knockout_setting{ "8BIM", "knko" };
			AdditionalLayerInfo protected_setting{ "8BIM", "lspf" };
			AdditionalLayerInfo sheet_color_setting{ "8BIM", "lclr" };
			AdditionalLayerInfo metadata_setting{ "8BIM", "shmd" };
			AdditionalLayerInfo cust{};
			AdditionalLayerInfo reference_point{ "8BIM", "fxrp" };
		};

		struct GlobalLayerMaskInfo
		{
			uint32_t length{};
			uint32_t overlay_colour_space{};
			uint16_t colour_comp_1{};
			uint16_t colour_comp_2{};
			uint16_t colour_comp_3{};
			uint16_t colour_comp_4{};
			uint16_t opacity{};
			uint8_t kind{};
			uint16_t filler{};
		};

		struct ImageChannel
		{
			uint16_t channel_compression{};
			std::vector<uint16_t> channel_bytecounts{};
			std::vector<uint8_t> channel_image_data{};
		};

		struct Image
		{
			std::vector<ImageChannel> channels{};
		};

		bool little_endian();

		void write(std::ofstream& file, const uint8_t& val) const;
		void write(std::ofstream& file, const uint16_t& val) const;
		void write(std::ofstream& file, const int16_t& val) const;
		void write(std::ofstream& file, const uint32_t& val) const;
		void write(std::ofstream& file, const std::vector<uint8_t>& val) const;
		void write(std::ofstream& file, const std::vector<uint16_t>& val) const;
		void write(std::ofstream& file, const std::string& val,
			bool pascal_string=false) const;
		void write(std::ofstream& file, const LayerRect& val) const;
		void write(std::ofstream& file, const LayerBlendingRanges& val) const;

		PSDStatus m_status;
		bool m_little_endian;

		// Header section data.
		const std::string m_file_signature;
		const uint16_t m_version;
		uint16_t m_channel_count;
		uint16_t m_colour_mode;
		uint32_t m_width;
		uint32_t m_height;
		uint16_t m_depth;

		// Colour mode section data.
		uint32_t m_colour_mode_data_length;

		// Image resources section data.
		uint32_t m_image_resources_length;
		const std::string m_resource_signature;
		const uint16_t m_null_name;

		uint16_t m_resolution_info_uid;
		uint32_t m_resolution_info_length;
		ResolutionInfo m_resolution;

		uint16_t m_icc_profile_uid;
		std::string m_icc_profile;

		// Layer and mask section data.
		uint32_t m_layer_and_mask_info_length;
		uint32_t m_layer_info_length;
		uint16_t m_layer_count;

		std::vector<LayerRecord> m_layer_records;

		// Band-sequential, possibly compressed, layered image data.
		std::vector<std::unique_ptr<psdimpl::PSDImage>> m_layer_image_data;

		GlobalLayerMaskInfo m_global_layer_mask_info;
		std::vector<AdditionalLayerInfo> m_additional_layer_info;

		// Band-sequential, uncompressed, merged image data.
		// A compressed version will be created on save.
		psdimpl::PSDRawImage m_merged_image_data;
	};
}

#endif
