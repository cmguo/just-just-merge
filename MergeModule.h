// MergeModule.h

#ifndef _PPBOX_MERGE_MERGE_MODULE_H_
#define _PPBOX_MERGE_MERGE_MODULE_H_

#include <framework/string/Url.h>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/function.hpp>

namespace ppbox
{
    namespace merge
    {

        class MergerBase;

        class MergeModule
            : public ppbox::common::CommonModuleBase<MergeModule>
        {
        public:
            typedef boost::function<void (
                boost::system::error_code const &, 
                MergerBase *)
            > open_response_type;

        public:
            MergeModule(
                util::daemon::Daemon & daemon);

            ~MergeModule();

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();

        public:
            MergerBase * open(
                framework::string::Url const & play_link, 
                size_t & close_token, 
                boost::system::error_code & ec);

            void async_open(
                framework::string::Url const & url, 
                size_t & close_token, 
                open_response_type const & resp);

            boost::system::error_code close(
                std::size_t close_token, 
                boost::system::error_code & ec);

            MergerBase * find(
                framework::string::Url const & url);

        private:
            struct MergeInfo;

        private:
            MergeInfo * create(
                framework::string::Url const & play_link, 
                open_response_type const & resp, 
                boost::system::error_code & ec);

            void async_open(
                boost::mutex::scoped_lock & lock, 
                MergeInfo * info);

            void handle_open(
                boost::system::error_code const & ec,
                MergeInfo * info);

            boost::system::error_code close_locked(
                MergeInfo * info, 
                bool inner_call, 
                boost::system::error_code & ec);

            boost::system::error_code close(
                MergeInfo * info, 
                boost::system::error_code & ec);

            boost::system::error_code cancel(
                MergeInfo * info, 
                boost::system::error_code & ec);

            void destory(
                MergeInfo * info);

        private:
            std::vector<MergeInfo *> mergers_;
            boost::mutex mutex_;
            boost::condition_variable cond_;
        };

    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_MERGE_MODULE_H_
