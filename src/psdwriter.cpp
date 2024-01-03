#include "psdwriter.hpp"
#include "psddata.hpp"
#include "psdtypes.hpp"

#include <filesystem>
#include <bit>
#include <cstdint>
#include <string>
#include <vector>

using namespace psdw;
using namespace psdimpl;

PSDWriter::PSDWriter(const PSDData& psd_data)
    : m_status{ PSDStatus::Success }, m_data{ psd_data }
{
}

PSDStatus PSDWriter::write(const std::filesystem::path& filepath, bool overwrite)
{
    m_status = PSDStatus::Success;

    if (!overwrite && std::filesystem::exists(filepath))
    {
        m_status = PSDStatus::FileExistsError;
        return m_status;
    }

    m_writer.open(filepath, std::ios::binary | std::ios::trunc);
    if (!m_writer)
    {
        m_status = PSDStatus::FileWriteError;
        return m_status;
    }
    
    // Header section.
    write(m_data.header.file_signature);
    write(m_data.header.version);
    write(m_data.header.reserved);
    write(m_data.header.channel_count);
    write(m_data.header.height);
    write(m_data.header.width);
    write(m_data.header.depth);
    write(m_data.header.colour_mode);

    // Colour mode section.
    write(m_data.colour_mode_data.colour_mode_data_length);

    // Image resources section.
    write(m_data.image_resources.length());

    write(m_data.image_resources.resolution.signature);
    write(m_data.image_resources.resolution.uid);
    write(m_data.image_resources.resolution.null_name);
    write(m_data.image_resources.resolution.length);
    write(m_data.image_resources.resolution.h_res_int);
    write(m_data.image_resources.resolution.h_res_frac);
    write(m_data.image_resources.resolution.h_res_unit);
    write(m_data.image_resources.resolution.width_unit);
    write(m_data.image_resources.resolution.v_res_int);
    write(m_data.image_resources.resolution.v_res_frac);
    write(m_data.image_resources.resolution.v_res_unit);
    write(m_data.image_resources.resolution.height_unit);

    if (m_data.image_resources.icc_profile.length())
    {
        write(m_data.image_resources.icc_profile.signature);
        write(m_data.image_resources.icc_profile.uid);
        write(m_data.image_resources.icc_profile.null_name);
        write(m_data.image_resources.icc_profile.length());
        write(m_data.image_resources.icc_profile.data);
    }

    write(m_data.image_resources.grid_and_guides.signature);
    write(m_data.image_resources.grid_and_guides.uid);
    write(m_data.image_resources.grid_and_guides.null_name);
    write(m_data.image_resources.grid_and_guides.length());
    write(m_data.image_resources.grid_and_guides.version);
    write(m_data.image_resources.grid_and_guides.grid_cycle_horizontal);
    write(m_data.image_resources.grid_and_guides.grid_cycle_vertical);
    write(m_data.image_resources.grid_and_guides.guide_count);
    for (const Guide& g : m_data.image_resources.grid_and_guides.guides)
    {
        write(g.position);
        write(g.orientation);
    }
    if (m_data.image_resources.grid_and_guides.length() % 2 != 0)
    {
        write(m_data.image_resources.grid_and_guides.padding);
    }

    // Layer and mask section.
    write(m_data.layer_and_mask_info.layer_and_mask_info_length());
    write(m_data.layer_and_mask_info.layer_info_length());
    write(m_data.layer_and_mask_info.layer_count());
    for (const LayerRecord& lr : m_data.layer_and_mask_info.layer_records)
    {
        write(lr.layer_content_rect);
        write(lr.channel_count);
        if (lr.alpha_channel_info.length)
        {
            write(lr.alpha_channel_info);
        }
        if (lr.red_channel_info.length)
        {
            write(lr.red_channel_info);
        }
        if (lr.green_channel_info.length)
        {
            write(lr.green_channel_info);
        }
        if (lr.blue_channel_info.length)
        {
            write(lr.blue_channel_info);
        }
        write(lr.blend_mode_signature);
        write(lr.blend_mode_key);
        write(lr.opacity);
        write(lr.clipping);
        write(lr.flags);
        write(lr.filler);
        write(lr.extra_data_length());
        write(lr.layer_mask_data.length());
        if (lr.layer_mask_data.active)
        {
            write(lr.layer_mask_data.rect);
            write(lr.layer_mask_data.colour);
            write(lr.layer_mask_data.flags);
            write(lr.layer_mask_data.padding);
        }
        write(lr.blending_ranges_length);
        write(lr.blending_range_grey);
        write(lr.blending_range_red);
        write(lr.blending_range_green);
        write(lr.blending_range_blue);
        write(lr.blending_range_alpha);
        write(lr.layer_name);
        write(lr.unicode_layer_name);
        write(lr.layer_name_source_setting);
        write(lr.layer_id);
        write(lr.blend_clipping_elements);
        write(lr.blend_interior_elements);
        write(lr.knockout_setting);
        write(lr.protected_setting);
        write(lr.sheet_color_setting);
        write(lr.metadata_setting);
        write(lr.cust);
        write(lr.reference_point);
    }

    for (const auto& image_ptr : m_data.layer_and_mask_info.layer_image_data)
    {
        for (const auto& channel : image_ptr->data())
        {
            write(channel.compression);
            write(channel.bytecounts);
            write(channel.image_data);
        }
    }
    write(m_data.layer_and_mask_info.mystery_null);

    if (m_data.layer_and_mask_info.global_layer_mask_info.active)
    {
        write(m_data.layer_and_mask_info.global_layer_mask_info.length());
        write(m_data.layer_and_mask_info.global_layer_mask_info.overlay_cs);
        write(m_data.layer_and_mask_info.global_layer_mask_info.colour_comp_1);
        write(m_data.layer_and_mask_info.global_layer_mask_info.colour_comp_2);
        write(m_data.layer_and_mask_info.global_layer_mask_info.colour_comp_3);
        write(m_data.layer_and_mask_info.global_layer_mask_info.colour_comp_4);
        write(m_data.layer_and_mask_info.global_layer_mask_info.opacity);
        write(m_data.layer_and_mask_info.global_layer_mask_info.kind);
        write(m_data.layer_and_mask_info.global_layer_mask_info.filler);
    }
    else
    {
        write(m_data.layer_and_mask_info.global_layer_mask_info.filler);
    }
    
    write(m_data.layer_and_mask_info.patterns);
    write(m_data.layer_and_mask_info.filter_mask);
    write(m_data.layer_and_mask_info.compositor_info);

    // Image data section.
    PSDCompressedImage compressed_merged_image_data{};
    compressed_merged_image_data.load(
        m_data.image_data.data(),
        m_data.image_data.channels(),
        m_data.image_data.width(),
        m_data.image_data.height());

    write(compressed_merged_image_data.data()[0].compression);
    for (const auto& channel : compressed_merged_image_data.data())
        write(channel.bytecounts);
    for (const auto& channel : compressed_merged_image_data.data())
        write(channel.image_data);

    // Close writer and check for errors. PSD files should not exceed 2GiB.
    auto file_size = m_writer.tellp();
    m_writer.close();
    if (m_writer.fail() || file_size >= 2147483648)
    {
        m_status = PSDStatus::FileWriteError;
        std::filesystem::remove(filepath);
    }

    return m_status;
}

