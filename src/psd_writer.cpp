#include "psd_writer.hpp"
#include <cstdint>
#include <bit>
#include <string>
#include <fstream>
#include <stdexcept>
#include <vector>
//#include <array>
#include <algorithm>
#include <filesystem>

#include <iostream>

using namespace psdw;

PSDocument::PSDocument(
    const int doc_width,
    const int doc_height,
    const double doc_ppi,
    const PSDColour doc_background_rgb,
    const std::filesystem::path doc_profile_path)
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
    , m_image_resources_length{ 0 }
    , m_resource_signature{ "8BIM" }
    , m_null_name{ 0 }
    , m_resolution_info_uid{ 1005 }
    , m_resolution_info_length{ 16 }
    , m_resolution{ 0, 0, 1, 2, 0, 0, 1, 2 }
    , m_icc_profile_uid{ 1039 }
    , m_icc_profile_path{ doc_profile_path }
    , m_layer_and_mask_info_length{ 0 }
    , m_layer_info_length{ 0 }
    , m_layer_count{ 1 }
    , m_layer_records{}
    , m_channel_image_compression{ 1 }
    , m_channel_image_data{}
    , m_global_layer_mask_info{}
    , m_additional_layer_info{}
    , m_image_compression{ 1 }
    , m_row_bytecounts{}
    , m_image_data{}
{
    // Check inputs are within the PSD maximum values.
    if (doc_width < 1 || doc_width > 30000
        || doc_height < 1 || doc_height > 30000
        || doc_ppi < 1 || doc_ppi >= 30000)
        throw std::invalid_argument("Argument out of acceptable range");
    m_width = static_cast<uint16_t>(doc_width);
    m_height = static_cast<uint16_t>(doc_height);

    // Resolution is stored as a fixed point value.
    uint16_t res_int{ static_cast<uint16_t>(doc_ppi) };
    uint16_t res_frac{ static_cast<uint16_t>(
        (doc_ppi - static_cast<int>(doc_ppi)) * 65536) };
    m_resolution.h_res_int = res_int;
    m_resolution.v_res_int = res_int;
    m_resolution.h_res_frac = res_frac;
    m_resolution.v_res_frac = res_frac;

    doc_background_rgb;
}

