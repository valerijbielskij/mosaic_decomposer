#include <memory>
#include <string>
#include <limits>

#include <spdlog/spdlog.h>
#include <docopt/docopt.h>

#include "videoframeprovideropencv.h"
#include "imageframeprovideropencv.h"
#include "mosaicdecomposer.h"

static constexpr auto VERSION = "0.1";

// clang-format off
static constexpr auto USAGE =
R"(Mosaic decomposer.

    Usage:
      decompose (video | image) <file_path> [--frames_to_analyze=<frames>] [--skip_front_lines=<front>] [--skip_back_lines=<back>] [--pixel_match=<pm>] [--color_match_diff=<diff>] [--line_match=<lm>]
      decompose (-h | --help)
      decompose --version

    Options:
      -h --help                     Show this screen.
      video                         Specifies video decomposition mode. file path must be a valid video file.
      image                         Specifies image decomposition mode. file path must be a valid image file.
      --version                     Show version.
      --frames_to_analyze=<frames>  Amount of frames to analyze, 0 for all available [default: 0].
      --skip_front_lines=<front>    Amount of lines to be skipped from the front [default: 5].
      --skip_back_lines=<back>      Amount of lines to be skipped from the back [default: 5].
      --pixel_match=<pm>            Ratio used to decide whether a line shall be considered as a potential split line [default: 1.4].
      --color_match_diff=<diff>     Minimum amount of color units that have to match [default: 100].
      --line_match=<lm>             Ratio used to decide whether a line shall be considered as false positive [default: 1.5].
)";
// clang-format on

struct ParseOptions
{
    std::filesystem::path m_file_path;
    bool m_is_video;

    ConfigParams m_config_params;
};

template <class DowncastedType>
// note that long long is chosen to extend the range as long, otherwise range check may fail on some platforms
DowncastedType downcastLong(long long value) 
{
    if (value < 0)
    {
        throw std::invalid_argument("value: " + std::to_string(value) + " is negative");
    }

    if (value >= static_cast<long long>(std::numeric_limits<DowncastedType>::max()))
    {
        throw std::invalid_argument("value: " + std::to_string(value) +
           " exceeds required range (" + std::to_string(std::numeric_limits<DowncastedType>::max()-1) + ")");
    }

    return static_cast<DowncastedType>(value);
}

ParseOptions parseArgs(std::map<std::string, docopt::value> args)
{
    ParseOptions options{};

    options.m_is_video = args["video"].asBool();
    options.m_file_path = args["<file_path>"].asString();

    options.m_config_params.m_frames_to_analyze = 
        downcastLong<decltype(options.m_config_params.m_frames_to_analyze)>(args["--frames_to_analyze"].asLong());
    options.m_config_params.m_skip_front_lines  =
        downcastLong<decltype(options.m_config_params.m_skip_front_lines)>(args["--skip_front_lines"].asLong());
    options.m_config_params.m_skip_back_lines = 
        downcastLong<decltype(options.m_config_params.m_skip_back_lines)>(args["--skip_back_lines"].asLong());

    options.m_config_params.m_minimum_pixel_match_ratio = std::stod(args["--pixel_match"].asString());
    options.m_config_params.m_minimum_color_match_diff =
        downcastLong<decltype(options.m_config_params.m_minimum_color_match_diff)>(args["--color_match_diff"].asLong());
    options.m_config_params.m_minimum_line_match_ratio = std::stod(args["--line_match"].asString());

    return options;
}

int main(int argc, const char **argv)
{
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        { std::next(argv), std::next(argv, argc) },
        true,
        std::string("Mosaic decomposer ") + VERSION);

    try
    {
        ParseOptions options = parseArgs(args);

        std::unique_ptr<FrameProviderInterface> frame_provider;

        if (options.m_is_video)
        {
            frame_provider = std::make_unique<VideoFrameProviderOpenCv>(options.m_file_path);
        }
        else
        {
            frame_provider = std::make_unique<ImageFrameProviderOpenCv>(options.m_file_path);
        }

        MosaicDecomposer decomposer(*frame_provider, options.m_config_params);

        const auto& ret = decomposer.calculateMosaicsDimensions();

        for (const auto& data : ret)
        {
            spdlog::info("({}; {}), {}x{}", data.m_x, data.m_y, data.m_width, data.m_height);
        }
    }
    catch(const std::string& exception)
    {
        spdlog::warn("exception caught in main: {}", exception);
    }
}
