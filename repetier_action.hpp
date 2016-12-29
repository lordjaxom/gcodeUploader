#ifndef GCODEUPLOADER_REPETIER_ACTION_HPP
#define GCODEUPLOADER_REPETIER_ACTION_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Json {
    class Value;
} // namespace Json

namespace gcu {
    namespace repetier {

        class Action
        {
        public:
            Action( char const* name );
            Action( Action const& ) = delete;
            virtual ~Action() = 0;

            virtual Json::Value createOutgoing() const;
            virtual void handleResponse( Json::Value const& response ) const;

        protected:
            virtual void fillOutgoingData( Json::Value& data ) const {}
            virtual void handleResponseData( Json::Value const& data ) const {}

        private:
            char const* name_;
        };

        class LoginAction
                : public Action
        {
        public:
            using Callback = std::function< void () >;

            LoginAction( std::string apikey, Callback callback );

        protected:
            virtual void fillOutgoingData( Json::Value& data ) const override;
            virtual void handleResponseData( Json::Value const& data ) const override;

        private:
            std::string apikey_;
            Callback callback_;
        };

        class PrinterAction
            : public Action
        {
        public:
            PrinterAction( char const* name, std::string printer );

            virtual Json::Value createOutgoing() const override;

        private:
            std::string printer_;
        };

        class ListModelGroupsAction
                : public PrinterAction
        {
        public:
            using Callback = std::function< void ( std::vector< std::string > ) >;

            ListModelGroupsAction( std::string printer, Callback callback );

        protected:
            virtual void handleResponseData( Json::Value const& data ) const override;

        private:
            Callback callback_;
        };

    } // namespace repetier
} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_ACTION_HPP
