SET(lib_name decomposerlib)
SET(exec_name decomposer)

find_package(spdlog)
find_package(docopt)
find_package(opencv REQUIRED)

add_library(decomposerlib STATIC imageframeprovideropencv.cpp videoframeprovideropencv.cpp mosaicdecomposer.cpp pixel.cpp frame.cpp)
target_include_directories(
  ${lib_name}
    PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(
  ${lib_name}
  PRIVATE spdlog::spdlog
)

add_executable(${exec_name} main.cpp)

if (USE_CONAN_OPENCV)
  set(CONAN_OPENCV_LIB opencv::opencv)
else()
  set(CONAN_OPENCV_LIB ${OpenCV_LIBS})
endif(USE_CONAN_OPENCV)

target_link_libraries(
  ${exec_name}
  PRIVATE ${lib_name}
          project_options
          project_warnings
          docopt::docopt
          spdlog::spdlog
          ${CONAN_OPENCV_LIB}
)
