from conans import ConanFile


class MosaicDecomposerProject(ConanFile):
    # Note: options are copied from CMake boolean options.
    # When turned off, CMake sometimes passes them as empty strings.
    options = {
        "use_conan_opencv": ["ON", "OFF", ""]
    }
    name = "MosaicDecomposer"
    version = "0.1"
    requires = (
        "catch2/2.13.7",
        "docopt.cpp/0.6.2",
        "spdlog/1.9.2"
    )
    generators = "cmake", "gcc", "txt", "cmake_find_package"

    def requirements(self):
        if self.options.use_conan_opencv == "ON":
            self.requires("opencv/4.0.1@conan/stable")
            self.requires("bzip2/1.0.8")
            self.requires("libjpeg/9c")


