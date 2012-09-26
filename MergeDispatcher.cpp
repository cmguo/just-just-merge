// RtspSession.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/MergeDispatcher.h"
#include "ppbox/merge/BigheadMp4Merge.h"

#include <framework/logger/StreamRecord.h>
using namespace framework::logger;

#include <boost/bind.hpp>
using namespace boost::system;

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
            merge_->async_open(framework::string::Url(playlink),NULL,resp);
        }


        void MergeDispatcher::cancel_open_playlink(boost::system::error_code& ec)
        {
            merge_->cancel(ec);
        }

        void MergeDispatcher::close_playlink(boost::system::error_code& ec)
        {
            merge_->close(ec);
        }

        void MergeDispatcher::pause_moive(boost::system::error_code& ec)
        {
            ec.clear();
        }

        void MergeDispatcher::resume_moive(boost::system::error_code& ec)
        {
            ec.clear();
        }

        void MergeDispatcher::async_play_playlink(ppbox::common::Session* session,ppbox::common::session_callback_respone const &resp)
        {
            player_->set(merge_,resp,session);
            post(*player_);
        }

        void MergeDispatcher::cancel_play_playlink(boost::system::error_code& ec)
        {
            player_->stop();
        }

        void MergeDispatcher::async_buffering(ppbox::common::Session* session,ppbox::common::session_callback_respone const &resp)
        {
            player_->set(merge_,resp);
            post(*player_);
        }

        void MergeDispatcher::cancel_buffering(boost::system::error_code& ec)
        {
            player_->stop();
        }

        boost::system::error_code MergeDispatcher::get_media_info(
            ppbox::common::MediaInfo & info)
        {
            return boost::system::error_code();
        }

        boost::system::error_code MergeDispatcher::get_play_info(
            ppbox::common::PlayInfo & info)
        {
            return boost::system::error_code();
        }

    } // namespace mux
} // namespace ppbox
