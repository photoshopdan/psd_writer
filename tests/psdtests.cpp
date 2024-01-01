#include "psdocument.hpp"
#include <cstdio>

using namespace psdw;

int main()
{
    PSDocument psd{ 3000, 2000, {128, 128, 128} };
    psd.set_resolution(300.0);
    psd.set_profile("C:/Windows/System32/spool/drivers/color/profile.icm");

    const char filename[]{ "Test.psd" };
    psd.save(filename);

    std::remove(filename);

    if (psd.status() == PSDStatus::Success)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}
