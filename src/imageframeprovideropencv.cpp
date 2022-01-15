#include "imageframeprovideropencv.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include "frame.h"

ImageFrameProviderOpenCv::ImageFrameProviderOpenCv(const std::filesystem::path& file_path): FrameProviderInterface(), m_image(cv::imread(file_path.string()))
{
}

bool ImageFrameProviderOpenCv::isReady() const 
{
    return m_image.cols != 0 && m_image.rows != 0;
}

std::optional<Frame> ImageFrameProviderOpenCv::getNext() 
{
    static_assert(sizeof(cv::Vec3b::value_type) == sizeof(Color));

    if (!isReady())
    {
        return {}; // log
    }

    if (m_consumed)
    {
        return {}; // log
    }

    // TODO range check for rows and cols?
    
    cv::Mat rotated;

    transpose(m_image, rotated);  
    flip(rotated, rotated, 1);

    cv::imwrite("out.jpg", rotated);

    Frame frame(static_cast<uint16_t>(m_image.cols), static_cast<uint16_t>(m_image.rows));
    for (uint16_t row = 0; row < static_cast<uint16_t>(m_image.rows); row++) 
    {
        for (uint16_t col = 0; col < static_cast<uint16_t>(m_image.cols); col++)
        {
            const auto& src_pixel = m_image.at<cv::Vec3b>(row, col);

            Pixel dst_pixel;
            dst_pixel.m_red = src_pixel[0];
            dst_pixel.m_green = src_pixel[1];
            dst_pixel.m_blue = src_pixel[2];

            frame.set(col, row, dst_pixel);
        }
    }

    Frame frame_r(static_cast<uint16_t>(rotated.cols), static_cast<uint16_t>(rotated.rows));
    for (uint16_t row = 0; row < static_cast<uint16_t>(rotated.rows); row++) 
    {
        for (uint16_t col = 0; col < static_cast<uint16_t>(rotated.cols); col++)
        {
            const auto& src_pixel = rotated.at<cv::Vec3b>(row, col);

            Pixel dst_pixel;
            dst_pixel.m_red = src_pixel[0];
            dst_pixel.m_green = src_pixel[1];
            dst_pixel.m_blue = src_pixel[2];

            frame_r.set(col, row, dst_pixel);
        }
    }

    auto rotat = frame.rotate90();

    if (rotat.getHeight() != frame_r.getHeight())
    {
        throw std::string("bad height");
    }

    if (rotat.getWidth() != frame_r.getWidth())
    {
        throw std::string("bad width");
    }

    cv::Mat m(rotat.getWidth(), rotat.getHeight(), m_image.type());

    for (uint16_t row = 0; row < frame_r.getWidth(); row++) 
    {
        for (uint16_t col = 0; col < frame_r.getHeight(); col++)
        {
            auto p1 = rotat.get(row, col);

            auto& dst_pixel = m.at<cv::Vec3b>(row, col);
            dst_pixel[0] = p1.m_red;
            dst_pixel[1] = p1.m_green;
            dst_pixel[2] = p1.m_blue;

            //auto p2 = frame_r.get(row, col);

            //if (!p1.coarseCompare(p2, 0))
            {
                //std::cout << row << ", " << col;
                //throw std::string("whyyyy");
            }
        }
    }

    cv::imwrite("out2.jpg", m);

    m_consumed = true;

    return frame;
}