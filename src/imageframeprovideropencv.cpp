#include <opencv2/opencv.hpp>
#include <spdlog/spdlog.h>

#include "imageframeprovideropencv.h"
#include "frame.h"

ImageFrameProviderOpenCv::ImageFrameProviderOpenCv(const std::string& file_path): 
    FrameProviderInterface(), 
    m_image(cv::imread(file_path))
{
    static_assert(sizeof(cv::Vec3b::value_type) == sizeof(Pixel::Color));
}

bool ImageFrameProviderOpenCv::isReady() const 
{
    return m_image.cols > 0 && m_image.rows > 0;
}

std::optional<Frame> ImageFrameProviderOpenCv::getNext() 
{
    if (!isReady())
    {
        spdlog::warn("image read is not ready");
        return {};
    }

    if (m_consumed)
    {
        spdlog::warn("image's frame has been already consumed");
        return {};
    }

    const auto cols = static_cast<Frame::DimensionsType>(m_image.cols);
    const auto rows = static_cast<Frame::DimensionsType>(m_image.rows);

    Frame frame(cols, rows);
    for (Frame::DimensionsType row = 0; row < rows; row++) 
    {
        for (Frame::DimensionsType col = 0; col < cols; col++)
        {
            const auto& src_pixel = m_image.at<cv::Vec3b>(row, col);

            Pixel dst_pixel;
            dst_pixel.m_red = src_pixel[0];
            dst_pixel.m_green = src_pixel[1];
            dst_pixel.m_blue = src_pixel[2];

            frame.set(col, row, dst_pixel);
        }
    }

    m_consumed = true;

    return frame;
}
