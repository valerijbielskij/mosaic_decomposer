#include "frame.h"
#include <stdexcept>
#include <iostream>

Frame::Frame(DimensionsType width, DimensionsType height): m_pixels(std::make_shared<PixelContainer>(width, std::vector<Pixel>(height)))
{
}

Frame::Frame(const Frame& other, Transposition transposition) : m_pixels(other.m_pixels), m_transposition(transposition)  {}

Frame Frame::rotate90() const
{
    if (m_transposition != Transposition::None)
    {
        throw std::runtime_error("Consuequent transpositions not supported");
    }

    Frame transposed(*this, Transposition::Rotated90);
    return transposed;
}

void Frame::set(DimensionsType x, DimensionsType y, const Pixel& pixel)
{
    transpose(x, y);

    if (x >= getActualWidth())
    {
        // TODO log
        std::cout << "my bad\n";
        return;
    }

    if (y >= getActualHeight())
    {
        // TODO log
        std::cout << "bad luck\n";
        return;
    }

    get()[x][y] = pixel;
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
    auto tx = x;
    auto ty = y;
    transpose(x, y);

    if (x >= getActualWidth())
    {
        // TODO spdlog
        std::cout << "a bad x" << x << " " << getActualWidth() << std::endl;
    }

    if (y >= getActualHeight())
    {
        // TODO spdlog
        std::cout << "a bad y" << x << " " << getActualHeight() << std::endl;
    }

    return get()[x][y];
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

void Frame::transpose(DimensionsType& x, DimensionsType& y) const
{
    if (m_transposition == Transposition::Rotated90)
    {
        if (x >= getActualHeight())
        {
            std::cout << "OMG\n";
            return;
        }

        const DimensionsType transposed_x = y;
        const DimensionsType transposed_y = static_cast<DimensionsType>(getActualHeight() - x - 1);

        x = transposed_x;
        y = transposed_y;
    }
}
