
#pragma once

#include <opencv2/videoio.hpp>

#include "frameproviderinterface.h"

class VideoFrameProviderOpenCv: public FrameProviderInterface
{
public:
    VideoFrameProviderOpenCv(const std::string& file_path);
    ~VideoFrameProviderOpenCv() = default;

    bool isReady() const override;
    std::optional<Frame> getNext() override;

private:
    cv::VideoCapture m_video_capture;
};
