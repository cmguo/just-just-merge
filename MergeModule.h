// MergeModule.h

#ifndef _PPBOX_MERGE_MERGE_MODULE_H_
#define _PPBOX_MERGE_MERGE_MODULE_H_

#include <framework/string/Url.h>

#include <boost/thread/mutex.hpp>

namespace ppbox
{
    namespace data
    {
        class MediaBase;
    }

    namespace merge
    {

        class MergerBase;
        class Strategy;

        class MergeModule
            : public ppbox::common::CommonModuleBase<MergeModule>
        {
        public:
            MergeModule(
                util::daemon::Daemon & daemon);

            ~MergeModule();

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();

        public:
            void set_download_buffer_size(
                boost::uint32_t buffer_size);

        public:
            MergerBase * create(
                framework::string::Url const & play_link, 
                framework::string::Url const & config, 
                boost::system::error_code & ec);

            bool destroy(
                MergerBase * mergeer, 
                boost::system::error_code & ec);

            MergerBase * find(
                framework::string::Url const & play_link);

            MergerBase * find(
                ppbox::data::MediaBase const & media);

        private:
            struct MergeInfo;

        private:
            MergeInfo * priv_create(
                framework::string::Url const & play_link, 
                framework::string::Url const & config, 
                boost::system::error_code & ec);

            void priv_destroy(
                MergeInfo * info);

        private:
            // ≈‰÷√
            boost::uint32_t buffer_size_;

        private:
            std::vector<MergeInfo *> mergers_;
            boost::mutex mutex_;
        };

    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_MERGE_MODULE_H_
