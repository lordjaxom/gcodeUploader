#ifndef GCODEUPLOADER_REPETIER_ACTION_HPP
#define GCODEUPLOADER_REPETIER_ACTION_HPP

#include <cstdint>
#include <string>
#include <utility>

#include <json/value.h>

namespace gcu {
    namespace repetier {

        namespace action {

            template< typename ...Handlers >
            class Handled;

            template< typename Handler0 >
            class Handled< Handler0 >
            {
            public:
                Handled( Handled<>&& previous, Handler0&& handler0 )
                        : handler0_( std::move( handler0 ) )
                {
                }

                template< typename Callback >
                void send( Callback&& callback )
                {
                    auto f = [this, callback]( Json::Value&& response, std::error_code ec ) {
                        auto handled = ec ? std::result_of_t< Handler0( Json::Value&&, std::error_code& ) >() : handler0_( response, ec );
                        callback( std::move( handled ), ec );
                    };

                    Json::Value response;
                    std::error_code ec;
                    f( std::move( response ), ec );
                }

            private:
                Handler0 handler0_;
            };

            template< typename ...Handlers >
            class Handled
            {
            public:
                Handled( Json::Value&& request )
                        : request_( std::move( request ) )
                {
                }

                template< typename Handler >
                auto handle( Handler&& handler )
                {
                    return Handled< Handler >( std::move( *this ), std::forward< Handler >( handler ) );
                }

                template< typename Callback >
                void send( Callback&& callback )
                {
                    auto f = [callback]( Json::Value&& response, std::error_code ec ) {
                        callback( ec );
                    };

                    Json::Value response;
                    std::error_code ec;
                    f( std::move( response ), ec );
                }

            private:
                Json::Value request_;
            };

        } // namespace action

        class Action
        {
        public:
            Action( std::intmax_t callbackId, char const* name )
            {
                request_[ Json::StaticString( "callback_id" ) ] = callbackId;
                request_[ Json::StaticString( "action" ) ] = name;
            }

            Action& printer( char const* value )
            {
                request_[ Json::StaticString( "printer" ) ] = value;
                return *this;
            }

            Action& arg( char const* name, char const* value )
            {
                data_[ Json::StaticString( name ) ] = Json::StaticString( value );
                return *this;
            }

            template< typename T >
            Action& arg( char const* name, T&& value )
            {
                data_[ Json::StaticString( name ) ] = std::forward< T >( value );
                return *this;
            }

            template< typename Handler >
            auto handle( Handler&& handler )
            {
                return action::Handled<>( std::move( request_ ) ).handle( std::forward< Handler >( handler ) );
            }

            template< typename Callback >
            void send( Callback&& callback )
            {
                return action::Handled<>( std::move( request_ ) ).send( std::forward< Callback >( callback ) );
            }

        private:
            Json::Value request_;
            Json::Value& data_ { request_[ Json::StaticString( "data" ) ] = Json::objectValue };
        };

    } // namespoace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_ACTION_HPP
