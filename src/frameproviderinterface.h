
#pragma once

#include <optional>
#include <memory>

class Frame;

class FrameProviderInterface
{
public:
    FrameProviderInterface() = default;
    virtual ~FrameProviderInterface() = default;

    virtual bool isReady() const;
    virtual std::optional<Frame> getNext() = 0;
};