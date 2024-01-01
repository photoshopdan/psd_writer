#include "psdocument.hpp"
#include <cstdio>

using namespace psdw;

int main()
{
    PSDocument psd{ 3000, 2000, {128, 128, 128} };

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
