from conans import ConanFile, CMake


class TinyAesCConan(ConanFile):
    name = "tiny-AES-c"
    version = "1.0.0"
    license = "MIT"
    author = "Torfinn Berset <torfinn@bloomlife.com>"
    url = "https://github.com/kokke/tiny-AES-c"
    description = "Small portable AES128/192/256 in C"
    topics = ("encryption", "crypto", "AES")
    settings = "os", "compiler", "build_type", "arch"

    generators = "cmake"
    exports_sources = ["CMakeLists.txt", "*.c", '*.h', '*.h']

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include")
        self.copy("*.hpp", dst="include")

        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["tiny-aes"]
