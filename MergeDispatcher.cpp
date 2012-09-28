// RtspSession.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/MergeDispatcher.h"
#include "ppbox/merge/BigheadMp4Merge.h"

#include <ppbox/data/MediaBase.h>
#include <ppbox/data/SegmentSource.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
using namespace framework::logger;

#include <boost/bind.hpp>
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("MergeDispatcher", 0);

namespace ppbox
{
    namespace merge
    {
        MergeDispatcher::MergeDispatcher(
            boost::asio::io_service & ios)
            : ppbox::common::Dispatcher(ios)
            , player_(new MergePlayer(ios))
            , merge_(new BigheadMp4Merge(ios,4*1024*1024))
        {
        }

        MergeDispatcher::~MergeDispatcher()
        {
            assert(!player_->is_working());
            delete player_;
            player_ = NULL;

            delete merge_;
            merge_ = NULL;
        }

        void MergeDispatcher::async_open_playlink(std::string const &playlink,ppbox::common::session_callback_respone const &resp)
        {
            LOG_INFO("[async_open_playlink] playlink:"<<playlink);
            merge_->async_open(framework::string::Url(playlink),NULL,resp);
        }


        void MergeDispatcher::cancel_open_playlink(boost::system::error_code& ec)
        {
            LOG_INFO("[cancel_open_playlink]");
            merge_->cancel(ec);
        }

        void MergeDispatcher::close_playlink(boost::system::error_code& ec)
        {
            LOG_INFO("[close_playlink]");
            merge_->close(ec);
        }

        void MergeDispatcher::pause_moive(boost::system::error_code& ec)
        {
            LOG_INFO("[pause_moive]");
            ec.clear();
        }

        void MergeDispatcher::resume_moive(boost::system::error_code& ec)
        {
            LOG_INFO("[resume_moive]");
            ec.clear();
        }

        void MergeDispatcher::async_play_playlink(ppbox::common::Session* session,ppbox::common::session_callback_respone const &resp)
        {
            LOG_INFO("[async_play_playlink]");
            player_->set(merge_,resp,session);
            post(boost::bind(&MergePlayer::doing,player_));
        }

        void MergeDispatcher::cancel_play_playlink(boost::system::error_code& ec)
        {
            LOG_INFO("[cancel_play_playlink]");
            player_->stop();
        }

        void MergeDispatcher::async_buffering(ppbox::common::Session* session,ppbox::common::session_callback_respone const &resp)
        {
            LOG_INFO("[async_buffering]");
            player_->set(merge_,resp);
            post(boost::bind(&MergePlayer::doing,player_));
        }

        void MergeDispatcher::cancel_buffering(boost::system::error_code& ec)
        {
            LOG_INFO("[cancel_buffering]");
            player_->stop();
        }

        boost::system::error_code MergeDispatcher::get_media_info(
            ppbox::common::MediaInfo & info)
        {
            LOG_INFO("[get_media_info]");
            boost::system::error_code ec;
            ppbox::data::MediaInfo infoBase;
            merge_->source()->media()->get_info(infoBase,ec);
            if(!ec)
            {
                info.duration = infoBase.duration;
                info.filesize = infoBase.file_size;
            }
            return ec;
        }

        boost::system::error_code MergeDispatcher::get_play_info(
            ppbox::common::PlayInfo & info)
        {
            LOG_INFO("[get_play_info]");
            boost::system::error_code ec;
            return ec;
        }

    } // namespace mux
} // namespace ppbox
