#include "psdimage.hpp"
#include "psdtypes.hpp"

#include <vector>
#include <cstdint>

using namespace psdimpl;
using namespace psdw;

size_t PSDImage::get_index(int channels, int width, int channel,
	int x, int y) const
{
	return static_cast<size_t>(y) * width * channels
		+ static_cast<size_t>(x) * channels + channel;
}

std::vector<int> PSDImage::enumerate_channels(
    ChannelOrder channel_order) const
{
    std::vector<int> channels;
    if (channel_order == ChannelOrder::RGBA)
        channels = { 1, 2, 3, 0 };
    else if (channel_order == ChannelOrder::BGRA)
        channels = { 3, 2, 1, 0 };
    else
        channels = { 0, 1, 2 };

    return channels;
}

PSDStatus PSDRawImage::load(const unsigned char* img,
    ChannelOrder channel_order, int width, int height)
{
    // Overwrite.
    if (!m_image_data.empty())
        m_image_data.clear();

    std::vector<int> channels{ enumerate_channels(channel_order) };

    m_channels = static_cast<int>(channels.size());
    m_width = width;
    m_height = height;

    // Read band-interleaved-by-pixel, store as band-sequential.
    for (int c : channels)
    {
        m_image_data.push_back(PSDChannel());
        m_image_data.back().compression = 0;
        m_image_data.back().image_data.reserve(
            static_cast<size_t>(width) * height);
        for (int y{}; y < height; y++)
        {
            for (int x{}; x < width; x++)
            {
                m_image_data.back().image_data.push_back(
                    img[get_index(m_channels, width, c, x, y)]);
            }
        }
    } 

    return PSDStatus::Success;
}

PSDStatus PSDRawImage::load(std::vector<PSDChannel> img, int channels,
    int width, int height)
{
    m_channels = channels;
    m_width = width;
    m_height = height;
    m_image_data = img;

    return PSDStatus::Success;
}

psdw::PSDStatus PSDRawImage::generate(int width, int height, PSDColour colour)
{
    // Overwrite.
    if (!m_image_data.empty())
        m_image_data.clear();

    m_channels = 3;
    m_width = width;
    m_height = height;

    // Create background.
    size_t elements{ static_cast<size_t>(width)
        * static_cast<size_t>(height) };
    std::vector<uint8_t> rgb{ colour.r, colour.g, colour.b };

    for (uint8_t c : rgb)
    {
        m_image_data.push_back({});
        m_image_data.back().compression = 0;
        m_image_data.back().image_data.reserve(elements);
        for (size_t i{}; i < elements; i++)
            m_image_data.back().image_data.push_back(c);
    }

    return PSDStatus::Success;
}

void PSDRawImage::composite(const unsigned char* foreground,
    psdw::PSDRect rect, psdw::PSDChannelOrder foreground_channel_order)
{
    std::vector<int> fg_channels;
    if (foreground_channel_order == psdw::PSDChannelOrder::BGRA)
        fg_channels = { 2, 1, 0, 3 };
    else
        fg_channels = { 0, 1, 2, 3 };

    std::vector<int> bg_channels;
    if (m_channels == 3)
        bg_channels = { 0, 1, 2 };
    else
        bg_channels = { 1, 2, 3 };

    for (int y{}; y < rect.h; y++)
    {
        for (int x{}; x < rect.w; x++)
        {
            // Basic implementation of alpha compositing.
            int fg_channel_count{ static_cast<int>(fg_channels.size()) };
            float fg_red{
                static_cast<float>(
                    foreground[get_index(
                        fg_channel_count, rect.w,
                        fg_channels[0], x, y)]) };
            float fg_green{
                static_cast<float>(
                    foreground[get_index(
                        fg_channel_count, rect.w,
                        fg_channels[1], x, y)]) };
            float fg_blue{
                static_cast<float>(
                    foreground[get_index(
                        fg_channel_count, rect.w,
                        fg_channels[2], x, y)]) };
            float fg_alpha{
                static_cast<float>(
                    foreground[get_index(
                        fg_channel_count, rect.w,
                        fg_channels[3], x, y)]) };

            fg_alpha *= 1.0f / 255; // Normalise 0 - 1.

            size_t bg_index{ static_cast<size_t>(y) * rect.w + x };
            
            uint8_t new_red{
                static_cast<uint8_t>(
                    round(fg_red * fg_alpha
                        + m_image_data[bg_channels[0]].image_data[bg_index]
                        * (1.0f - fg_alpha))) };
            uint8_t new_green{ 
                static_cast<uint8_t>(
                    round(fg_red * fg_alpha
                        + m_image_data[bg_channels[1]].image_data[bg_index]
                        * (1.0f - fg_alpha))) };
            uint8_t new_blue{
                static_cast<uint8_t>(
                    round(fg_red * fg_alpha
                        + m_image_data[bg_channels[2]].image_data[bg_index]
                        * (1.0f - fg_alpha))) };

            m_image_data[bg_channels[0]].image_data[bg_index] = new_red;
            m_image_data[bg_channels[1]].image_data[bg_index] = new_green;
            m_image_data[bg_channels[2]].image_data[bg_index] = new_blue;
        }
    }
}

