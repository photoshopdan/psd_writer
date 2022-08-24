#include "psdocument.hpp"
#include "psdtypes.hpp"
#include "psdimage.hpp"

#include <cstdint>
#include <bit>
#include <string>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cmath>
#include <memory>
#include <locale>
#include <codecvt>
#include <cuchar>
#include <chrono>

#include <iostream>

using namespace psdw;
using namespace psdimpl;

PSDocument::PSDocument(
    int doc_width,
    int doc_height,
    const PSDColour doc_background_rgb)
    : m_status{ PSDStatus::Success }
    , m_little_endian{ little_endian() }
    , m_file_signature{ "8BPS" }
    , m_version{ 1 }
    , m_channel_count{ 3 }
    , m_colour_mode{ 3 }
    , m_width{}
    , m_height{}
    , m_depth{ 8 }
    , m_colour_mode_data_length{ 0 }
    , m_image_resources_length{ 40 }
    , m_resource_signature{ "8BIM" }
    , m_null_name{ 0 }
    , m_resolution_info_uid{ 1005 }
    , m_resolution_info_length{ 16 }
    , m_resolution{ 72, 0, 1, 2, 72, 0, 1, 2 }
    , m_icc_profile_uid{ 1039 }
    , m_icc_profile{}
    , m_layer_and_mask_info_length{ 0 }
    , m_layer_info_length{ 0 }
    , m_layer_count{ 1 }
    , m_layer_records{}
    , m_layer_image_data{}
    , m_global_layer_mask_info{}
    , m_additional_layer_info{}
    , m_merged_image_data{}
{
    // Check inputs are within the PSD maximum values.
    if (doc_width < 1 || doc_width > 30000
        || doc_height < 1 || doc_height > 30000)
        throw std::invalid_argument("Invalid document size");
    m_width = static_cast<uint16_t>(doc_width);
    m_height = static_cast<uint16_t>(doc_height);

    // Generate background, add to channel data and merged image data.
    m_merged_image_data.generate(m_width, m_height, doc_background_rgb);
    m_layer_image_data.push_back(
        std::make_unique<PSDCompressedImage>(PSDCompressedImage{}));
    m_layer_image_data.back()->load(
        m_merged_image_data.data(),
        m_merged_image_data.channels(),
        m_merged_image_data.width(),
        m_merged_image_data.height());

    // Update layer and mask section data.
    m_layer_records.push_back(LayerRecord{});
    m_layer_records.back().blending_range_length = 1;
}

PSDStatus PSDocument::set_resolution(double ppi)
{
    m_status = PSDStatus::Success;
    if (ppi < 1 || ppi >= 30000)
    {
        m_status = PSDStatus::InvalidArgument;
        return m_status;
    }

    // Resolution is stored as a fixed point value.
    uint16_t res_int{ static_cast<uint16_t>(ppi) };
    uint16_t res_frac{ static_cast<uint16_t>(
        (ppi - static_cast<int>(ppi)) * 65536) };
    m_resolution.h_res_int = res_int;
    m_resolution.v_res_int = res_int;
    m_resolution.h_res_frac = res_frac;
    m_resolution.v_res_frac = res_frac;

    return m_status;
}

PSDStatus PSDocument::set_profile(std::filesystem::path icc_profile)
{
    m_status = PSDStatus::Success;

    std::ifstream profilef{ icc_profile, std::ios::binary };
    if (!profilef)
    {
        m_status = PSDStatus::NoProfileError;
        return m_status;
    }

    std::ostringstream ss;
    ss << profilef.rdbuf();
    m_icc_profile = ss.str();

    m_image_resources_length += static_cast<uint32_t>(m_icc_profile.length());

    return m_status;
}

