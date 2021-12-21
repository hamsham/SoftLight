
/*
 * g++ --std=c++11 -O3 -DNUM_TEST_RUNS=50000 -Wall -Werror -Wextra -pedantic -pedantic-errors line_drawing.cpp -o line_drawing
 */

#include <cstdint> // uint8_t
#include <fstream> // std::ofstream
#include <iostream> // std::cout, std::cerr
#include <limits> // std::numeric_limits<>
#include <utility> // std::move

#include <chrono>
#include <thread>

#include <cassert>
#include <cstring>

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Pointer.h"

#include "softlight/SL_Geometry.hpp"
#include "softlight/SL_ImgFilePPM.hpp"



/*------------------------------------------------------------------------------
 * Benchmark Setup
------------------------------------------------------------------------------*/
#ifndef NUM_TEST_RUNS
    #define NUM_TEST_RUNS 10000
#endif /* NUM_TEST_RUNS */

#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 640
#endif /* IMAGE_WIDTH  */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 480
#endif /* IMAGE_HEIGHT  */

namespace chrono = std::chrono;
typedef chrono::steady_clock hr_clock;
typedef hr_clock::time_point hr_time;
typedef chrono::milliseconds hr_prec;



/*------------------------------------------------------------------------------
 * Create Images
------------------------------------------------------------------------------*/
ls::utils::Pointer<SL_ColorRGB8[]> create_image(const coord_shrt_t width, const coord_shrt_t height)
{
    ls::utils::Pointer<SL_ColorRGB8[]> pImg{new SL_ColorRGB8[width * height]};

    if (!pImg)
    {
        std::cerr << "Unable to create an " << width << 'x' << height << " image." << std::endl;
        return pImg;
    }

    for (coord_shrt_t i = 0; i < height; ++i)
    {
        for (coord_shrt_t j = 0; j < width; ++j)
        {
            pImg[width * i + j] = SL_ColorRGB8{0, 0, 0};
        }
    }

    return pImg;
}



/*------------------------------------------------------------------------------
 * Benchmark Function
------------------------------------------------------------------------------*/
int run_benchmark(
    const std::string& testName,
    coord_shrt_t w,
    coord_shrt_t h,
    void (*line_callback)(SL_ColorRGB8* const, coord_shrt_t, coord_shrt_t, coord_shrt_t, coord_shrt_t, coord_shrt_t, const SL_ColorRGB8&))
{
    const coord_shrt_t w1 = w - 1;
    const coord_shrt_t h1 = h - 1;
    ls::utils::Pointer<SL_ColorRGB8[]> img = create_image(w, h);
    hr_time t1, t2;
    
    t1 = hr_clock::now();
    
    for (unsigned t = 0; t < NUM_TEST_RUNS; ++t)
    {
        for (coord_shrt_t i = 0; i < w; i += 10)
        {
            line_callback(img.get(), w, i, 0, w1-i, h1, SL_ColorRGB8{0, 255, 0});
        }
    
        for (coord_shrt_t i = 0; i < h; i += 10)
        {
            line_callback(img.get(), w, 0, i, w1, h1-i, SL_ColorRGB8{255, 0, 0});
        }
    }

    t2 = hr_clock::now();

    std::cout.precision(std::numeric_limits<double>::digits10);
    std::cout << testName.c_str() << " Benchmark: "
              << (double)chrono::duration_cast< hr_prec >(t2 - t1).count() / 1000.0
              << std::endl;

    const std::string filename = testName + ".ppm";
    const int ret = sl_img_save_ppm(w, h, img.get(), filename.c_str());

    const std::string filename2 = testName + "2.ppm";
    coord_shrt_t w2, h2;
    const SL_ColorRGB8* pImg2 = sl_img_load_ppm(w2, h2, filename.c_str());

    LS_ASSERT(w == w2);
    LS_ASSERT(h == h2);
    LS_ASSERT(pImg2 != nullptr);
    LS_ASSERT(sl_img_save_ppm(w2, h2, pImg2, filename2.c_str()) == 0);
    LS_ASSERT(memcmp(img.get(), pImg2, (std::size_t)(w2 * h2)) == 0);

    delete pImg2;

    return ret;
}



/*------------------------------------------------------------------------------
 * Main
------------------------------------------------------------------------------*/
int main()
{
    std::thread t1(run_benchmark, std::string{"EFLA_5"},       (uint16_t)IMAGE_WIDTH, (uint16_t)IMAGE_HEIGHT, &sl_draw_line_efla5);
    std::thread t2(run_benchmark, std::string{"Bresenham_FP"}, (uint16_t)IMAGE_WIDTH, (uint16_t)IMAGE_HEIGHT, &sl_draw_line_fixed);
    std::thread t3(run_benchmark, std::string{"Bresenham"},    (uint16_t)IMAGE_WIDTH, (uint16_t)IMAGE_HEIGHT, &sl_draw_line_bresenham);
    t1.join();
    t2.join();
    t3.join();
    return 0;
}

