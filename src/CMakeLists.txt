
SET(lib_name decomposerlib)
SET(exec_name decomposer)

#find_package(fmt)
find_package(spdlog)
find_package(docopt)

#find_package(opencv)
#SET("OpenCV_DIR" "/usr/share/OpenCV")
find_package(OpenCV REQUIRED)


add_library(decomposerlib STATIC  imageframeprovideropencv.cpp videoframeprovideropencv.cpp mosaicdecomposer.cpp pixel.cpp frame.cpp)
target_include_directories(
  ${lib_name}
    PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    /mingw64/include/opencv4/
)
target_link_libraries(
  ${lib_name}
  PRIVATE spdlog::spdlog
)

add_executable(${exec_name} main.cpp)

target_link_libraries(
  ${exec_name}
  PRIVATE ${lib_name}
          project_options
          project_warnings
          docopt::docopt
          spdlog::spdlog
          #opencv::opencv
          ${OpenCV_LIBS}
          )
