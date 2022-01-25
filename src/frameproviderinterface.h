
#pragma once

#include <optional>

class Frame;

class FrameProviderInterface
{
public:
    FrameProviderInterface() = default;
    virtual ~FrameProviderInterface() = default;

    virtual bool isReady() const = 0;
    virtual std::optional<Frame> getNext() = 0;
};
