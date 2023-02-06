#include "psdtypes.hpp"
#include "psdimage.hpp"

#include <string>
#include <chrono>
#include <cstdint>
#include <memory>

using namespace psdimpl;

AdditionalLayerInfoBase::AdditionalLayerInfoBase(std::string i_key)
    : signature{ "8BIM" }, key{ i_key }
{
}

AdditionalLayerInfo::AdditionalLayerInfo(std::string i_key,
    std::vector<uint8_t> i_data={})
    : AdditionalLayerInfoBase{ i_key }
    , data{ i_data }
{
}

uint32_t AdditionalLayerInfo::length() const
{
    return static_cast<uint32_t>(data.size());
}

AdditionalLayerInfoLuni::AdditionalLayerInfoLuni(std::string l_name)
    : AdditionalLayerInfo{ "luni" }
{
    // Populate unicode_layer_name.
    // Horrible, wrong, utf-16 conversion which ignores surrogates. Fix.
    data.push_back(static_cast<uint8_t>(l_name.length()));
    for (auto i : l_name)
    {
        data.push_back(0);
        data.push_back(i);
    }
    if (l_name.length() % 2 == 1) // Padding for odd lengths.
    {
        data.push_back(0);
        data.push_back(0);
    }
}

AdditionalLayerInfoLyid::AdditionalLayerInfoLyid(uint32_t l_number)
    : AdditionalLayerInfoBase{ "lyid" }
    , data{ l_number }
{
}

uint32_t AdditionalLayerInfoLyid::length() const
{
    return static_cast<uint32_t>(sizeof(data));
}

AdditionalLayerInfoCust::AdditionalLayerInfoCust()
    : AdditionalLayerInfo{ "cust" }
{
    /* 'cust' is an undocumented, oddball AdditionalLayerInfo, but is required.
    The first part has unknown purpose but doesn't appear to change between
    files, the second part is a float64 Unix timestamp of the layer creation
    time. */
    data = {
        0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x6D, 0x65, 0x74, 0x61,
        0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x09, 0x6C, 0x61, 0x79, 0x65, 0x72, 0x54, 0x69, 0x6D, 0x65, 0x64,
        0x6F, 0x75, 0x62 };

    const auto current_time = std::chrono::system_clock::now();
    time = std::chrono::duration_cast<std::chrono::milliseconds>(
        current_time.time_since_epoch()).count() / 1000.0;
}

uint32_t AdditionalLayerInfoCust::length() const
{
    return static_cast<uint32_t>(data.size() + sizeof(time));
}

