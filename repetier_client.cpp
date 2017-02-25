#include <functional>
#include <iostream>
#include <utility>

#include <asio/io_service.hpp>

#include <json/value.h>

#include "repetier_action.hpp"
#include "repetier_client.hpp"
#include "utility.hpp"

namespace gcu {
    namespace repetier {

        static std::string buildSocketUrl( std::string const& hostname, std::uint16_t port )
        {
            return util::str( "ws://", hostname, ':', port, "/socket" );
        }

        Client::Client( asio::io_service& service )
        {
            wsclient_.clear_access_channels( websocketpp::log::alevel::all );
            wsclient_.clear_error_channels( websocketpp::log::elevel::all );

            wsclient_.init_asio( &service );
            wsclient_.start_perpetual();
        }

        Client::~Client()
        {
            wsclient_.stop_perpetual();
            close( false );
        }

        void Client::connect(
                std::string const& hostname, std::uint16_t port, std::string const& apikey,
                ConnectHandler&& handler )
        {
            if ( status_ != CLOSED ) {
                throw std::invalid_argument( "connect() called on an already connected client" );
            }

            hostname_ = &hostname;
            port_ = port;
            apikey_ = &apikey;
            connectHandler_ = std::move( handler );

            connect();
        }

        void Client::connect()
        {
            std::error_code ec;
            auto connection = wsclient_.get_connection( buildSocketUrl( *hostname_, port_ ), ec );
            if ( ec ) {
                propagateError( ec );
                return;
            }

            connection->set_open_handler( [this] ( auto&& ) { this->handleOpen(); } );
            connection->set_fail_handler( [this] ( auto&& ) { this->handleFail(); } );
            connection->set_close_handler( [this] ( auto&& ) { this->handleClose(); } );
            connection->set_message_handler( [this] ( auto&&, auto const& message ) { this->handleMessage( message ); } );

            std::cerr << "INFO: Connecting to " << connection->get_uri()->str() << "\n";

            status_ = CONNECTING;
            wshandle_ = connection->get_handle();
            wsclient_.connect( connection );
        }

        bool Client::reconnect()
        {
            if ( errorCount_ >= retryCount_ ) {
                return false;
            }

            std::lock_guard< std::recursive_mutex > lock( actionMutex_ );

            std::cerr << "INFO: trying to reconnect to " << *hostname_ << ":" << port_ << "\n";

            ++errorCount_;
            if ( !actionQueue_.empty() ) {
                actionQueue_.front().pending = false;
            }
            status_ = CLOSED;
            connect();

            return true;
        }

        void Client::close()
        {
            close( true );
        }

        void Client::handleOpen()
        {
            std::cerr << "INFO: Connection established, logging in\n";

            makeAction( this, "login" )
                    .arg( "apikey", apikey_->c_str() )
                    .handle( action::checkOkFlag() )
                    .send( [this]( std::error_code ec ) {
                        if ( ec ) {
                            std::cerr << "ERROR: Login failed, closing connection\n";
                            close();
                        }
                        else {
                            std::cerr << "INFO: Login successful, connection ready\n";
                            status_ = CONNECTED;
                            errorCount_ = 0;
                        }
                        if ( connectHandler_ ) {
                            connectHandler_( ec );
                            connectHandler_ = nullptr;
                        }
                    } );
        }

        void Client::handleFail()
        {
            auto connection = wsclient_.get_con_from_hdl( wshandle_ );

            std::cerr << "ERROR: Connection failed: " << connection->get_ec() << "\n";

            if ( !reconnect() ) {
                propagateError( connection->get_ec() );
            }
        }

        void Client::handleClose()
        {
            auto connection = wsclient_.get_con_from_hdl( wshandle_ );
            if ( status_ != CLOSING ) {
                std::cerr << "ERROR: Connection closed by server: code "
                          << websocketpp::close::status::get_string( connection->get_remote_close_code())
                          << ", reason: " << connection->get_remote_close_reason() << "\n";

                if ( reconnect() ) {
                    return;
                }
            }
            status_ = CLOSED;
            propagateError( std::make_error_code( std::errc::invalid_argument ) ); // TODO
        }

