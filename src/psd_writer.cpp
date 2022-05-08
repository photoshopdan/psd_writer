#include "psd_writer.hpp"
#include <cstdint>
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
    , m_resolution_info_length{ 0 }
    , m_resolution{ 0, 0, 1, 2, 0, 0, 1, 2 }
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
    const std::string layer_name, PSDChannelOrder channel_order)
{
    m_status = PSDStatus::Success;
    if (img_width <= 0 || img_height <= 0)
    {
        m_status = PSDStatus::InvalidArgument;
        return m_status;
    }
    
    pack_image(img, img_width, img_height, channel_order);

    return m_status;
}

PSDStatus PSDocument::save(std::filesystem::path filepath)
{
    m_status = PSDStatus::Success;

    // Check file doesn't already exist.
    std::ifstream testf{ filepath };
    if (testf.good())
    {
        m_status = PSDStatus::FileExistsError;
        return m_status;
    }

    // If profile provided, check it's accessible.
    if (!m_icc_profile_path.empty())
    {
        std::ifstream profilef{ m_icc_profile_path };
        if (!profilef)
        {
            m_status = PSDStatus::NoProfileError;
            return m_status;
        }
    }

    // Open new file.
	std::ofstream outf{ filepath, std::ios::binary };
    if (!outf)
    {
        m_status = PSDStatus::FileWriteError;
        return m_status;
    }


    /*
    uint8_t testval8{ 255 };
    write(outf, testval8);

    char testchar{ 'k' };
    write(outf, testchar);

    write(outf, m_file_signature);
    */

    return m_status;
}

PSDStatus PSDocument::pack_image(
    const unsigned char* img,
    int img_width,
    int img_height,
    PSDChannelOrder channel_order)
{
    m_channel_image_data.reserve(img_width * img_height * 3);

    std::vector<int> channels;
    if (channel_order == PSDChannelOrder::BGR)
        channels = { 2, 1, 0 };
    else
        channels = { 0, 1, 2 };

    for (int c : channels)
    {
        for (int y{}; y < img_height; y++)
        {
            for (int x{}; x < img_width; x++)
            {
                //std::cout << static_cast<int>(img[y * img_width * 3 + x * 3 + c]) << " ";
                int index{ y * img_width * 3 + x * 3 + c };
                // Implement some PackBits here.
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }


    return PSDStatus::Success;
}

template<typename T>
void PSDocument::write(std::ofstream& file, const T& val)
{
    // Convert to big-endian.
    auto ptr{ reinterpret_cast<const char*>(&val) };
    std::array<char, sizeof(T)> raw_src, raw_dst;

    for (size_t i = 0; i < sizeof(T); ++i)
        raw_src[i] = ptr[i];

    std::reverse_copy(raw_src.begin(), raw_src.end(), raw_dst.begin());

    // Write to file.
    file.write(raw_dst.data(), raw_dst.size());
}

template <>
void PSDocument::write(std::ofstream& file, const std::string& val)
{
    // Write to file.
    file.write(val.data(), val.size());
}