AdditionalLayerInfoCinf::AdditionalLayerInfoCinf()
    : AdditionalLayerInfo{ "cinf" }
{
    /* This needs to be broken down into descriptor structures but the 
    documentation is too sparse for me to understand what they are. 
    The default value seems to work fine. */
    data = {
        0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x6E, 0x75, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x09,
        0x00, 0x00, 0x00, 0x00, 0x56, 0x72, 0x73, 0x6E, 0x4F, 0x62, 0x6A,
        0x63, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x6E, 0x75, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x05, 0x6D, 0x61, 0x6A, 0x6F, 0x72, 0x6C, 0x6F, 0x6E, 0x67, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x6D, 0x69, 0x6E, 0x6F,
        0x72, 0x6C, 0x6F, 0x6E, 0x67, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x00, 0x03, 0x66, 0x69, 0x78, 0x6C, 0x6F, 0x6E, 0x67, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x70, 0x73, 0x56, 0x65, 0x72,
        0x73, 0x69, 0x6F, 0x6E, 0x4F, 0x62, 0x6A, 0x63, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6E, 0x75, 0x6C, 0x6C,
        0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0x6D, 0x61, 0x6A,
        0x6F, 0x72, 0x6C, 0x6F, 0x6E, 0x67, 0x00, 0x00, 0x00, 0x15, 0x00,
        0x00, 0x00, 0x05, 0x6D, 0x69, 0x6E, 0x6F, 0x72, 0x6C, 0x6F, 0x6E,
        0x67, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x66, 0x69,
        0x78, 0x6C, 0x6F, 0x6E, 0x67, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
        0x00, 0x0B, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69,
        0x6F, 0x6E, 0x54, 0x45, 0x58, 0x54, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x06, 0x72, 0x65, 0x61, 0x73, 0x6F, 0x6E,
        0x54, 0x45, 0x58, 0x54, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x45, 0x6E, 0x67, 0x6E, 0x65, 0x6E, 0x75, 0x6D,
        0x00, 0x00, 0x00, 0x00, 0x45, 0x6E, 0x67, 0x6E, 0x00, 0x00, 0x00,
        0x08, 0x63, 0x6F, 0x6D, 0x70, 0x43, 0x6F, 0x72, 0x65, 0x00, 0x00,
        0x00, 0x0E, 0x65, 0x6E, 0x61, 0x62, 0x6C, 0x65, 0x43, 0x6F, 0x6D,
        0x70, 0x43, 0x6F, 0x72, 0x65, 0x65, 0x6E, 0x75, 0x6D, 0x00, 0x00,
        0x00, 0x06, 0x65, 0x6E, 0x61, 0x62, 0x6C, 0x65, 0x00, 0x00, 0x00,
        0x07, 0x66, 0x65, 0x61, 0x74, 0x75, 0x72, 0x65, 0x00, 0x00, 0x00,
        0x11, 0x65, 0x6E, 0x61, 0x62, 0x6C, 0x65, 0x43, 0x6F, 0x6D, 0x70,
        0x43, 0x6F, 0x72, 0x65, 0x47, 0x50, 0x55, 0x65, 0x6E, 0x75, 0x6D,
        0x00, 0x00, 0x00, 0x06, 0x65, 0x6E, 0x61, 0x62, 0x6C, 0x65, 0x00,
        0x00, 0x00, 0x07, 0x66, 0x65, 0x61, 0x74, 0x75, 0x72, 0x65, 0x00,
        0x00, 0x00, 0x0F, 0x63, 0x6F, 0x6D, 0x70, 0x43, 0x6F, 0x72, 0x65,
        0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x65, 0x6E, 0x75, 0x6D,
        0x00, 0x00, 0x00, 0x06, 0x72, 0x65, 0x61, 0x73, 0x6F, 0x6E, 0x00,
        0x00, 0x00, 0x09, 0x73, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x65,
        0x64, 0x00, 0x00, 0x00, 0x12, 0x63, 0x6F, 0x6D, 0x70, 0x43, 0x6F,
        0x72, 0x65, 0x47, 0x50, 0x55, 0x53, 0x75, 0x70, 0x70, 0x6F, 0x72,
        0x74, 0x65, 0x6E, 0x75, 0x6D, 0x00, 0x00, 0x00, 0x06, 0x72, 0x65,
        0x61, 0x73, 0x6F, 0x6E, 0x00, 0x00, 0x00, 0x0F, 0x66, 0x65, 0x61,
        0x74, 0x75, 0x72, 0x65, 0x44, 0x69, 0x73, 0x61, 0x62, 0x6C, 0x65,
        0x64
    };
}

uint32_t LayerMaskData::length() const
{
    return active ? 20 : 0;
}

LayerRecord::LayerRecord(uint32_t l_number, std::string l_name)
    : layer_name{ l_name }
    , unicode_layer_name{ l_name }
    , layer_id{ l_number }
{
}

uint32_t LayerRecord::length() const
{
    return 54 + extra_data_length();
}

uint32_t LayerRecord::extra_data_length() const
{
    uint32_t length{};

    length += layer_mask_data.length();
    length += blending_ranges_length;
    length += layer_name.size();
    length += unicode_layer_name.length();
    length += layer_id.length();
    length += blend_clipping_elements.length();
    length += blend_interior_elements.length();
    length += knockout_setting.length();
    length += protected_setting.length();
    length += sheet_color_setting.length();
    length += metadata_setting.length();
    length += cust.length();
    length += reference_point.length();
    
    return length;
}

uint32_t ImageResources::length() const
{
    return resolution.length + icc_profile.length();
}

uint32_t ICCProfile::length() const
{
    return static_cast<uint32_t>(data.size());
}

uint32_t LayerAndMaskInfo::layer_and_mask_info_length() const
{
    return layer_info_length()
        + global_layer_mask_info.length()
        + patterns.length()
        + filter_mask.length()
        + compositor_info.length();
}

uint32_t LayerAndMaskInfo::layer_info_length() const
{
    uint32_t length{};

    length += sizeof(layer_count);
    for (LayerRecord record : layer_records)
    {
        length += record.length();
    }
    for (auto &image : layer_image_data)
    {
        for (PSDChannel channel : image->data())
        {
            length += channel.bytecounts.size();
            length += channel.image_data.size();
        }
    }

    return length;
}

uint32_t GlobalLayerMaskInfo::length() const
{
    if (active)
    {
        return static_cast<uint32_t>(
            sizeof(overlay_cs)
            + sizeof(colour_comp_1)
            + sizeof(colour_comp_2)
            + sizeof(colour_comp_3)
            + sizeof(colour_comp_4)
            + sizeof(opacity)
            + sizeof(kind)
            + filler.size());
    }
    else
    {
        return static_cast<uint32_t>(filler.size());
    }
}