#ifndef GCODEUPLOADER_STD_FILESYSTEM_HPP
#define GCODEUPLOADER_STD_FILESYSTEM_HPP

#if __cplusplus < 201700L
#   include <experimental/filesystem>
namespace std {
    namespace filesystem {
        using namespace std::experimental::filesystem;
    } // namespace filesystem
} // namespace std
#else
#   include <optional>
#endif

#endif //GCODEUPLOADER_STD_FILESYSTEM_HPP