PSDStatus PSDocument::add_layer(const unsigned char* img,
    const int img_width, const int img_height,
    const int layer_x, const int layer_y,
    const std::string layer_name, PSDChannelOrder channel_order,
    PSDCompression compression)
{
    m_status = PSDStatus::Success;
    if (img_width <= 0 || img_height <= 0)
    {
        m_status = PSDStatus::InvalidArgument;
        return m_status;
    }
    
    pack_image(img, img_width, img_height, channel_order, compression);

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
    
    // If profile provided, check it's accessible.
    std::ifstream profilef{ m_icc_profile_path, std::ios::binary };
    if (!m_icc_profile_path.empty() && !profilef)
    {
        m_status = PSDStatus::NoProfileError;
        return m_status;
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
    write(outf, m_channel_count);
    write(outf, m_colour_mode);
    write(outf, m_width);
    write(outf, m_height);
    write(outf, m_depth);

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

    if (!m_icc_profile_path.empty())
    {
        write(outf, m_resource_signature);
        write(outf, m_icc_profile_uid);
        write(outf, m_null_name);

        profilef.seekg(0, std::ios::end);
        write(outf, static_cast<uint32_t>(profilef.tellg()));
        profilef.seekg(0, std::ios::beg);

        write(outf, profilef);
        profilef.close();
    }

    // Layer and mask section.
    write(outf, m_layer_and_mask_info_length);
    write(outf, m_layer_info_length);
    write(outf, m_layer_count);
    for (LayerRecord lr : m_layer_records)
    {
        write(outf, lr.layer_content_rect);
        write(outf, lr.layer_content_rect);
        write(outf, lr.channel_count);
        for (ChannelInfo i : lr.channel_info)
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
        write(outf, lr.layer_mask_rect);
        write(outf, lr.layer_mask_colour);
        write(outf, lr.layer_mask_flags);
        write(outf, lr.layer_mask_padding);
        write(outf, lr.blending_range_length);
        write(outf, lr.grey);
        write(outf, lr.red);
        write(outf, lr.green);
        write(outf, lr.blue);
        write(outf, lr.alpha);
        write(outf, lr.layer_name);
        for (AdditionalLayerInfo i : lr.additional_layer_info)
        {
            write(outf, i.signature);
            write(outf, i.key);
            write(outf, i.data_length);
            write(outf, i.data);
        }
    }

    write(outf, m_channel_image_compression); // Doesnt work. Fix.
    write(outf, m_channel_row_bytecounts);
    write(outf, m_channel_image_data);

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
    write(outf, m_image_compression);
    write(outf, m_row_bytecounts);
    write(outf, m_image_data);

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

PSDStatus PSDocument::pack_image(
    const unsigned char* img,
    int img_width,
    int img_height,
    PSDChannelOrder channel_order,
    PSDCompression compression)
{
    m_status = PSDStatus::Success;
    constexpr int channel_count{ 4 };

    m_channel_image_data.reserve(
        m_channel_image_data.size() + img_width * img_height * channel_count);

    std::vector<int> channels;
    if (channel_order == PSDChannelOrder::BGRA)
        channels = { 3, 2, 1, 0 };
    else
        channels = { 1, 2, 3, 0 };

    if (compression == PSDCompression::None
        || img_width < 4 || img_height < 4)
    {
        // Convert band-interleaved-by-pixel to band sequential, then store.
        for (int c : channels)
        {
            for (int y{}; y < img_height; y++)
            {
                for (int x{}; x < img_width; x++)
                {
                    m_channel_image_data.push_back(
                        img[get_index(channel_count, img_width, c, x, y)]);
                }
            }
        }
    }
    else
    {
        // An attempt to replicate Photoshop's implementation of PackBits.
        m_channel_row_bytecounts.reserve(
            m_channel_row_bytecounts.size() + img_height * channel_count);

        enum class State
        {
            Repeat,
            Literal
        };

        State state{ State::Repeat };
        constexpr int max_run{ 128 };

        std::vector<uint8_t> buffer{};
        buffer.reserve(max_run);

        // Convert band-interleaved-by-pixel to band sequential.
        for (int c : channels)
        {
            for (int y{}; y < img_height; y++)
            {
                uint16_t row_bytecount{ 0 };
                int run{ 0 };
                int run_group{ 0 };
                for (int x{}; x < img_width; x++)
                {
                    // Move x to the end of a run and count its length.
                    for (int r{ x };
                        img[get_index(channel_count, img_width, c, r, y)]
                        == img[get_index(channel_count, img_width, c, x, y)]
                        && r < img_width
                        && run <= max_run
                        && run_group < max_run;
                        r++)
                    {
                        run++;
                        run_group++;
                        x = r;
                    }

                    uint8_t current_val{ 
                        img[get_index(channel_count, img_width, c, x, y)] };
                    if (run >= 3)
                    {
                        // Always treat runs over two as repeat runs.
                        if (state == State::Repeat)
                        {
                            finalise_pack(buffer, row_bytecount);
                            finalise_pack(current_val, run, row_bytecount);
                        }
                        else
                        {
                            finalise_pack(buffer, row_bytecount);
                            finalise_pack(current_val, run, row_bytecount);
                            state = State::Repeat;
                        }
                    }
                    else if (run == 2)
                    {
                        // If the last run was a repeat, treat runs of two as
                        // a repeat, otherwise add to the last literal run.
                        if (state == State::Repeat)
                        {
                            finalise_pack(buffer, row_bytecount);
                            finalise_pack(current_val, run, row_bytecount);
                        }
                        else
                        {
                            for (int r{}; r < run; r++)
                                buffer.push_back(current_val);
                        }
                    }
                    else
                    {
                        // Always treat runs of one as a literal.
                        if (state == State::Repeat)
                        {
                            buffer.push_back(current_val);
                            state = State::Literal;
                        }
                        else
                        {
                            buffer.push_back(current_val);
                        }
                    }

                    if (run_group >= max_run)
                    {
                        finalise_pack(buffer, row_bytecount);
                        run_group = 0;
                        state = State::Repeat;
                    }
                    run = 0;
                }
                finalise_pack(buffer, row_bytecount);
                m_channel_row_bytecounts.push_back(row_bytecount);
                state = State::Repeat;
            }
        }
    }
    
    return m_status;
}

int PSDocument::get_index(int channels, int width, int channel, int x, int y)
{
    return y * width * channels + x * channels + channel;
}

void PSDocument::finalise_pack(std::vector<uint8_t>& buf, uint16_t& bytes)
{
    // For literal bytes.
    if (buf.empty())
        return;
    bytes += 1 + static_cast<uint16_t>(buf.size());
    m_channel_image_data.push_back(static_cast<uint8_t>(buf.size() - 1));
    m_channel_image_data.insert(
        m_channel_image_data.end(),
        buf.begin(), buf.end());
    buf.clear();
}

void PSDocument::finalise_pack(const uint8_t val, int& reps, uint16_t& bytes)
{
    // For repeated bytes.
    m_channel_image_data.push_back(reps * -1 + 1);
    m_channel_image_data.push_back(val);
    bytes += 2;
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
        buffer[1] = val & 0x00ff;
        buffer[0] = (val & 0x00ff) >> 8;
    }
    else
    {
        buffer[0] = val & 0x00ff;
        buffer[1] = (val & 0x00ff) >> 8;
    }
    
    file.write(buffer, sizeof(val));
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

void PSDocument::write(std::ofstream& file, const std::string& val) const
{
    file.write(val.data(), val.size());
}

void PSDocument::write(std::ofstream& file, std::ifstream& val) const
{
    file << val.rdbuf();
}

void PSDocument::write(std::ofstream& file, const Rect& val) const
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