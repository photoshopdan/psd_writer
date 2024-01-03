#ifndef PSDDATA_H
#define PSDDATA_H

#include "psdimage.hpp"

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

namespace psdimpl
{
	struct LayerRect
	{
		uint32_t top{}, left{}, bottom{}, right{};
	};

	struct ChannelInfo
	{
		int16_t id{};
		uint32_t length{};
	};

	struct PascalString
	{
		PascalString(std::string i_string);
		uint32_t length() const;

		std::string string;
		std::vector<uint8_t> buffer;
	};

	struct LayerMaskData
	{
		// If no layer mask is in use, this is empty.
		bool active{ false };
		uint32_t length() const;
		LayerRect rect{};
		uint8_t colour{};
		uint8_t flags{};
		uint8_t parameters{};
		uint16_t padding{};
		uint8_t real_flags{};
		uint8_t real_user_mask_background{};
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

	struct AdditionalLayerInfoBase
	{
		AdditionalLayerInfoBase(std::string i_key);
		virtual uint32_t length() const = 0;

		const std::string signature;
		std::string key;
	};

	struct AdditionalLayerInfo : public AdditionalLayerInfoBase
	{
		AdditionalLayerInfo(std::string i_key,
			std::vector<uint8_t> i_data = {});
		virtual uint32_t length() const override;

		std::vector<uint8_t> data;
	};

	struct AdditionalLayerInfoLuni : public AdditionalLayerInfo
	{
		AdditionalLayerInfoLuni(std::string l_name);
		virtual uint32_t length() const override;

		uint32_t string_length{};
	};

	struct AdditionalLayerInfoLnsr : public AdditionalLayerInfoBase
	{
		AdditionalLayerInfoLnsr(std::string kword);
		virtual uint32_t length() const override;

		std::string keyword{};
	};

	struct AdditionalLayerInfoLyid : public AdditionalLayerInfoBase
	{
		AdditionalLayerInfoLyid(uint32_t l_number);
		virtual uint32_t length() const override;

		uint32_t data{};
	};

	struct AdditionalLayerInfoShmd : public AdditionalLayerInfo
	{
		AdditionalLayerInfoShmd();
	};

	struct AdditionalLayerInfoCust : public AdditionalLayerInfo
	{
		AdditionalLayerInfoCust();
		virtual uint32_t length() const override;

		double time{};
	};

	struct AdditionalLayerInfoCinf : public AdditionalLayerInfo
	{
		AdditionalLayerInfoCinf();
	};

	struct AdditionalLayerInfoFxrp : public AdditionalLayerInfoBase
	{
		AdditionalLayerInfoFxrp();
		virtual uint32_t length() const override;

		double x{};
		double y{};
	};

	struct LayerRecord
	{
		LayerRecord(uint32_t l_number, std::string l_name);

		uint32_t length() const;
		LayerRect layer_content_rect{};
		uint16_t channel_count{};
		ChannelInfo alpha_channel_info{ -1 };
		ChannelInfo red_channel_info{ 0 };
		ChannelInfo green_channel_info{ 1 };
		ChannelInfo blue_channel_info{ 2 };
		std::string blend_mode_signature{ "8BIM" };
		std::string blend_mode_key{ "norm" };
		uint8_t opacity{ 255 };
		uint8_t clipping{ 0 };
		uint8_t flags;
		uint8_t filler{ 0 };
		uint32_t extra_data_length() const;
		LayerMaskData layer_mask_data{};
		uint32_t blending_ranges_length{ 40 };
		LayerBlendingRanges blending_range_grey{};
		LayerBlendingRanges blending_range_red{};
		LayerBlendingRanges blending_range_green{};
		LayerBlendingRanges blending_range_blue{};
		LayerBlendingRanges blending_range_alpha{};
		PascalString layer_name;
		AdditionalLayerInfoLuni unicode_layer_name;
		AdditionalLayerInfoLnsr layer_name_source_setting;
		AdditionalLayerInfoLyid layer_id;
		AdditionalLayerInfo blend_clipping_elements{ "clbl", {1, 0, 0, 0} };
		AdditionalLayerInfo blend_interior_elements{ "infx", {0, 0, 0, 0} };
		AdditionalLayerInfo knockout_setting{ "knko", {0, 0, 0, 0} };
		AdditionalLayerInfo protected_setting;
		AdditionalLayerInfo sheet_color_setting{ "lclr",
			{0, 0, 0, 0, 0, 0, 0, 0} };
		AdditionalLayerInfoShmd metadata_setting{};
		AdditionalLayerInfoCust cust{};
		AdditionalLayerInfoFxrp reference_point{};
	};

