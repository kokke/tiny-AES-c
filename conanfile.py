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
    exports_sources = ["CMakeLists.txt", "*.c", '*.h', '*.hpp']

    _options_dict = {
        # enable AES128
        "AES128": [True, False],

        # enable AES192
        "AES192": [True, False],

        # enable AES256
        "AES256": [True, False],

        # enable AES encryption in CBC-mode of operation
        "CBC": [True, False],

        # enable the basic ECB 16-byte block algorithm
        "ECB": [True, False],

        # enable encryption in counter-mode
        "CTR": [True, False],
    }

    options = _options_dict

    default_options = {
        "AES128": True,
        "AES192": False,
        "AES256": False,
        "CBC": False,
        "ECB": False,
        "CTR": False
    }

    def build(self):
        cmake = CMake(self)

        for key in self._options_dict.keys():
            if self.options[key]:
                cmake.definitions["CMAKE_CFLAGS"].append(key)

        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include")
        self.copy("*.hpp", dst="include")
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["tiny-aes"]
