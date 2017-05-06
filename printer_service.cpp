#include <system_error>

#include "std/filesystem.hpp"

#include "printer_service.hpp"

namespace gcu {

    PrinterService::PrinterService( std::string const& hostname, std::uint16_t port, std::string const& apikey )
    {
        client_.events().printersChanged.connect( [this] {
            std::lock_guard< std::recursive_mutex > lock( mutex_ );
            listPrinters();
        } );
        client_.events().modelGroupsChanged.connect( [this]( auto const& printer ) {
            this->listModelGroups( printer );
        } );
        client_.events().modelsChanged.connect( [this]( auto const& printer ) {
            this->listModels( printer );
        } );

        client_.connect( hostname, port, apikey, [this]( std::error_code ec ) {
            std::lock_guard< std::recursive_mutex > lock( mutex_ );
            if ( success( ec ) ) {
                state_ = CONNECTED;
                listPrinters();
            }
        } );
    }

    void PrinterService::requestPrinters()
    {
        std::lock_guard< std::recursive_mutex > lock( mutex_ );
        if ( checkConnection() && printers_ ) {
            printersChanged( *printers_ );
        }
    }

    void PrinterService::requestModelGroups( std::string const& printer )
    {
        std::lock_guard< std::recursive_mutex > lock( mutex_ );
        if ( checkConnection() ) {
            auto it = modelGroups_.find( printer );
            if ( it != modelGroups_.end() ) {
                modelGroupsChanged( printer, it->second );
            }
        }
    }

    void PrinterService::requestModels( std::string const& printer )
    {
        std::lock_guard< std::recursive_mutex > lock( mutex_ );
        if ( checkConnection() ) {
            auto it = models_.find( printer );
            if ( it != models_.end() ) {
                modelsChanged( printer, it->second );
            }
        }
    }

    void PrinterService::addModelGroup(
            std::string const& printer, std::string const& modelGroup, std::function< void() > callback )
    {
        client_.addModelGroup( printer, modelGroup, [this, callback = std::move( callback )]( auto ec ) {
            std::lock_guard< std::recursive_mutex > lock( mutex_ );
            if ( this->success( ec ) && callback ) {
                callback();
            }
        } );
    }

    void PrinterService::delModelGroup(
            std::string const& printer, std::string const& modelGroup, bool deleteModels,
            std::function< void() > callback )
    {
        client_.delModelGroup( printer, modelGroup, deleteModels, [this, callback = std::move( callback ) ]( auto ec ) {
            std::lock_guard< std::recursive_mutex > lock( mutex_ );
            if ( this->success( ec ) && callback ) {
                callback();
            }
        } );
    }

    void PrinterService::removeModel( std::string const& printer, std::size_t id, std::function< void() > callback )
    {
        client_.removeModel( printer, id, [this, callback = std::move( callback )]( auto ec ) {
            std::lock_guard< std::recursive_mutex > lock( mutex_ );
            if ( this->success( ec ) && callback ) {
                callback();
            }
        } );
    }

    void PrinterService::moveModelToGroup(
            std::string const& printer, unsigned int modelId, std::string const& modelGroup,
            std::function< void() > callback )
    {
        client_.moveModelFileToGroup(
                printer, modelId, modelGroup, [this, callback = std::move( callback )]( auto ec ) {
                    std::lock_guard< std::recursive_mutex > lock( mutex_ );
                    if ( this->success( ec ) && callback ) {
                        callback();
                    }
                } );
    }

    void PrinterService::upload(
            std::string const& printer, std::string const& modelName, std::string const& modelGroup,
            std::filesystem::path const& gcodePath, std::function< void() > callback )
    {
        client_.upload(
                printer, modelName, modelGroup, gcodePath,
                [this, callback = std::move( callback )]( auto ec ) {
                    std::lock_guard< std::recursive_mutex > lock( mutex_ );
                    if ( this->success( ec ) && callback ) {
                        callback();
                    }
                } );
    }

    bool PrinterService::success( std::error_code ec )
    {
        errorCode_ = ec;
        if ( errorCode_ ) {
            state_ = CLOSED;
            connectionLost( errorCode_ );
            return false;
        }
        return true;
    }

    bool PrinterService::checkConnection()
    {
        if ( state_ == CLOSED ) {
            connectionLost( errorCode_ );
            return false;
        }
        return state_ == CONNECTED;
    }

    void PrinterService::listPrinters()
    {
        client_.listPrinter( [this]( std::vector< repetier::Printer > printers, std::error_code ec ) {
            std::lock_guard< std::recursive_mutex > lock( mutex_ );
            if ( success( ec ) ) {
                printers_.emplace( std::move( printers ) );
                printersChanged( *printers_ );
                listModelsAndModelGroups();
            }
        } );
    }

    void PrinterService::listModelsAndModelGroups()
    {
        modelGroups_.clear();
        models_.clear();
        for ( auto const& printer : *printers_ ) {
            listModelGroups( printer.slug() );
            listModels( printer.slug() );
        }
    }

    void PrinterService::listModelGroups( std::string const& printer )
    {
        client_.listModelGroups( printer, [this, printer]( auto&& modelGroups, auto ec ) {
            std::lock_guard< std::recursive_mutex > lock( mutex_ );
            if ( this->success( ec ) ) {
                auto it = modelGroups_.find( printer );
                if ( it == modelGroups_.end() ) {
                    it = modelGroups_.emplace( printer, std::move( modelGroups ) ).first;
                }
                else {
                    it->second = std::move( modelGroups );
                }
                modelGroupsChanged( printer, it->second );
            }
        } );
    }

    void PrinterService::listModels( std::string const& printer )
    {
        client_.listModels( printer, [this, printer]( auto&& models, auto ec ) {
            std::lock_guard< std::recursive_mutex > lock( mutex_ );
            if ( this->success( ec ) ) {
                auto it = models_.find( printer );
                if ( it == models_.end() ) {
                    it = models_.emplace( printer, std::move( models ) ).first;
                }
                else {
                    it->second = std::move( models );
                }
                modelsChanged( printer, it->second );
            }
        } );
    }

} // namespace gcu