PSDStatus PSDocument::add_layer(const unsigned char* img,
    PSDRect rect,
    const std::string layer_name, PSDChannelOrder channel_order,
    PSDCompression compression)
{
    m_status = PSDStatus::Success;
    if (rect.w <= 0 || rect.h <= 0 || layer_name.length() > 251)
    {
        m_status = PSDStatus::InvalidArgument;
        return m_status;
    }

    // Store image.
    if (compression == PSDCompression::None)
        m_layer_image_data.push_back(
            std::make_unique<PSDRawImage>(PSDRawImage{}));
    else
        m_layer_image_data.push_back(
            std::make_unique<PSDCompressedImage>(PSDCompressedImage{}));

    psdimpl::PSDChannelOrder co;
    if (channel_order == psdw::PSDChannelOrder::RGBA)
        co = psdimpl::PSDChannelOrder::RGBA;
    else
        co = psdimpl::PSDChannelOrder::BGRA;

    m_layer_image_data.back()->load(img, co, rect.w, rect.h);

    // Add image to merged image.
    m_merged_image_data.composite(img, rect, channel_order);

    // Update layer data.
    m_layer_count++;


    return m_status;
}

PSDStatus PSDocument::save(const std::filesystem::path& filepath,
    bool overwrite)
{
    m_status = PSDStatus::Success;

    if (!overwrite)
    {
        // Check file doesn't already exist.
        std::ifstream testf{ filepath };
        if (testf.good())
        {
            m_status = PSDStatus::FileExistsError;
            return m_status;
        }
    }

    // Open new file.
	std::ofstream outf{ filepath, std::ios::binary | std::ios::trunc };
    if (!outf)
    {
        m_status = PSDStatus::FileWriteError;
        return m_status;
    }

    // Header section.
    write(outf, m_file_signature);
    write(outf, m_version);
    std::vector<uint16_t> reserved{ 0, 0, 0 };
    write(outf, reserved);
    write(outf, m_channel_count);
    write(outf, m_height);
    write(outf, m_width);
    write(outf, m_depth);
    write(outf, m_colour_mode);

    // Colour mode section.
    write(outf, m_colour_mode_data_length);

    // Image resources section.
    write(outf, m_image_resources_length);

    write(outf, m_resource_signature);
    write(outf, m_resolution_info_uid);
    write(outf, m_null_name);
    write(outf, m_resolution_info_length);
    write(outf, m_resolution.h_res_int);
    write(outf, m_resolution.h_res_frac);
    write(outf, m_resolution.h_res_unit);
    write(outf, m_resolution.width_unit);
    write(outf, m_resolution.v_res_int);
    write(outf, m_resolution.v_res_frac);
    write(outf, m_resolution.v_res_unit);
    write(outf, m_resolution.height_unit);

    if (!m_icc_profile.empty())
    {
        write(outf, m_resource_signature);
        write(outf, m_icc_profile_uid);
        write(outf, m_null_name);

        std::cout << m_icc_profile.length() << '\n';
        write(outf, static_cast<uint32_t>(m_icc_profile.length()));
        write(outf, m_icc_profile);
    }

    // Layer and mask section.
    write(outf, m_layer_and_mask_info_length);
    write(outf, m_layer_info_length);
    write(outf, m_layer_count);
    for (const LayerRecord& lr : m_layer_records)
    {
        write(outf, lr.layer_content_rect);
        write(outf, lr.layer_content_rect);
        write(outf, lr.channel_count);
        for (const ChannelInfo& i : lr.channel_info)
        {
            write(outf, i.channel_id);
            write(outf, i.channel_data_length);
        }
        write(outf, lr.blend_mode_signature);
        write(outf, lr.blend_mode_key);
        write(outf, lr.opacity);
        write(outf, lr.clipping);
        write(outf, lr.flags);
        write(outf, lr.filler);
        write(outf, lr.extra_data_length);
        write(outf, lr.layer_mask_data_length);
        if (lr.layer_mask_data_length != 0)
        {
            write(outf, lr.layer_mask_rect);
            write(outf, lr.layer_mask_colour);
            write(outf, lr.layer_mask_flags);
            write(outf, lr.layer_mask_padding);
        }
        write(outf, lr.blending_range_length);
        write(outf, lr.grey);
        write(outf, lr.red);
        write(outf, lr.green);
        write(outf, lr.blue);
        write(outf, lr.alpha);
        write(outf, lr.layer_name, true);
        for (const AdditionalLayerInfo& i : lr.additional_layer_info)
        {
            write(outf, i.signature);
            write(outf, i.key);
            write(outf, i.data_length);
            write(outf, i.data);
        }
    }

    for (const auto& image_ptr : m_layer_image_data)
    {
        for (const auto& channel : image_ptr->data())
        {
            write(outf, channel.compression);
            write(outf, channel.bytecounts);
            write(outf, channel.image_data);
        }
    }

    write(outf, m_global_layer_mask_info.length);
    write(outf, m_global_layer_mask_info.overlay_colour_space);
    write(outf, m_global_layer_mask_info.colour_comp_1);
    write(outf, m_global_layer_mask_info.colour_comp_2);
    write(outf, m_global_layer_mask_info.colour_comp_3);
    write(outf, m_global_layer_mask_info.colour_comp_4);
    write(outf, m_global_layer_mask_info.opacity);
    write(outf, m_global_layer_mask_info.kind);
    write(outf, m_global_layer_mask_info.filler);

    for (AdditionalLayerInfo i : m_additional_layer_info)
    {
        write(outf, i.signature);
        write(outf, i.key);
        write(outf, i.data_length);
        write(outf, i.data);
    }

    // Image data section.
    PSDCompressedImage compressed_merged_image_data{};
    compressed_merged_image_data.load(
        m_merged_image_data.data(),
        m_merged_image_data.channels(),
        m_merged_image_data.width(),
        m_merged_image_data.height());

    write(outf, compressed_merged_image_data.data()[0].compression);
    for (const auto& channel : compressed_merged_image_data.data())
        write(outf, channel.bytecounts);
    for (const auto& channel : compressed_merged_image_data.data())
        write(outf, channel.image_data);

    // Check writer for error flags. If so, destroy and fail.
    if (outf.fail())
    {
        outf.close();
        std::filesystem::remove(filepath);
        m_status = PSDStatus::FileWriteError;
        return m_status;
    }

    return m_status;
}

