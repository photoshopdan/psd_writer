// Copyright (c) 2024 Dan Kemp. All rights reserved.
// This source code is licensed under the MIT license found in the 
// LICENSE file in the root directory of this source tree.

#include "psdocument.hpp"
#include "psddata.hpp"
#include "psdtypes.hpp"
#include "psdimage.hpp"
#include "psdwriter.hpp"

#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <memory>

using namespace psdw;
using namespace psdimpl;

// Implementation class.
class PSDocument::PSDocumentImpl
{
public:
    PSDocumentImpl(
        int doc_width,
        int doc_height,
        const PSDColour doc_background_rgb)
    {
        // Check inputs are within the PSD maximum values. Clip if not.
        m_data.header.width = doc_width < 1 ? 1 :
            doc_width > 30000 ? 30000 :
            static_cast<uint32_t>(doc_width);
        m_data.header.height = doc_height < 1 ? 1 :
            doc_height > 30000 ? 30000 :
            static_cast<uint32_t>(doc_height);

        // Generate background, add to channel data and merged image data.
        m_data.image_data.generate(
            m_data.header.width, m_data.header.height, doc_background_rgb);
        m_data.layer_and_mask_info.layer_image_data.push_back(
            std::make_unique<PSDCompressedImage>(PSDCompressedImage{}));
        m_data.layer_and_mask_info.layer_image_data.back()->load(
            m_data.image_data.data(),
            m_data.image_data.channels(),
            m_data.image_data.width(),
            m_data.image_data.height());

        // Update layer and mask section data.
        m_data.layer_and_mask_info.layer_records.push_back(
            LayerRecord{1, "Background", true});
        m_data.layer_and_mask_info.layer_records.back().layer_content_rect = {
            0, 0, m_data.header.height, m_data.header.width };
        m_data.layer_and_mask_info.layer_records.back().channel_count = 3;

        m_data.layer_and_mask_info.layer_records.back().red_channel_info.length =
            static_cast<uint32_t>(
                sizeof(m_data.layer_and_mask_info.layer_image_data.
                    back()->data()[0].compression)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[0].bytecounts.size() * sizeof(uint16_t)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[0].image_data.size());
        m_data.layer_and_mask_info.layer_records.back().green_channel_info.length =
            static_cast<uint32_t>(
                sizeof(m_data.layer_and_mask_info.layer_image_data.
                    back()->data()[1].compression)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[1].bytecounts.size() * sizeof(uint16_t)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[1].image_data.size());
        m_data.layer_and_mask_info.layer_records.back().blue_channel_info.length =
            static_cast<uint32_t>(
                sizeof(m_data.layer_and_mask_info.layer_image_data.
                    back()->data()[2].compression)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[2].bytecounts.size() * sizeof(uint16_t)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[2].image_data.size());
    }

    PSDStatus set_resolution(double ppi)
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
        m_data.image_resources.resolution.h_res_int = res_int;
        m_data.image_resources.resolution.v_res_int = res_int;
        m_data.image_resources.resolution.h_res_frac = res_frac;
        m_data.image_resources.resolution.v_res_frac = res_frac;