constexpr bool PSDWriter::little_endian()
{
    static_assert(
        std::endian::native == std::endian::little ||
        std::endian::native == std::endian::big,
        "Invalid system endianness");
    if (std::endian::native == std::endian::little)
        return true;
    else
        return false;
}

void PSDWriter::write(const uint8_t& val)
{
    char buffer[sizeof(val)]{ static_cast<char>(val) };
    m_writer.write(buffer, sizeof(val));
}

void PSDWriter::write(const uint16_t& val)
{
    char buffer[sizeof(val)]{};

    if (little_endian())
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

    m_writer.write(buffer, sizeof(val));
}

void PSDWriter::write(const int16_t& val)
{
    write(static_cast<uint16_t>(val));
}

void PSDWriter::write(const uint32_t& val)
{
    char buffer[sizeof(val)]{};

    if (little_endian())
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

    m_writer.write(buffer, sizeof(val));
}

void PSDWriter::write(const int32_t& val)
{
    write(static_cast<uint32_t>(val));
}

void PSDWriter::write(const double& val)
{
    char buffer[sizeof(val)]{ 0 };
    const uint64_t temp_val{ *(uint64_t*)&val };

    if (little_endian())
    {
        // Convert to big-endian.
        buffer[7] = static_cast<char>(temp_val & 0x00000000000000ff);
        buffer[6] = static_cast<char>((temp_val & 0x000000000000ff00) >> 8);
        buffer[5] = static_cast<char>((temp_val & 0x0000000000ff0000) >> 16);
        buffer[4] = static_cast<char>((temp_val & 0x00000000ff000000) >> 24);
        buffer[3] = static_cast<char>((temp_val & 0x000000ff00000000) >> 32);
        buffer[2] = static_cast<char>((temp_val & 0x0000ff0000000000) >> 40);
        buffer[1] = static_cast<char>((temp_val & 0x00ff000000000000) >> 48);
        buffer[0] = static_cast<char>((temp_val & 0xff00000000000000) >> 56);
    }
    else
    {
        buffer[0] = static_cast<char>(temp_val & 0x00000000000000ff);
        buffer[1] = static_cast<char>((temp_val & 0x000000000000ff00) >> 8);
        buffer[2] = static_cast<char>((temp_val & 0x0000000000ff0000) >> 16);
        buffer[3] = static_cast<char>((temp_val & 0x00000000ff000000) >> 24);
        buffer[4] = static_cast<char>((temp_val & 0x000000ff00000000) >> 32);
        buffer[5] = static_cast<char>((temp_val & 0x0000ff0000000000) >> 40);
        buffer[6] = static_cast<char>((temp_val & 0x00ff000000000000) >> 48);
        buffer[7] = static_cast<char>((temp_val & 0xff00000000000000) >> 56);
    }

    m_writer.write(buffer, sizeof(val));
}