bool PSDocument::little_endian()
{
    if constexpr (std::endian::native == std::endian::little)
        return true;
    else if constexpr (std::endian::native == std::endian::big)
        return false;
    else
        throw std::runtime_error("Invalid endianness");
}

void PSDocument::write(std::ofstream& file, const uint8_t& val) const
{
    char buffer[sizeof(val)]{ static_cast<char>(val) };
    file.write(buffer, sizeof(val));
}

void PSDocument::write(std::ofstream& file, const uint16_t& val) const
{
    char buffer[sizeof(val)]{};

    if (m_little_endian)
    {
        // Convert to big-endian.
        buffer[0] = (val >> 8);
        buffer[1] = val & 0x00ff;
    }
    else
    {
        buffer[0] = val & 0x00ff;
        buffer[1] = (val >> 8);
    }
    
    file.write(buffer, sizeof(val));
}

void PSDocument::write(std::ofstream& file, const int16_t& val) const
{
    uint16_t tmp{ static_cast<uint16_t>(val) };
    write(file, tmp);
}

void PSDocument::write(std::ofstream& file, const uint32_t& val) const
{
    char buffer[sizeof(val)]{};

    if (m_little_endian)
    {
        // Convert to big-endian.
        buffer[3] = val & 0x000000ff;
        buffer[2] = (val & 0x0000ff00) >> 8;
        buffer[1] = (val & 0x00ff0000) >> 16;
        buffer[0] = (val & 0xff000000) >> 24;
    }
    else
    {
        buffer[0] = val & 0x000000ff;
        buffer[1] = (val & 0x0000ff00) >> 8;
        buffer[2] = (val & 0x00ff0000) >> 16;
        buffer[3] = (val & 0xff000000) >> 24;
    }

    file.write(buffer, sizeof(val));
}

void PSDocument::write(std::ofstream& file,
    const std::vector<uint8_t>& val) const
{
    for (uint8_t i : val)
        write(file, i);
}

void PSDocument::write(std::ofstream& file,
    const std::vector<uint16_t>& val) const
{
    for (uint16_t i : val)
        write(file, i);
}