        void Client::handleMessage( websocketclient::message_ptr message )
        {
            std::cerr << "<<< " << message->get_payload().substr( 0, 80 ) << "\n";

            auto response = jsonContext_.toJson( message->get_payload() );
            auto callbackId = response[ "callback_id" ].asLargestInt();
            if ( callbackId != -1 ) {
                handleActionResponse( callbackId, std::move( response ) );
            }
            else if ( response[ "eventList" ].asBool() ) {
                auto& eventList = response[ "data" ];
                std::for_each( eventList.begin(), eventList.end(), [this]( auto&& event ) {
                    this->handleEvent( std::move( event ) );
                } );
            }
            else {
                std::cerr << "WARN: Unknown response (neither callback nor events)\n";
            }
        }

        void Client::handleActionResponse( std::intmax_t callbackId, Json::Value&& response )
        {
            std::lock_guard< std::recursive_mutex > lock( actionMutex_ );

            if ( !actionQueue_.empty() ) {
                auto& action = actionQueue_.front();
                if ( action.pending && action.callbackId == callbackId ) {
                    action.handler( std::move( response[ "data" ] ), {} );
                    actionQueue_.pop_front();
                    return sendIfReady();
                }
            }

            std::cerr << "WARN: Received response to unrequested callback " << callbackId << ", ignoring message\n";
        }

        void Client::handleEvent( Json::Value&& event )
        {
            auto type = event[ "event" ];
            if ( type == "printerListChanged" ) {
                events_.printersChanged();
            }
            else if ( type == "modelGroupListChanged" ) {
                events_.modelGroupsChanged( event[ "printer" ].asString() );
            }
            else if ( type == "jobsChanged" ) {
                events_.modelsChanged( event[ "printer" ].asString() );
            }
        }

        void Client::send( Json::Value&& request, ActionHandler&& handler )
        {
            std::lock_guard< std::recursive_mutex > lock( actionMutex_ );

            auto it = request[ "action" ].asString() == "login" ? actionQueue_.begin() : actionQueue_.end();
            actionQueue_.emplace( it, ++nextCallbackId_, std::move( request ), std::move( handler ) );
            sendIfReady();
        }

        void Client::sendIfReady()
        {
            if ( !actionQueue_.empty() ) {
                auto& action = actionQueue_.front();
                if ( !action.pending ) {
                    auto message = jsonContext_.toString( action.request );
                    std::cerr << ">>> " << message.substr( 0, 80 ) << "\n";

                    auto connection = wsclient_.get_con_from_hdl( wshandle_ );
                    connection->send( message, websocketpp::frame::opcode::text );

                    action.pending = true;
                }
            }
        }

        void Client::close( bool checked )
        {
            if ( checked && status_ != CONNECTING && status_ != CONNECTED ) {
                throw std::invalid_argument( "close() called on already closed (or closing) client" );
            }
            if ( status_ == CLOSED ) {
                return;
            }

            if ( status_ == CLOSING ) {
                return forceClose();
            }
            status_ = CLOSING;

            std::error_code ec;
            wsclient_.close( wshandle_, websocketpp::close::status::normal, {}, ec );
            if ( ec ) {
                std::cerr << "WARN: Closing connection normally failed, close forced: " << ec.message() << "\n";
                forceClose();
            }
        }

        void Client::forceClose()
        {
            status_ = CLOSED;

            std::error_code ec;
            wsclient_.close( wshandle_, websocketpp::close::status::force_tcp_drop, {}, ec );
        }

        void Client::propagateError( std::error_code ec )
        {
            std::lock_guard< std::recursive_mutex > lock( actionMutex_ );

            if ( connectHandler_ ) {
                connectHandler_( ec );
            }

            std::for_each( actionQueue_.begin(), actionQueue_.end(), [&]( auto& action ) {
                action.handler( {}, ec );
            } );
            actionQueue_.clear();
        }

    } // namespace repetier
} // namespace gcu