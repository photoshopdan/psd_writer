# psd_writer

![Build](https://github.com/photoshopdan/psd_writer/actions/workflows/build.yml/badge.svg)

psd_writer is a simple C++ library for creating Adobe® Photoshop® PSD files. At present, it only supports the creation of 8BPC RGB documents.

## Getting started
Builds of the shared library can be found in [releases](https://github.com/photoshopdan/psd_writer/releases). Alternatively you can build it yourself, as shown below.

### Prerequisites
- Git.
- CMake 3.26 or higher.

### Installation
Clone this repository:
```Shell
git clone https://github.com/photoshopdan/psd_writer.git
```
To configure and build from the project directory:
```Shell
cmake -S . -B build
cmake --build build --config Release
```
To run the tests from the project directory:
```Shell
ctest -C Release --test-dir build  
```

## Usage
The output of the build process is an import library and DLL which can be imported into your code as usual.

A basic use is shown below. First a blank, 1920px X 1080px document is created, then an image is added as a layer, then the document is saved as a file. An OpenCV Mat object has been used in this case by accessing the pointer to its internal array, but any pointer to an image array will work, as long as the data is band-interleaved-by-pixel RGBA or BGRA.

```cpp
#include "psdocument.hpp"

using namespace psdw;

int main()
{
    PSDocument psd{ 1920, 1080 };

    cv::Mat img;
    img = cv::imread("C:/Users/Dan/Desktop/img.png", cv::IMREAD_COLOR);
    psd.add_layer(img.data, { 100, 200, img.cols, img.rows }, "Layer 1");

    psd.save("Test.psd");

    return EXIT_SUCCESS;
}
```

The PSDocument object can be initialised with finer control over the PSD options. Here a 3000px wide by 2000px tall, 300ppi, grey canvas is created, with an assigned colour profile and a guide. The visibility, channel order and compression type can be specified when adding a layer. Each method returns a value indicating whether the operation succeeded or if not, the reason for failure.

```cpp
#include "psdocument.hpp"

using namespace psdw;

int main()
{
    PSDocument psd{ 3000, 2000, {128, 128, 128} };
    psd.set_resolution(300.0);
    psd.set_profile("C:/Windows/System32/spool/drivers/color/profile.icm");
    psd.add_guide(25, PSDOrientation::Vertical);

    cv::Mat img;
    img = cv::imread("C:/Users/Dan/Desktop/img.png", cv::IMREAD_COLOR);
    psd.add_layer(img.data, { 50, 30, img.cols, img.rows }, "Layer 1",
        true, PSDChannelOrder::BGRA, PSDCompression::RLE);

    psd.save("Test.psd");

    if (psd.status() == PSDStatus::Success)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}
```

## License
MIT License

Copyright (c) 2024 Dan Kemp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
