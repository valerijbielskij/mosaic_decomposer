#pragma once

#include <vector>
#include <memory>
#include <utility>

#include "pixel.h"
#include "commondefinitions.h"

/*!
 * @brief Defines two dimensional array which maintains pixels of a frame.
 * @note it utilizes implicit sharing to avoid deep copies, 
 * however it detaches when modifications are performed on the copied object.
 */
class Frame
{
public:
    using PixelContainer = std::vector<std::vector<Pixel>>;
    using DimensionsType = common::DimensionsType;

    Frame(DimensionsType width, DimensionsType height);
    Frame(const Frame& other);

    Frame rotate90() const;

    void set(DimensionsType x, DimensionsType y, const Pixel& pixel);
    Pixel get(DimensionsType x, DimensionsType y) const;

    DimensionsType getWidth() const;
    DimensionsType getHeight() const;

private:
    enum class Transposition
    {
        None,
        Rotated90
    };

    Frame(const Frame& other, Transposition transposition);

    PixelContainer& get();
    const PixelContainer& get() const;

    DimensionsType getActualHeight() const;
    DimensionsType getActualWidth() const;

    std::pair<DimensionsType, DimensionsType> transpose(DimensionsType x, DimensionsType y) const;

    std::shared_ptr<PixelContainer> m_pixels;
    // ownership semantics imply only content ownership, not the allocated memory ownership
    bool m_owns_content;
    const Transposition m_transposition = Transposition::None;
};