void PSDocument::write(std::ofstream& file, const std::string& val,
    bool pascal_string) const
{
    if (pascal_string)
    {
        uint8_t string_length{ static_cast<uint8_t>(val.length()) };
        int buffer_length{ 4 - (string_length + 1) % 4 };
        uint8_t buffer_val{ 0 };
        write(file, string_length);
        file.write(val.data(), val.size());
        for (int i{ 0 }; i < buffer_length; i++)
            write(file, buffer_val);
    }
    else
    {
        file.write(val.data(), val.size());
    }
}

void PSDocument::write(std::ofstream& file, const LayerRect& val) const
{
    write(file, val.top);
    write(file, val.left);
    write(file, val.bottom);
    write(file, val.right);
}

void PSDocument::write(std::ofstream& file, const LayerBlendingRanges& val) const
{
    write(file, val.src_black_lower);
    write(file, val.src_black_upper);
    write(file, val.src_white_lower);
    write(file, val.src_white_upper);
    write(file, val.dst_black_lower);
    write(file, val.dst_black_upper);
    write(file, val.dst_white_lower);
    write(file, val.dst_white_upper);
}

PSDocument::LayerRecord::LayerRecord(bool little_endian, int layer_number,
    std::string layer_name)
{
    m_little_endian = little_endian;
    populate_luni(layer_name);
    populate_lyid(layer_number);
    populate_cust();
}

void PSDocument::LayerRecord::populate_luni(std::string layer_name)
{
    unicode_layer_name.signature = "8BIM";
    unicode_layer_name.key = "luni";
    unicode_layer_name.data_length = 4;

    std::wstring_convert<
        std::codecvt<char16_t, char, std::mbstate_t>, char16_t> convert;
    std::u16string u16{ convert.from_bytes(layer_name) };
    
    unicode_layer_name.data.push_back(u16.length());
    for (char16_t i : u16)
    {
        uint8_t* buffer{ reinterpret_cast<uint8_t*>(&i) };
        unicode_layer_name.data_length += 2;
        if (m_little_endian)
        {
            unicode_layer_name.data.push_back(buffer[1]);
            unicode_layer_name.data.push_back(buffer[0]);
        }
        else
        {
            unicode_layer_name.data.push_back(buffer[0]);
            unicode_layer_name.data.push_back(buffer[1]);
        }
    }
    if (u16.length() % 2 == 1) // Padding for odd lengths.
    {
        unicode_layer_name.data.push_back(0);
        unicode_layer_name.data.push_back(0);
        unicode_layer_name.data_length += 2;
    }
}

void PSDocument::LayerRecord::populate_lyid(int layer_number)
{
    unicode_layer_name.signature = "8BIM";
    unicode_layer_name.key = "lyid";
    unicode_layer_name.data_length = 4;


}

void PSDocument::LayerRecord::populate_cust()
{
    // 'cust' is an undocumented, oddball AdditionalLayerInfo, but is required.
    // The first part has unknown purpose but doesn't appear to change between
    // files, the second part is a float64 Unix timestamp of the layer
    // creation time.
    cust.signature = "8BIM";
    cust.key = "cust";
    cust.data_length = 0;
    cust.data = {
        0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x6D, 0x65,
        0x74, 0x61, 0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x09, 0x6C, 0x61, 0x79, 0x65, 0x72, 0x54,
        0x69, 0x6D, 0x65, 0x64, 0x6F, 0x75, 0x62 };

    const auto current_time = std::chrono::system_clock::now();
    double unix_time{ std::chrono::duration_cast<std::chrono::milliseconds>(
        current_time.time_since_epoch()).count() / 1000.0 };

    uint8_t* buffer{ reinterpret_cast<uint8_t*>(&unix_time) };

    if (m_little_endian)
    {
        for (int i{ sizeof(double) - 1 }; i >= 0; i--)
        {
            cust.data.push_back(buffer[i]);
        }
    }
    else
    {
        for (int i{ 0 }; i < sizeof(double); i++)
        {
            cust.data.push_back(buffer[i]);
        }
    }

    cust.data.push_back(0);
}
