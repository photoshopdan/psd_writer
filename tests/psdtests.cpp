#include "psdocument.hpp"

using namespace psdw;

int main()
{
    PSDocument psd{ 3000, 2000, {128, 128, 128} };
    psd.set_resolution(300.0);
    psd.set_profile("C:/Windows/System32/spool/drivers/color/profile.icm");

    psd.save("Test.psd");

    if (psd.status() == PSDStatus::Success)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}
