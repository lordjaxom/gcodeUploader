#ifndef GCODEUPLOADER_REPETIER_ACTION_HPP
#define GCODEUPLOADER_REPETIER_ACTION_HPP

#include <cstdint>
#include <functional>
#include <string>

namespace Json {
    class Value;
} // namespace Json

namespace gcu {
    namespace repetier {

        class Action
        {
        public:
            Action( std::intmax_t callbackId, char const* action );
            Action( Action const& ) = delete;
            virtual ~Action() = 0;

            std::intmax_t callbackId() const { return callbackId_; }
            char const* action() const { return action_; }

            void handle( Json::Value const& value ) const;

            void toJson( Json::Value& value ) const;

        protected:
            virtual void doHandle( Json::Value const& value ) const = 0;
            virtual void doToJson( Json::Value& data ) const = 0;

        private:
            std::intmax_t callbackId_;
            char const* action_;
        };

        class LoginAction
                : public Action
        {
        public:
            using Callback = std::function< void ( char const* ) >;

            LoginAction( std::intmax_t callbackId, std::string apikey, Callback callback );

        protected:
            virtual void doHandle( Json::Value const& value ) const override;
            virtual void doToJson( Json::Value& data ) const override;

        private:
            std::string apikey_;
            Callback callback_;
        };

    } // namespace repetier
} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_ACTION_HPP
