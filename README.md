# Mosaic Decomposer

Mosaic decomposer is a tool for mosaic recongnition in videos or images. It uses simple appproach to identify major mismatches in the pixel's colors to collect potential line splits, having high amount of frames combined with extensible configurability of algorithm's parameters let's pretty accurately recognize composite videos or images.

## Dependencies
* **catch2** (optional) - for unit testing
* **docopt** - for convenient CLI arguments parsing
* **spdlog** - for convenient logging
* **opencv (v3.2 or higher)** - for reading images or video files

## Build instructions
    git clone git@github.com:valerijbielskij/mosaic_decomposer.git
    cd mosaic_decomposer
    mkdir build
    cd build
    python -m venv ~/py3venv
    source ~/py3venv/bin/activate # on windows use CALL
    pip3 install conan
    cmake .. -DUSE_CONAN_OPENCV=ON # pass OFF if you have opencv preinstalled, it might require setting OpenCV_DIR variable
    cmake --build .
    
## Execution
    Usage:
      decompose (video | image) <file-path> [--frames=<frames>] [--skip-front-lines=<front>] [--skip-back-lines=<back>] [--pixel-match=<pm>] [--color-match=<diff>] [--line-match=<lm>]
      decompose (-h | --help)
      decompose --version

    Options:
      -h --help                   Show this screen.
      video                       Specifies video decomposition mode, file path must be a valid video file.
      image                       Specifies image decomposition mode, file path must be a valid image file.
      --version                   Show version.
      --frames=<frames>           Amount of frames to analyze, 0 for all available [default: 0].
      --skip-front-lines=<front>  Amount of lines to be skipped from the front [default: 5].
      --skip-back-lines=<back>    Amount of lines to be skipped from the back [default: 5].
      --pixel-match=<pm>          Ratio used to decide whether a line shall be considered as a potential split line [default: 1.5].
      --color-match=<diff>        Minimum amount of color units that have to match [default: 80].
      --line-match=<lm>           Ratio used to decide whether a line shall be considered as false positive [default: 1.8].
    
    Exapmle:
        decompose video ~/input/mosaic-sample.mp4
        ....
        [2022-01-22 12:04:35.488] [info] (0  ; 0  ), 157x89
        [2022-01-22 12:04:35.488] [info] (157; 0  ), 160x89
        [2022-01-22 12:04:35.488] [info] (317; 0  ), 162x89
        [2022-01-22 12:04:35.488] [info] (479; 0  ), 161x89
        [2022-01-22 12:04:35.488] [info] (0  ; 89 ), 157x89
        [2022-01-22 12:04:35.488] [info] (157; 89 ), 160x89
        [2022-01-22 12:04:35.488] [info] (317; 89 ), 162x89
        [2022-01-22 12:04:35.488] [info] (479; 89 ), 161x89
        [2022-01-22 12:04:35.488] [info] (0  ; 178), 157x92
        [2022-01-22 12:04:35.488] [info] (157; 178), 160x92
        [2022-01-22 12:04:35.488] [info] (317; 178), 162x92
        [2022-01-22 12:04:35.488] [info] (479; 178), 161x92
        [2022-01-22 12:04:35.488] [info] (0  ; 270), 157x90
        [2022-01-22 12:04:35.488] [info] (157; 270), 160x90
        [2022-01-22 12:04:35.488] [info] (317; 270), 162x90
        [2022-01-22 12:04:35.488] [info] (479; 270), 161x90
    
## Tests
Minimalistic unit tests can be found in *tests/tests.cpp*.
Make sure to run cmake with:

    -DENABLE_TESTING=ON
then to execute tests run:

    ctest
or manually:

    test/tests
    
## Verified platforms
* **Windows 10** with msys2/mingw64/gcc11.2, opencv preinstalled
* **Windows 10** with msvc2022, opencv built with conan
* **Ubuntu 18.04** with gcc7.5, opencv preinstalled or built with conan

## Notes
* default parameters were derived from manual testing of couple mosaicked videos, videos/images of a very different quality/resolution might require completely different values for accurate operation
* *scripts/main.py* contains draft implementation in python but main parts of the solution are commented out and have to be rewritten
* test coverage is way too low, core algorithm is not covered, neither video or image provider classes

## Possible improvements
* video frame analysis could be done concurrently if performance is a concern
* it's possible to avoid transposing frame coordinates, potential split recognition could also learn to iterate vertically instead of always iterating horizontally
* opencv is heavyweight dependency, it could be reduced by using only some of it's submodules (videoio, core?) or switching to completely different library
* provide possibility to control amount of analyzed frames throughout the whole video, it's not very useful and efficient to analyze adjacent frames as their content barely changes, especially for videos with a high frame rate
