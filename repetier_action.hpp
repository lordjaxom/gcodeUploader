#ifndef GCODEUPLOADER_REPETIER_ACTION_HPP
#define GCODEUPLOADER_REPETIER_ACTION_HPP

#include <cstdint>
#include <iterator>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <json/value.h>

#include "repetier_client.hpp"

namespace gcu {
    namespace repetier {

        namespace detail {

            template< std::size_t I, typename Data, typename ...Handlers >
            auto invokeHandler( Data&& data, std::error_code& ec, std::tuple< Handlers... > const& handlers )
            {
                auto invoker = [&] { return std::get< I >( handlers )( std::forward< Data >( data ), ec ); };
                return !ec ? invoker() : decltype( invoker() )();
            }

            template< std::size_t I = 0, typename Data, typename ...Handlers >
            auto invokeHandlers( Data&& data, std::error_code& ec, std::tuple< Handlers... > const& handlers,
                                 std::enable_if_t< I == sizeof...( Handlers ) - 1 >* = nullptr )
            {
                return invokeHandler< I >( std::forward< Data >( data ), ec, handlers );
            }

            template< std::size_t I = 0, typename Data, typename ...Handlers >
            auto invokeHandlers( Data&& data, std::error_code& ec, std::tuple< Handlers... > const& handlers,
                                 std::enable_if_t< I != sizeof...( Handlers ) - 1 >* = nullptr )
            {
                auto handled = invokeHandler< I >( std::forward< Data >( data ), ec, handlers );
                auto subInvoker = [&] { return invokeHandlers< I + 1 >( std::move( handled ), ec, handlers ); };
                return !ec ? subInvoker() : decltype( subInvoker() )();
            }

            template< typename Data, typename Callback >
            void invokeCallback( Data&& data, std::error_code ec, Callback&& callback,
                                 std::enable_if_t< !std::is_same< std::remove_reference_t< Data >, Json::Value >::value >* = nullptr )
            {
                std::forward< Callback >( callback )( std::forward< Data >( data ), ec );
            }

            template< typename Data, typename Callback >
            void invokeCallback( Data&& data, std::error_code ec, Callback&& callback,
                                 std::enable_if_t<  std::is_same< std::remove_reference_t< Data >, Json::Value >::value >* = nullptr )
            {
                std::forward< Callback >( callback )( ec );
            }

            template< typename ...Handlers >
            class Handled
            {
            public:
                Handled( Client* client, Json::Value&& request, std::tuple< Handlers... >&& handlers )
                        : client_( client )
                        , request_( std::move( request ) )
                        , handlers_( std::move( handlers ) )
                {
                }

                template< typename Handler >
                auto handle( Handler&& handler ) &&
                {
                    auto handlers = std::tuple_cat( std::move( handlers_ ), std::forward_as_tuple( handler ) );
                    return Handled< Handlers..., Handler >( client_, std::move( request_ ), std::move( handlers ) );
                }

                template< typename Callback >
                void send( Callback&& callback ) &&
                {
                    client_->send( request_,
                            [callback = std::forward< Callback >( callback ), handlers = std::move( handlers_ )]
                            ( auto&& data, std::error_code ec ) {
                                auto handled = invokeHandlers( std::forward< decltype( data ) >( data ), ec, handlers );
                                invokeCallback( std::move( handled ), ec, callback );
                            } );
                }

            private:
                Client* client_;
                Json::Value request_;
                std::tuple< Handlers... > handlers_;
            };

        } // namespace detail

        class Action
        {
        public:
            Action( Client* client, char const* name )
                    : client_( client )
            {
                request_[ Json::StaticString( "action" ) ] = name;
            }

            Action printer( char const* value ) &&
            {
                request_[ Json::StaticString( "printer" ) ] = value;
                return std::move( *this );
            }

            Action arg( char const* name, char const* value ) &&
            {
                data_[ Json::StaticString( name ) ] = Json::StaticString( value );
                return std::move( *this );
            }

            template< typename T >
            Action arg( char const* name, T&& value ) &&
            {
                data_[ Json::StaticString( name ) ] = std::forward< T >( value );
                return std::move( *this );
            }

            template< typename Handler >
            auto handle( Handler&& handler ) &&
            {
                return detail::Handled< Handler >( client_, std::move( request_ ), std::forward_as_tuple( handler ) );
            }

            template< typename Callback >
            void send( Callback&& callback )
            {
                client_->send( request_,
                               [callback = std::forward< Callback >( callback )]
                               ( auto&& data, std::error_code ec ) {
                                    detail::invokeCallback( std::forward< decltype( data ) >( data ), ec, callback );
                                } );
            }

        private:
            Client* client_;
            Json::Value request_;
            Json::Value& data_ { request_[ Json::StaticString( "data" ) ] = Json::objectValue };
        };

        namespace action {

            inline auto checkOkFlag()
            {
                return []( Json::Value&& data, std::error_code& ec ) {
                    if ( !data[ Json::StaticString( "ok" ) ].asBool() ) {
                        ec = std::make_error_code( std::errc::invalid_argument ); // TODO
                    }
                    return std::move( data );
                };
            }

            inline auto resolveKey( char const* key )
            {
                return [key]( Json::Value&& data, std::error_code& ec ) {
                    return std::move( data[ Json::StaticString( key ) ] );
                };
            }

            template< typename Result, typename Handler >
            auto transform( Handler&& handler )
            {
                return [handler = std::forward< Handler >( handler )]
                        ( auto&& data, std::error_code& ec ) {
                            std::vector< Result > result;
                            std::transform( data.begin(), data.end(), std::back_inserter( result ), handler );
                            return std::move( result );
                        };
            }

        } // namespace action

    } // namespoace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_ACTION_HPP
