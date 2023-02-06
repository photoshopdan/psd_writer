# psd_writer
psd_writer is a simple C++ library for creating Adobe® Photoshop® PSD files. At present, it only supports the creation of 8BPC RGB documents.

## Usage
A basic use is shown below. First a blank, 1920px X 1080px document is created, then an image is added as a layer, before the document is saved as a file.
An OpenCV Mat object has been used in this case by accessing the pointer to its internal array, but any pointer to an image array will work, as long as the 
data is band-interleaved-by-pixel RGB or BGR.

    #include "psdocument.hpp"

    int main()
    {
        psdw::PSDocument psd{ 1920, 1080 };

        cv::Mat img;
        img = cv::imread("C:/Users/Dan/Desktop/img.png", cv::IMREAD_COLOR);
        psd.add_layer(img.data, { 100, 200, img.cols, img.rows }, "Layer 1");

        psd.save("Test.psd");

        return EXIT_SUCCESS;
    }
    
The PSDocument object can be initialised with finer control over the PSD options.
Here a 3000px wide by 2000px tall, 300ppi, grey canvas is created, with an assigned colour profile.

    psdw::PSDocument psd{ 3000, 2000, {128, 128, 128} };
    psd.set_resolution(300.0);
    psd.set_profile("C:/Windows/System32/spool/drivers/color/profile.icm");

    