void PSDWriter::write_with_null(const double& val)
{
    write(val);
    char buffer[1]{ 0 };

    m_writer.write(buffer, 1);
}

void PSDWriter::write(const std::vector<char>& val)
{
    m_writer.write(val.data(), val.size());
}

void PSDWriter::write(const std::vector<uint8_t>& val)
{
    for (uint8_t i : val)
        write(i);
}

void PSDWriter::write(const std::vector<uint16_t>& val)
{
    for (uint16_t i : val)
        write(i);
}

void PSDWriter::write(const std::string& val)
{
    m_writer.write(val.data(), val.size());
}

void PSDWriter::write(const PascalString& val)
{
    write(static_cast<uint8_t>(val.string.size()));
    write(val.string);
    write(val.buffer);
}

void PSDWriter::write(const LayerRect& val)
{
    write(val.top);
    write(val.left);
    write(val.bottom);
    write(val.right);
}

void PSDWriter::write(const ChannelInfo& val)
{
    write(val.id);
    write(val.length);
}

void PSDWriter::write(const LayerBlendingRanges& val)
{
    write(val.src_black_lower);
    write(val.src_black_upper);
    write(val.src_white_lower);
    write(val.src_white_upper);
    write(val.dst_black_lower);
    write(val.dst_black_upper);
    write(val.dst_white_lower);
    write(val.dst_white_upper);
}

void PSDWriter::write(const AdditionalLayerInfo& val)
{
    write(val.signature);
    write(val.key);
    write(val.length());
    write(val.data);
}

void PSDWriter::write(const AdditionalLayerInfoLuni& val)
{
    write(val.signature);
    write(val.key);
    write(val.length());
    write(val.string_length);
    write(val.data);
}

void PSDWriter::write(const AdditionalLayerInfoLyid& val)
{
    write(val.signature);
    write(val.key);
    write(val.length());
    write(val.data);
}

void PSDWriter::write(const AdditionalLayerInfoLnsr& val)
{
    write(val.signature);
    write(val.key);
    write(val.length());
    write(val.keyword);
}

void PSDWriter::write(const AdditionalLayerInfoShmd& val)
{
    write(val.signature);
    write(val.key);
    write(val.data);
}

void PSDWriter::write(const AdditionalLayerInfoCust& val)
{
    write(val.signature);
    write(val.key);
    write(val.data);
    write_with_null(val.time);
}

void PSDWriter::write(const AdditionalLayerInfoFxrp& val)
{
    write(val.signature);
    write(val.key);
    write(val.length());
    write(val.x);
    write(val.y);
}

