SET(lib_name decomposerlib)
SET(exec_name decompose)

find_package(spdlog)
find_package(docopt)
find_package(OpenCV REQUIRED)

if (USE_CONAN_OPENCV)
  set(MY_OPENCV_LIB opencv::opencv)
else()
  set(MY_OPENCV_LIB ${OpenCV_LIBS})
endif(USE_CONAN_OPENCV)

set(SOURCES imageframeprovideropencv.cpp videoframeprovideropencv.cpp mosaicdecomposer.cpp pixel.cpp frame.cpp)
set(HEADERS commondefinitions.h configparams.h frame.h frameproviderinterface.h	imageframeprovideropencv.h mosaicdecomposer.h pixel.h videoframeprovideropencv.h)

add_library(decomposerlib STATIC ${SOURCES} ${HEADERS})
target_include_directories(
  ${lib_name}
    PRIVATE
      ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(
  ${lib_name}
    PRIVATE 
      spdlog::spdlog
      project_options
      project_warnings
      ${MY_OPENCV_LIB}
)

add_executable(${exec_name} main.cpp)

target_link_libraries(
  ${exec_name}
    PRIVATE ${lib_name}
      project_options
      project_warnings
      docopt::docopt
      spdlog::spdlog
	  ${MY_OPENCV_LIB}
)
