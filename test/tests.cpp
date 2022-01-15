#include <catch2/catch.hpp>
#include "frame.h"


TEST_CASE("Pixel operations", "Pixel")
{
    Pixel pixel{123, 75, 255};

    SECTION( "trivial direct comparisons" ) 
    {
        REQUIRE (pixel.m_red == 123);
        REQUIRE (pixel.m_green == 75);
        REQUIRE (pixel.m_blue == 255);
    }

    SECTION( "coarse comparisons of same object" ) 
    {
        REQUIRE (pixel.coarseCompare(pixel, 0));

        REQUIRE (pixel.coarseCompare(pixel, 1));

        REQUIRE (pixel.coarseCompare(pixel, 100));

        REQUIRE (pixel.coarseCompare(pixel, 255));
    }

    SECTION( "coarse comparisons of similar objects" ) 
    {
        auto other = pixel;
        other.m_red += 10;

        REQUIRE (pixel.coarseCompare(other, 11));

        REQUIRE (pixel.coarseCompare(other, 10));

        REQUIRE (pixel.coarseCompare(other, 9) == false);

        other.m_green += 5;

        REQUIRE (pixel.coarseCompare(other, 16));

        REQUIRE (pixel.coarseCompare(other, 15));

        REQUIRE (pixel.coarseCompare(other, 14) == false);

        other.m_blue -= 100;

        REQUIRE (pixel.coarseCompare(other, 116));

        REQUIRE (pixel.coarseCompare(other, 115));

        REQUIRE (pixel.coarseCompare(other, 114) == false);

        REQUIRE (pixel.coarseCompare(other, 110) == false);
    } 
}

TEST_CASE("Frame operations", "Frame")
{
    Frame frame(10, 15);

    unsigned char count = 0;
    for (unsigned char i = 0; i < frame.getWidth(); i++)
    {
        for (unsigned char j = 0; j < frame.getHeight(); j++)
        {
            frame.set(i, j, Pixel{count, count, count});
            count++;
        }  
    }

    SECTION( "frame width" ) 
    {
        REQUIRE (frame.getWidth() == 10);
    }

    SECTION( "frame height" ) 
    {
        REQUIRE (frame.getHeight() == 15);
    }

    SECTION( "(0;0) corner pixels" ) 
    {
        REQUIRE (frame.get(0, 0).m_red == 0);
        REQUIRE (frame.get(0, 0).m_green == 0);
        REQUIRE (frame.get(0, 0).m_blue == 0);
    }

    SECTION( "(9;0) corner pixels" ) 
    {
        REQUIRE (frame.get(9, 0).m_red == 135);
        REQUIRE (frame.get(9, 0).m_green == 135);
        REQUIRE (frame.get(9, 0).m_blue == 135);
    }

    SECTION( "(9;14) corner pixels" ) 
    {
        REQUIRE (frame.get(9, 14).m_red == 149);
        REQUIRE (frame.get(9, 14).m_green == 149);
        REQUIRE (frame.get(9, 14).m_blue == 149);
    }

    SECTION( "(0;14) corner pixels" ) 
    {
        REQUIRE (frame.get(0, 14).m_red == 14);
        REQUIRE (frame.get(0, 14).m_green == 14);
        REQUIRE (frame.get(0, 14).m_blue == 14);
    }

    auto rotated = frame.rotate90();

    SECTION( "rotated (0;0) corner pixels" ) 
    {
        REQUIRE (rotated.get(0, 0).m_red == 14);
        REQUIRE (rotated.get(0, 0).m_green == 14);
        REQUIRE (rotated.get(0, 0).m_blue == 14);
    }

    SECTION( "rotated (9;0) corner pixels" ) 
    {
        REQUIRE (rotated.get(14, 0).m_red == 0);
        REQUIRE (rotated.get(14, 0).m_green == 0);
        REQUIRE (rotated.get(14, 0).m_blue == 0);
    }

    SECTION( "rotated (9;14) corner pixels" ) 
    {
        REQUIRE (rotated.get(14, 9).m_red == 135);
        REQUIRE (rotated.get(14, 9).m_green == 135);
        REQUIRE (rotated.get(14, 9).m_blue == 135);
    }

    SECTION( "rotated (0;14) corner pixels" ) 
    {
        REQUIRE (rotated.get(0, 9).m_red == 149);
        REQUIRE (rotated.get(0, 9).m_green == 149);
        REQUIRE (rotated.get(0, 9).m_blue == 149);
    }
}
