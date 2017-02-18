#ifndef GCODEUPLOADER_STD_OPTIONAL_HPP
#define GCODEUPLOADER_STD_OPTIONAL_HPP

#if __cplusplus < 201700L
#   include <experimental/optional>
namespace std {
    using std::experimental::optional;
    using std::experimental::make_optional;
    using std::experimental::bad_optional_access;
    using std::experimental::nullopt_t;
    using std::experimental::nullopt;
    using std::experimental::in_place_t;
    using std::experimental::in_place;
} // namespace std
#else
#   include <optional>
#endif

#endif //GCODEUPLOADER_STD_OPTIONAL_HPP
