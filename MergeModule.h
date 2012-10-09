// MergeModule.h
#ifndef   _PPBOX_MERGE_MERGE_MODULE_
#define   _PPBOX_MERGE_MERGE_MODULE_

#include <framework/string/Url.h>

namespace ppbox
{
    namespace merge
    {
        class Merge;
        class MergeModule;

        struct MergeType
        {
            enum Enum
            {
                bighead,
                smallhead,
                full,
            };
        };

        struct StateType
        {
            enum Enum
            {
                opening,
                canceled,
                opened,
                closed,
            };
        };

        typedef boost::function<void (
            boost::system::error_code const &, 
            Merge *)
        > open_response_type;

        struct MergeInfo
        {
            std::size_t id;
            framework::string::Url playlink;
            Merge * merge;
            StateType::Enum state;
            open_response_type resp;

            struct MergeFinder
            {
                MergeFinder(std::size_t id)
                    : id_(id)
                {
                }

                bool operator()(MergeInfo const & info)
                {
                    if (info.id == id_) {
                        return true;
                    } else {
                        return false;
                    }
                }

                std::size_t id_;
            };
        };

        class MergeModule
            : public ppbox::common::CommonModuleBase<MergeModule>
        {
        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();

        public:
            MergeModule(util::daemon::Daemon & daemon);

            ~MergeModule();

            void async_open(
                framework::string::Url const & url, 
                MergeType::Enum const & type, 
                size_t & close_token, 
                open_response_type const & resp);

            boost::system::error_code close(
                std::size_t close_token, 
                boost::system::error_code & ec);

            Merge * find(framework::string::Url const & url);

        private:
            void open_callback(
                boost::system::error_code const & ec, 
                std::size_t const & close_token);

        private:
            boost::mutex mutex_;
            std::vector<MergeInfo> merge_infos_;
        };
    }
}

#endif
