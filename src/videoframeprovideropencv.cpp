#include "videoframeprovideropencv.h"
#include <opencv2/core/mat.hpp>
#include "frame.h"

VideoFrameProviderOpenCv::VideoFrameProviderOpenCv(const std::filesystem::path& file_path): FrameProviderInterface(), m_video_capture(file_path.string())
{
}

bool VideoFrameProviderOpenCv::isReady() const 
{
    return m_video_capture.isOpened();
}

std::optional<Frame> VideoFrameProviderOpenCv::getNext() 
{
    static_assert(sizeof(cv::Vec3b::value_type) == sizeof(Color));

    cv::Mat mat;
    cv::OutputArray array(mat);

    if (m_video_capture.read(mat))
    {
        // TODO range check for frame size

        Frame frame(static_cast<uint16_t>(mat.cols), static_cast<uint16_t>(mat.rows));
        for (uint16_t row = 0; row < static_cast<uint16_t>(mat.rows); row++) 
        {
            for (uint16_t col = 0; col < static_cast<uint16_t>(mat.cols); col++)
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
        return {};
    }
}