#ifndef GCODEUPLOADER_FILESYSTEM_HPP
#define GCODEUPLOADER_FILESYSTEM_HPP

#if __cplusplus < 201700L
#   include <experimental/filesystem>
namespace std {
    namespace filesystem {
        using namespace std::experimental::filesystem;
    } // namespace filesystem
} // namespace std
#else
#   include <filesystem>
#endif

#endif //GCODEUPLOADER_FILESYSTEM_HPP
