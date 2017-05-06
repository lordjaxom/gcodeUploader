#ifndef GCODEUPLOADER_VARIANT_HPP
#define GCODEUPLOADER_VARIANT_HPP

#if __cplusplus < 201700L
#   include <mpark/variant.hpp>
namespace std {
    using mpark::variant;
    using mpark::monostate;
    using mpark::bad_variant_access;
    using mpark::variant_size;
    using mpark::variant_size_v;
    using mpark::variant_alternative;
    using mpark::variant_alternative_t;
    using mpark::variant_npos;
    using mpark::visit;
    using mpark::holds_alternative;
    using mpark::get_if;
} // namespace std
#else
#   include <variant>
#endif

#endif //GCODEUPLOADER_VARIANT_HPP