PSDStatus PSDCompressedImage::load(const unsigned char* img,
	ChannelOrder channel_order, int width, int height)
{
    // Overwrite.
    if (!m_image_data.empty())
        m_image_data.clear();
    
    // An attempt to replicate Photoshop's implementation of PackBits.
    std::vector<int> channels{ enumerate_channels(channel_order) };

    m_channels = static_cast<int>(channels.size());
    m_width = width;
    m_height = height;
    
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
        m_image_data.push_back(PSDChannel());
        m_image_data.back().compression = 1;
        m_image_data.back().image_data.reserve(
            static_cast<size_t>(width) * height);
        m_image_data.back().bytecounts.reserve(height);
        for (int y{}; y < height; y++)
        {
            uint16_t row_bytecount{ 0 };
            int run{ 0 };
            int run_group{ 0 };
            for (int x{}; x < width; x++)
            {
                // Move x to the end of a run and count its length.
                for (int r{ x };
                    img[get_index(m_channels, width, c, r, y)]
                    == img[get_index(m_channels, width, c, x, y)]
                    && r < width
                    && run <= max_run
                    && run_group < max_run;
                    r++)
                {
                    run++;
                    run_group++;
                    x = r;
                }

                uint8_t current_val{
                    img[get_index(m_channels, width, c, x, y)] };
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
            m_image_data.back().bytecounts.push_back(row_bytecount);
            state = State::Repeat;
        }
    }

    return PSDStatus::Success;
}

PSDStatus PSDCompressedImage::load(std::vector<PSDChannel> img, int channels,
    int width, int height)
{
    // Confirm correct number of channels.
    if (img.size() != 3)
        return PSDStatus::InvalidArgument;
    // Confirm this isn't already compressed.
    if (!img[0].bytecounts.empty())
        return PSDStatus::InvalidArgument;

    // Convert PSDRawImage into temporary flat BIP array.
    std::vector<uint8_t> flat_img;
    for (int i{}; i < width * height; i++)
    {
        flat_img.push_back(img.data()[0].image_data[i]);
        flat_img.push_back(img.data()[1].image_data[i]);
        flat_img.push_back(img.data()[2].image_data[i]);
    }

    // Load conventionally.
    PSDStatus status = load(flat_img.data(), ChannelOrder::RGB,
        width, height);

    return status;
}

void PSDCompressedImage::finalise_pack(std::vector<uint8_t>& buf, uint16_t& bytes)
{
    // For literal bytes.
    if (buf.empty())
        return;
    bytes += 1 + static_cast<uint16_t>(buf.size());
    m_image_data.back().image_data.push_back(
        static_cast<uint8_t>(buf.size() - 1));
    m_image_data.back().image_data.insert(
        m_image_data.back().image_data.end(),
        buf.begin(), buf.end());
    buf.clear();
}

void PSDCompressedImage::finalise_pack(const uint8_t val, int& reps, uint16_t& bytes)
{
    // For repeated bytes.
    m_image_data.back().image_data.push_back(reps * -1 + 1);
    m_image_data.back().image_data.push_back(val);
    bytes += 2;
}