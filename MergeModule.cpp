// MergerModule.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/MergeModule.h"
#include "ppbox/merge/Version.h"
#include "ppbox/merge/MergerBase.h"
#include "ppbox/merge/MergeError.h"
#include "ppbox/merge/Mp4Merger.h"

#include <ppbox/data/base/MediaBase.h>

#include <ppbox/common/UrlHelper.h>

#include <framework/timer/Timer.h>
#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

#include <boost/bind.hpp>
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.merge.MergeModule", framework::logger::Debug);

namespace ppbox
{
    namespace merge
    {

        struct MergeModule::MergeInfo
        {
            ppbox::data::MediaBase * media;
            MergerBase * merger;
            framework::string::Url play_link;
            error_code ec;

            MergeInfo()
                : media(NULL)
                , merger(NULL)
            {
            }

            struct Finder
            {
                Finder(
                    MergerBase * merger)
                    : merger_(merger)
                {
                }

                bool operator()(
                    MergeInfo const * info)
                {
                    return info->merger == merger_;
                }

            private:
                MergerBase * merger_;
            };
       };

        MergeModule::MergeModule(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<MergeModule>(daemon, "MergeModule")
        {
            buffer_size_ = 20 * 1024 * 1024;
        }

        MergeModule::~MergeModule()
        {
            boost::mutex::scoped_lock lock(mutex_);
            for (size_t i = mergers_.size() - 1; i != (size_t)-1; --i) {
                priv_destroy(mergers_[i]);
            }
        }

        error_code MergeModule::startup()
        {
            error_code ec;
            return ec;
        }

        void MergeModule::shutdown()
        {
            boost::mutex::scoped_lock lock(mutex_);
            error_code ec;
            for (size_t i = mergers_.size() - 1; i != (size_t)-1; --i) {
                mergers_[i]->merger->cancel(ec);
            }
        }

        MergerBase * MergeModule::create(
            framework::string::Url const & play_link, 
            framework::string::Url const & config, 
            error_code & ec)
        {
            MergeInfo * info = priv_create(play_link, config, ec);
            return info ? info->merger : NULL;
        }

        bool MergeModule::destroy(
            MergerBase * merger, 
            error_code & ec)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::vector<MergeInfo *>::const_iterator iter = 
                std::find_if(mergers_.begin(), mergers_.end(), MergeInfo::Finder(merger));
            //assert(iter != mergers_.end());
            if (iter == mergers_.end()) {
                ec = framework::system::logic_error::item_not_exist;
            } else {
                priv_destroy(*iter);
                ec.clear();
            }
            return !ec;
        }

        MergerBase * MergeModule::find(
            framework::string::Url const & play_link)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::vector<MergeInfo *>::const_iterator iter = mergers_.begin();
            for (; iter != mergers_.end(); ++iter) {
                if ((*iter)->play_link == play_link) {
                    return (*iter)->merger;
                }
            }
            return NULL;
        }

        MergerBase * MergeModule::find(
            ppbox::data::MediaBase const & media)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::vector<MergeInfo *>::const_iterator iter = mergers_.begin();
            for (; iter != mergers_.end(); ++iter) {
                if ((*iter)->media == &media) {
                    return (*iter)->merger;
                }
            }
            return NULL;
        }

        MergeModule::MergeInfo * MergeModule::priv_create(
            framework::string::Url const & play_link, 
            framework::string::Url const & config, 
            error_code & ec)
        {
            framework::string::Url playlink(play_link);
            ppbox::common::decode_url(playlink, ec);
            ppbox::data::MediaBase * media = ppbox::data::MediaBase::create(io_svc(), playlink, ec);
            MergerBase * merger = NULL;
            if (media) {
                merger = new Mp4Merger(io_svc(), *(ppbox::data::SegmentMedia *)media);
                if (merger == NULL) {
                    ec = error::format_not_match;
                } else {
                    ppbox::common::apply_config(merger->get_config(), config, "merge.");
                }
            }
            if (merger) {
                MergeInfo * info = new MergeInfo;
                info->media = media;
                info->merger = merger;
                info->play_link = playlink;
                boost::mutex::scoped_lock lock(mutex_);
                mergers_.push_back(info);
                return info;
            }
            return NULL;
        }

        void MergeModule::priv_destroy(
            MergeInfo * info)
        {
            MergerBase * merger = info->merger;
            if (merger)
                delete merger;
            if (info->media)
                delete info->media;
            mergers_.erase(
                std::remove(mergers_.begin(), mergers_.end(), info), 
                mergers_.end());
            delete info;
            info = NULL;
        }

        void MergeModule::set_download_buffer_size(
            boost::uint32_t buffer_size)
        {
            buffer_size_ = buffer_size;
        }

    } // namespace merge
} // namespace ppbox
