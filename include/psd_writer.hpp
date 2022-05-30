#ifndef PSD_WRITER_H
#define PSD_WRITER_H

#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>


namespace psdw
{
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

	struct PSDColour
	{
		uint8_t r{}, g{}, b{};
	};

	class PSDocument
	{
	public:
		/* Initialises a blank, RGB, 8BPC document.
		doc_width, doc_height and doc_ppi should be between 1 and 30,000. */
		PSDocument(
			const int doc_width,
			const int doc_height,
			const double doc_ppi = 72.0,
			const PSDColour doc_background_rgb = { 255, 255, 255 },
			const std::filesystem::path doc_profile_path = "");

		/* img should be a pointer to an 8BPC band-interleaved-by-pixel colour 
		array in RGBA or BGRA format. layer_x and layer_y refer to the position 
		of the top-left corner of the layer. Compression can either be turned 
		off with None or set to RLE for PackBits run-length encoding. Using 
		RLE will result in a much smaller file for layers with simple graphics 
		but may inflate file size for photographs. */
		PSDStatus add_layer(const unsigned char* img,
			const int img_width, const int img_height,
			const int layer_x, const int layer_y,
			const std::string layer_name,
			PSDChannelOrder channel_order=PSDChannelOrder::BGRA,
			PSDCompression compression=PSDCompression::RLE);

		PSDStatus save(const std::filesystem::path& filename,
			bool overwrite=false);

		PSDStatus status() const { return m_status; }

	private:
		struct Rect
		{
			uint32_t top{}, left{}, bottom{}, right{};
		};

		struct ResolutionInfo
		{
			uint16_t h_res_int{};
			uint16_t h_res_frac{};
			uint16_t h_res_unit{};
			uint16_t width_unit{};
			uint16_t v_res_int{};
			uint16_t v_res_frac{};
			uint16_t v_res_unit{};
			uint16_t height_unit{};
		};

		struct ChannelInfo
		{
			uint16_t channel_id{};
			uint32_t channel_data_length{};
		};

		struct LayerBlendingRanges
		{
			uint8_t src_black_lower{};
			uint8_t src_black_upper{};
			uint8_t src_white_lower{};
			uint8_t src_white_upper{};
			uint8_t dst_black_lower{};
			uint8_t dst_black_upper{};
			uint8_t dst_white_lower{};
			uint8_t dst_white_upper{};
		};

		struct AdditionalLayerInfo
		{
			uint32_t signature{};
			uint32_t key{};
			uint32_t data_length{};
			std::vector<uint8_t> data{};
		};

		struct LayerRecord
		{
			Rect layer_content_rect{};
			uint16_t channel_count{};
			std::vector<ChannelInfo> channel_info{};
			uint32_t blend_mode_signature{};
			uint32_t blend_mode_key{};
			uint8_t opacity{};
			uint8_t clipping{};
			uint8_t flags{};
			uint8_t filler{};
			uint32_t extra_data_length{};
			uint32_t layer_mask_data_length{};
			Rect layer_mask_rect{};
			uint8_t layer_mask_colour{};
			uint8_t layer_mask_flags{};
			uint16_t layer_mask_padding{};
			uint32_t blending_range_length{};
			LayerBlendingRanges grey{};
			LayerBlendingRanges red{};
			LayerBlendingRanges green{};
			LayerBlendingRanges blue{};
			LayerBlendingRanges alpha{};
			std::string layer_name{};
			std::vector<AdditionalLayerInfo> additional_layer_info{};
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

		bool little_endian();

		PSDStatus pack_image(
			const unsigned char* img,
			int img_width,
			int img_height,
			PSDChannelOrder channel_order,
			PSDCompression compression);

		void finalise_pack(std::vector<uint8_t>& buf, uint16_t& bytes);
		void finalise_pack(const uint8_t val, int& reps, uint16_t& bytes);

		// Given a pixel coordinate, retrieve the index of the corresponding
		// element in the band-interleaved-by-pixel 1D array.
		int get_index(int channels, int width, int channel, int x, int y);

		void write(std::ofstream& file, const uint8_t& val) const;
		void write(std::ofstream& file, const uint16_t& val) const;
		void write(std::ofstream& file, const uint32_t& val) const;
		void write(std::ofstream& file, const std::vector<uint8_t>& val) const;
		void write(std::ofstream& file, const std::vector<uint16_t>& val) const;
		void write(std::ofstream& file, const std::string& val) const;
		void write(std::ofstream& file, std::ifstream& val) const;
		void write(std::ofstream& file, const Rect& val) const;
		void write(std::ofstream& file, const LayerBlendingRanges& val) const;

		PSDStatus m_status;
		bool m_little_endian;

		// Header section data.
		const std::string m_file_signature;
		const uint16_t m_version;
		uint16_t m_channel_count;
		uint16_t m_colour_mode;
		uint16_t m_width;
		uint16_t m_height;
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
		std::filesystem::path m_icc_profile_path;

		// Layer and mask section data.
		uint32_t m_layer_and_mask_info_length;
		uint32_t m_layer_info_length;
		uint16_t m_layer_count;

		std::vector<LayerRecord> m_layer_records;

		uint16_t m_channel_image_compression;
		std::vector<uint16_t> m_channel_row_bytecounts;
		std::vector<uint8_t> m_channel_image_data;

		GlobalLayerMaskInfo m_global_layer_mask_info;
		std::vector<AdditionalLayerInfo> m_additional_layer_info{};

		// Image data.
		uint16_t m_image_compression;
		std::vector<uint16_t> m_row_bytecounts;
		std::vector<uint8_t> m_image_data;
	};
}

#endif
