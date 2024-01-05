// Copyright (c) 2024 Dan Kemp. All rights reserved.
// This source code is licensed under the MIT license found in the 
// LICENSE file in the root directory of this source tree.

#include "psdocument.hpp"
#include <cstdio>
#include <vector>

using namespace psdw;

// Default chequerboard pattern image.
class DefaultImage
{
public:
    DefaultImage()
        : m_width{ 200 }, m_height{ 200 }, m_image{}
    {
        const std::vector<unsigned char> magenta{ 255, 0, 255, 255 };
        const std::vector<unsigned char> black{ 0, 0, 0, 255 };
        const int run_length{ 25 };

        m_image.reserve(m_width * m_height * 4);
        std::vector<unsigned char> first_colour;
        std::vector<unsigned char> second_colour;
        for (int block{ 0 }; block < (m_width / run_length); block++)
        {
            first_colour = block % 2 == 0 ? magenta : black;
            second_colour = block % 2 == 0 ? black : magenta;
            for (int block_row{ 0 }; block_row < run_length; block_row++)
            {
                for (int row{ 0 }; row < (m_width / run_length / 2); row++)
                {
                    for (int fc{ 0 }; fc < run_length; fc++)
                    {
                        m_image.insert(m_image.end(), first_colour.begin(), first_colour.end());
                    }
                    for (int sc{ 0 }; sc < run_length; sc++)
                    {
                        m_image.insert(m_image.end(), second_colour.begin(), second_colour.end());
                    }
                }
            }
        }
    }

    const unsigned char* get_image_ptr() const
    {
        return m_image.data();
    }

    const int m_width;
    const int m_height;

private:
    std::vector<unsigned char> m_image;
};

int main()
{
    PSDocument psd{ 1200, 800, {128, 128, 128} };

    psd.set_resolution(300.0);
    if (psd.status() != PSDStatus::Success)
    {
        return EXIT_FAILURE;
    }
    
    psd.set_profile("sRGB_v4_ICC_preference.icc");
    if (psd.status() != PSDStatus::Success)
    {
        return EXIT_FAILURE;
    }

    psd.add_guide(25, PSDOrientation::Vertical);
    psd.add_guide(-25, PSDOrientation::Vertical);
    psd.add_guide(775, PSDOrientation::Horizontal);
    psd.add_guide(825, PSDOrientation::Horizontal);
    if (psd.status() != PSDStatus::Success)
    {
        return EXIT_FAILURE;
    }

    DefaultImage image{};
    psd.add_layer(image.get_image_ptr(), { 400, 200, image.m_width, image.m_height },
        "Layer 1", true, PSDChannelOrder::RGBA);
    if (psd.status() != PSDStatus::Success)
    {
        return EXIT_FAILURE;
    }

    psd.add_layer(image.get_image_ptr(), { 700, 400, image.m_width, image.m_height },
        "Layer 2", false, PSDChannelOrder::RGBA);
    if (psd.status() != PSDStatus::Success)
    {
        return EXIT_FAILURE;
    }

    const char filename[]{ "Test.psd" };
    psd.save(filename);
    if (psd.status() != PSDStatus::Success)
    {
        std::remove(filename);
        return EXIT_FAILURE;
    }

    std::remove(filename);

    return EXIT_SUCCESS;
}
