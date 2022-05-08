### Work in progress - not complete or functional

# psd_writer
psd_writer is a simple C++ library for creating PSD files. At present, it only supports the creation of 8BPC RGB documents.

## Usage
A basic use is shown below. First a blank, 1920px X 1080px document is created, then an image is added as a layer, before the document is saved as a file.
An OpenCV Mat object has been used in this case by accessing the pointer to it's internal array, but any pointer to an image array will work, as long as the 
data is band-interleaved-by-pixel RGB or BGR.

    psdw::PSDocument psd{ 1920, 1080 };

    cv::Mat img;
    img = cv::imread("C:/Users/Dan/Desktop/img.png", cv::IMREAD_COLOR);
    psd.add_layer(img.data, img.cols, img.rows, 100, 200, "Layer 1");

    psd.save("Test.psd");
    
The PSDocument object can be initialised with finer control over the PSD options.
Here a 3000px wide by 2000px tall, 300ppi, grey canvas is created, with an assigned colour profile.

    psdw::PSDocument psd{ 3000, 2000, 300.0, {128, 128, 128},
        "C:/Windows/System32/spool/drivers/color/sRGB Color Space Profile.icm" };

    
