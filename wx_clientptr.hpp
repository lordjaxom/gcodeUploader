#ifndef GCODEUPLOADER_WX_CLIENTPTR_HPP
#define GCODEUPLOADER_WX_CLIENTPTR_HPP

#include <utility>

#include <wx/clntdata.h>

namespace gct {

    template< typename Type >
    class wxClientPtr
            : public wxClientData
    {
    public:
        template< typename ...Args >
        wxClientPtr( Args&&... args )
                : value_( std::forward< Args >( args )... )
        {
        }

        wxClientPtr( wxClientPtr const& ) = delete;

        operator Type&() { return value_; }
        operator Type const&() const { return value_; }

        Type& operator*() { return value_; }
        Type const& operator*() const { return value_; }

        Type* operator->() { return &value_; }
        Type const* operator->() const { return &value_; }

    private:
        Type value_;
    };

    template< typename Type >
    Type& wxClientPtrCast( wxClientData* clientData )
    {
        return *static_cast< wxClientPtr< Type >& >( *clientData );
    }

    template< typename Type >
    Type const& wxClientPtrCast( wxClientData const* clientData )
    {
        return *static_cast< wxClientPtr< Type > const& >( *clientData );
    }

} // namespace gct

#endif //GCODEUPLOADER_WX_CLIENTPTR_HPP
