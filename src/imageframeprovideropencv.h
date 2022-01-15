
#pragma once

#include "frameproviderinterface.h"
#include <opencv2/core/mat.hpp>
#include <filesystem>

class ImageFrameProviderOpenCv: public FrameProviderInterface
{
public:

    ImageFrameProviderOpenCv(const std::filesystem::path& file_path);
    ~ImageFrameProviderOpenCv() = default;

    bool isReady() const override;
    std::optional<Frame> getNext() override;

private:
    const cv::Mat m_image;
    bool m_consumed = false;
};