	struct GlobalLayerMaskInfo
	{
		// If no layer mask is in use, this only consists of filler.
		bool active{ false };
		uint32_t length() const;
		uint32_t overlay_cs{};
		uint16_t colour_comp_1{};
		uint16_t colour_comp_2{};
		uint16_t colour_comp_3{};
		uint16_t colour_comp_4{};
		uint16_t opacity{};
		uint8_t kind{};
		const std::vector<uint8_t> filler{ 0, 0, 0, 0 };
	};

	struct Header
	{
		const std::string file_signature{ "8BPS" };
		const uint16_t version{ 1 };
		const std::vector<uint8_t> reserved{ 0, 0, 0, 0, 0, 0 };
		uint16_t channel_count{ 3 };
		uint32_t height{};
		uint32_t width{};
		uint16_t depth{ 8 };
		uint16_t colour_mode{ 3 };
	};

	struct ColourModeData
	{
		// Only indexed and duotone colour modes have colour mode data.
		uint32_t colour_mode_data_length{ 0 };
	};

	struct ImageResourceBlock
	{
		const std::string signature{ "8BIM" };
		const uint16_t null_name{ 0 };
	};

	struct ResolutionInfo : public ImageResourceBlock
	{
		uint16_t uid{ 1005 };
		uint32_t length{ 16 };
		uint16_t h_res_int{ 72 }; // Fixed-point integer component.
		uint16_t h_res_frac{ 0 }; // Fixed-point fractional component.
		uint16_t h_res_unit{ 1 }; // 1=PPI, 2=PPCM.
		uint16_t width_unit{ 2 }; // 1=in, 2=cm, 3=pt, 4=picas, 5=cols.
		uint16_t v_res_int{ 72 };
		uint16_t v_res_frac{ 0 };
		uint16_t v_res_unit{ 1 };
		uint16_t height_unit{ 2 };
	};

	struct ICCProfile : public ImageResourceBlock
	{
		uint16_t uid{ 1039 };
		uint32_t length() const;
		std::vector<char> data{};
	};

	struct Guide
	{
		int32_t position{};
		uint8_t orientation{};
	};

	struct GridAndGuides : public ImageResourceBlock
	{
		uint16_t uid{ 1032 };
		uint32_t length() const;
		uint32_t version{ 1 };
		uint32_t grid_cycle_horizontal{ 576 };
		uint32_t grid_cycle_vertical{ 576 };
		uint32_t guide_count{ 0 };
		std::vector<Guide> guides{};
		uint8_t padding{ 0 };
	};

	struct ImageResources
	{
		uint32_t length() const;
		ResolutionInfo resolution{};
		ICCProfile icc_profile{};
		GridAndGuides grid_and_guides{};
	};

	struct LayerAndMaskInfo
	{
		uint32_t layer_and_mask_info_length() const;
		uint32_t layer_info_length() const;
		uint16_t layer_count() const;
		std::vector<LayerRecord> layer_records{};
		std::vector<std::unique_ptr<PSDImage>> layer_image_data{};
		uint16_t mystery_null{ 0 };
		GlobalLayerMaskInfo global_layer_mask_info{};
		AdditionalLayerInfo patterns{ "Patt" };
		AdditionalLayerInfo filter_mask{ "FMsk",
			{0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x32} };
		AdditionalLayerInfoCinf compositor_info{};
	};

	struct PSDData
	{
		Header header{};
		ColourModeData colour_mode_data{};
		ImageResources image_resources{};
		LayerAndMaskInfo layer_and_mask_info{};
		PSDRawImage image_data{};
	};
}

#endif