        return m_status;
    }

    PSDStatus set_profile(std::filesystem::path icc_profile)
    {
        m_status = PSDStatus::Success;

        std::ifstream profilef{ icc_profile, std::ios::binary | std::ios::ate };
        if (!profilef)
        {
            m_status = PSDStatus::NoProfileError;
            return m_status;
        }
        
        uint32_t length = static_cast<uint32_t>(profilef.tellg());
        profilef.seekg(0, std::ios::beg);

        m_data.image_resources.icc_profile.data.resize(length);
        if (!profilef.read(m_data.image_resources.icc_profile.data.data(), length))
        {
            m_data.image_resources.icc_profile.data.clear();
            m_status = PSDStatus::NoProfileError;
            return m_status;
        }

        return m_status;
    }

    PSDStatus add_guide(int position, PSDOrientation orientation)
    {
        m_status = PSDStatus::Success;

        int multiplier = 32;
        psdimpl::Guide guide{ 
            static_cast<int32_t>(position * multiplier),
            static_cast<uint8_t>(orientation) };
        m_data.image_resources.grid_and_guides.guides.push_back(guide);
        m_data.image_resources.grid_and_guides.guide_count++;

        return m_status;
    }

    PSDStatus add_layer(const unsigned char* img,
        PSDRect rect,
        const std::string layer_name,
        bool visible,
        PSDChannelOrder channel_order,
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
            m_data.layer_and_mask_info.layer_image_data.push_back(
                std::make_unique<PSDRawImage>(PSDRawImage{}));
        else
            m_data.layer_and_mask_info.layer_image_data.push_back(
                std::make_unique<PSDCompressedImage>(PSDCompressedImage{}));

        psdimpl::ChannelOrder co;
        if (channel_order == psdw::PSDChannelOrder::RGBA)
            co = psdimpl::ChannelOrder::RGBA;
        else
            co = psdimpl::ChannelOrder::BGRA;

        m_data.layer_and_mask_info.layer_image_data.back()->load(
            img, co, rect.w, rect.h);

        // Add image to merged image.
        if (visible)
        {
            m_data.image_data.composite(img, rect, channel_order);
        }

        // Update layer data.
        m_data.layer_and_mask_info.layer_records.push_back(
            LayerRecord{
                static_cast<uint32_t>(
                    m_data.layer_and_mask_info.layer_count() + 1),
                layer_name,
                visible});
        m_data.layer_and_mask_info.layer_records.back().layer_content_rect = {
            static_cast<uint32_t>(rect.y),
            static_cast<uint32_t>(rect.x),
            static_cast<uint32_t>(rect.h + rect.y),
            static_cast<uint32_t>(rect.w + rect.x) };
        m_data.layer_and_mask_info.layer_records.back().channel_count = 4;
        m_data.layer_and_mask_info.layer_records.back().reference_point.x = rect.x;
        m_data.layer_and_mask_info.layer_records.back().reference_point.y = rect.y;
        
        m_data.layer_and_mask_info.layer_records.back().alpha_channel_info.length =
            static_cast<uint32_t>(
                sizeof(m_data.layer_and_mask_info.layer_image_data.
                    back()->data()[0].compression)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[0].bytecounts.size() * sizeof(uint16_t)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[0].image_data.size());
        m_data.layer_and_mask_info.layer_records.back().red_channel_info.length =
            static_cast<uint32_t>(
                sizeof(m_data.layer_and_mask_info.layer_image_data.
                    back()->data()[1].compression)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[1].bytecounts.size() * sizeof(uint16_t)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[1].image_data.size());
        m_data.layer_and_mask_info.layer_records.back().green_channel_info.length =
            static_cast<uint32_t>(
                sizeof(m_data.layer_and_mask_info.layer_image_data.
                    back()->data()[2].compression)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[2].bytecounts.size() * sizeof(uint16_t)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[2].image_data.size());
        m_data.layer_and_mask_info.layer_records.back().blue_channel_info.length =
            static_cast<uint32_t>(
                sizeof(m_data.layer_and_mask_info.layer_image_data.
                    back()->data()[3].compression)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[3].bytecounts.size() * sizeof(uint16_t)
                + m_data.layer_and_mask_info.layer_image_data.
                back()->data()[3].image_data.size());

        return m_status;
    }

    PSDStatus save(const std::filesystem::path& filepath,
        bool overwrite)
    {
        m_status = m_writer.write(filepath, overwrite);
        return m_status;
    }

    PSDStatus status() const { return m_status; }

private:
    PSDStatus m_status{ PSDStatus::Success };
	psdimpl::PSDData m_data{};
	psdimpl::PSDWriter m_writer{ m_data };
};

// Implementation of interface class.
PSDocument::PSDocument(int doc_width, int doc_height,
    const PSDColour doc_background_rgb)
    : m_psdocument{ new PSDocumentImpl(doc_width, doc_height, doc_background_rgb) }
{
}

PSDocument::~PSDocument()
{
    delete m_psdocument;
}

PSDocument::PSDocument(PSDocument&&) noexcept = default;
PSDocument& PSDocument::operator=(PSDocument&&) noexcept = default;

PSDStatus PSDocument::set_resolution(double ppi)
{
    return m_psdocument->set_resolution(ppi);
}

PSDStatus PSDocument::set_profile(std::filesystem::path icc_profile)
{
    return m_psdocument->set_profile(icc_profile);
}

PSDStatus PSDocument::add_guide(int position, PSDOrientation orientation)
{
    return m_psdocument->add_guide(position, orientation);
}

PSDStatus PSDocument::add_layer(const unsigned char* img,
    PSDRect rect,
	std::string layer_name,
    bool visible,
	PSDChannelOrder channel_order,
	PSDCompression compression)
{
    return m_psdocument->add_layer(img, rect, layer_name, visible, channel_order, compression);
}

PSDStatus PSDocument::save(const std::filesystem::path& filename,
    bool overwrite)
{
    return m_psdocument->save(filename, overwrite);
}

PSDStatus PSDocument::status() const
{
    return m_psdocument->status();
}
