#include <stdexcept>

#include <spdlog/spdlog.h>

#include "frame.h"

Frame::Frame(DimensionsType width, DimensionsType height): 
    m_pixels(std::make_shared<PixelContainer>(width, std::vector<Pixel>(height))),
    m_owns_content(true)
{
}

Frame::Frame(const Frame& other) : 
    m_pixels(other.m_pixels), 
    m_owns_content(false), 
    m_transposition(other.m_transposition)
{
}

Frame::Frame(const Frame& other, Transposition transposition) : 
    m_pixels(other.m_pixels), 
    m_owns_content(false), 
    m_transposition(transposition) 
{
}

Frame Frame::rotate90() const
{
    if (m_transposition != Transposition::None)
    {
        throw std::runtime_error("Consuequent transpositions are not supported");
    }

    Frame transposed(*this, Transposition::Rotated90);
    return transposed;
}

void Frame::set(DimensionsType x, DimensionsType y, const Pixel& pixel)
{
    auto [tx, ty] = transpose(x, y);

    if (tx >= getActualWidth())
    {
        spdlog::error("failed to set pixel for x={}, frame width is={}", x, getWidth());
        return;
    }

    if (ty >= getActualHeight())
    {
        spdlog::error("failed to set pixel for y={}, frame height is={}", y, getHeight());
        return;
    }

    if (m_pixels.use_count() == 0)
    {
        spdlog::critical("generic error occurred, internal container is malformed");
        return;
    }

    if (!m_owns_content)
    {
        if (m_pixels.use_count() == 1)
        {
            // if instance is the last owener of the memory then it can safely take over content ownership
            m_owns_content = true;
        }
        else
        {
            const auto copy = m_pixels;
            m_pixels = std::make_shared<PixelContainer>(*copy);

            m_owns_content = true;
        }
    }

    get()[tx][ty] = pixel;
}

Frame::PixelContainer& Frame::get()
{
    return *m_pixels.get();
}

const Frame::PixelContainer& Frame::get() const
{
    return *m_pixels.get();
}

Pixel Frame::get(DimensionsType x, DimensionsType y) const
{
    auto [tx, ty] = transpose(x, y);

    if (tx >= getActualWidth())
    {
        spdlog::error("failed to get pixel for x={}, frame width is={}", x, getWidth());
    }

    if (ty >= getActualHeight())
    {
        spdlog::error("failed to get pixel for y={}, frame height is={}", y, getHeight());
    }

    return get()[tx][ty];
}

Frame::DimensionsType Frame::getWidth() const 
{ 
    if (m_transposition == Transposition::Rotated90)
    {
        return getActualHeight();
    }
    return getActualWidth();
}

Frame::DimensionsType Frame::getHeight() const 
{ 
    if (m_transposition == Transposition::Rotated90) 
    {
        return getActualWidth();
    }
    return getActualHeight();
}

Frame::DimensionsType Frame::getActualHeight() const
{
    return static_cast<DimensionsType>(get()[0].size());
}

Frame::DimensionsType Frame::getActualWidth() const
{
    return static_cast<DimensionsType>(get().size());
}

std::pair<Frame::DimensionsType, Frame::DimensionsType> Frame::transpose(DimensionsType x, DimensionsType y) const
{
    if (m_transposition == Transposition::Rotated90)
    {
        if (x >= getActualHeight())
        {
            spdlog::error("x={} is out of range, width={}", x, getActualHeight());
            return {0, 0};
        }

        const DimensionsType transposed_x = y;
        const DimensionsType transposed_y = static_cast<DimensionsType>(getActualHeight() - x - 1);

        return {transposed_x, transposed_y};
    }

    return {x, y};
}
