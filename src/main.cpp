//#include <cstdint>
#include "psdocument.hpp"
#include <iostream>
#include <stdexcept>

#include <fstream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

//#include <opencv2/highgui.hpp>
#include <bitset>

using namespace cv;
using namespace psdw;
//#pragma warning(suppress: C5054);

int main()
{

    /*
    for (int i{}; i < 384; i++)
    {
        std::cout << static_cast<int>(img.data[i]) << " ";
        if ((i + 1) % 24 == 0 && i > 1)
            std::cout << '\n';
    }
    
    std::ifstream profilef{ "" };
    if (!profilef)
    {
        std::cout << "Errorly";
    }
    */

    PSDocument psd{ 420, 256 };

    psd.set_resolution(300.0);
    psd.set_profile("C:/Windows/System32/spool/drivers/color/sRGB Color Space Profile.icm");

    Mat img;
    img = imread("C:/Users/Dan/Documents/C++ Files/PSD/jeep-0022.png", IMREAD_UNCHANGED);

    psd.add_layer(img.data, { 50, 30, img.cols, img.rows }, "Jeep Layer",
        PSDChannelOrder::BGRA, PSDCompression::RLE);

    psd.save("C:/Users/Dan/Documents/C++ Files/PSD/Test.psd", true);
    
    if (psd.status() == PSDStatus::Success)
        std::cout << "Success" << '\n';
    else if (psd.status() == PSDStatus::FileExistsError)
        std::cout << "Delete old file" << '\n';
    else
        std::cout << "Failed" << '\n';
    

    return 0;
}
