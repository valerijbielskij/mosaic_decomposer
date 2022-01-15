#include <functional>
#include <iostream>

#include <spdlog/spdlog.h>
#include <docopt/docopt.h>

#include "videoframeprovideropencv.h"
#include "imageframeprovideropencv.h"
#include "mosaicdecomposer.h"

static constexpr auto USAGE =
  R"(Naval Fate.

    Usage:
          naval_fate ship new <name>...
          naval_fate ship <name> move <x> <y> [--speed=<kn>]
          naval_fate ship shoot <x> <y>
          naval_fate mine (set|remove) <x> <y> [--moored | --drifting]
          naval_fate (-h | --help)
          naval_fate --version
 Options:
          -h --help     Show this screen.
          --version     Show version.
          --speed=<kn>  Speed in knots [default: 10].
          --moored      Moored (anchored) mine.
          --drifting    Drifting mine.
)";

int main(int, const char **argv)
{
    VideoFrameProviderOpenCv image_provider(argv[1]); // TODO path
    //ImageFrameProviderOpenCv image_provider(argv[1]);
    MosaicDecomposer decomposer(image_provider);

try
{
      const auto& ret = decomposer.calculateMosaicsDimensions();
    //const auto& ret = decomposer.translate({0, 90, 180, 270, 360}, {0, 160, 320, 480, 640});

    for (const auto& data : ret)
    {
        //std::cout <<  data.m_x << " " << data.m_y << " " <<  data.m_width << " " << data.m_height;
        spdlog::info("({}; {}), {}x{}", data.m_x, data.m_y, data.m_width, data.m_height);
    }
}
catch(const std::string& e)
{
  std::cerr << e << '\n';
}



/*
  std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
    { std::next(argv), std::next(argv, argc) },
    true,// show help if requested
    "Naval Fate 2.0");// version string

  for (auto const &arg : args) {
    std::cout << arg.first << "=" << arg.second << std::endl;
  }
  */

  //Use the default logger (stdout, multi-threaded, colored)
  //spdlog::info("Hello, {}!", "World");

  //fmt::print("Hello, from {}\n", "{fmt}");
}
