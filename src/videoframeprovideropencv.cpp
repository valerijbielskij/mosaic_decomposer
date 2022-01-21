#include <opencv2/core/mat.hpp>
#include <spdlog/spdlog.h>

#include "videoframeprovideropencv.h"
#include "frame.h"

VideoFrameProviderOpenCv::VideoFrameProviderOpenCv(const std::string& file_path): 
    FrameProviderInterface(), 
    m_video_capture(file_path)
{
    static_assert(sizeof(cv::Vec3b::value_type) == sizeof(Pixel::Color));
}

bool VideoFrameProviderOpenCv::isReady() const 
{
    return m_video_capture.isOpened();
}

std::optional<Frame> VideoFrameProviderOpenCv::getNext() 
{
    if (!isReady())
    {
        spdlog::warn("video read is not ready");
        return {};
    }    

    cv::Mat mat;

    if (m_video_capture.read(mat))
    {
        if (mat.cols < 1 || mat.rows < 1)
        {
            spdlog::error("acquired invalid material dimensions from a video ({}, {})", mat.cols, mat.rows);
            return {};
        }

        const auto cols = static_cast<Frame::DimensionsType>(mat.cols);
        const auto rows = static_cast<Frame::DimensionsType>(mat.rows);

        Frame frame(cols, rows);
        for (Frame::DimensionsType row = 0; row < rows; row++) 
        {
            for (Frame::DimensionsType col = 0; col < cols; col++)
            {
                const auto& src_pixel = mat.at<cv::Vec3b>(row, col);

                Pixel dst_pixel;
                dst_pixel.m_red = src_pixel[0];
                dst_pixel.m_green = src_pixel[1];
                dst_pixel.m_blue = src_pixel[2];

                frame.set(col, row, dst_pixel);
            }
        }

        return frame;
    }
    else
    {
        spdlog::info("read from the video capture was unsuccessful, probably all of the frames were provided");
        return {};
    }
}
