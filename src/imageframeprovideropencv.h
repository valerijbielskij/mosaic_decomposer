
#pragma once

#include <filesystem>

#include <opencv2/core/mat.hpp>

#include "frameproviderinterface.h"

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