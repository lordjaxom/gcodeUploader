#ifndef GCODEUPLOADER_WX_CLIENTPTR_HPP
#define GCODEUPLOADER_WX_CLIENTPTR_HPP

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

        Type& GetValue() { return value_; }
        Type const& GetValue() const { return value_; }

    private:
        Type value_;
    };

    template< typename Type >
    Type& wxClientPtrCast( wxClientData* clientData )
    {
        return dynamic_cast< wxClientPtr< Type >& >( *clientData ).GetValue();
    }

    template< typename Type >
    Type const& wxClientPtrCast( wxClientData const* clientData )
    {
        return dynamic_cast< wxClientPtr< Type >& >( *clientData ).GetValue();
    }

} // namespace gct

#endif //GCODEUPLOADER_WX_CLIENTPTR_HPP
