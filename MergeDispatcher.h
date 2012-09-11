// MergeDispatcher.h

#ifndef _PPBOX_MERGE_PATCHER_H_
#define _PPBOX_MERGE_PATCHER_H_

#include "ppbox/merge/MergePlayer.h"

#include <ppbox/common/Dispatcher.h>

#include <boost/asio/deadline_timer.hpp>


namespace ppbox
{
    namespace merge
    {  
        class Merge;

        class MergeDispatcher : public ppbox::common::Dispatcher
        {
        public:

            MergeDispatcher(
                util::daemon::Daemon & daemon);

            virtual ~MergeDispatcher();

//²¥·Å¿ØÖÆº¯Êý
        public:
            virtual void async_open_playlink(std::string const &playlink,ppbox::common::session_callback_respone const &resp) ;
            virtual void cancel_open_playlink(boost::system::error_code& ec) ;
            virtual void close_playlink(boost::system::error_code& ec) ;

            virtual void open_format(std::string const &format,boost::system::error_code& ec) ;
            virtual void close_format(boost::system::error_code& ec) ;

            virtual void pause_moive(boost::system::error_code& ec);
            virtual void resume_moive(boost::system::error_code& ec);

            virtual void async_play_playlink(ppbox::common::Session* session,ppbox::common::session_callback_respone const &resp);
            virtual void cancel_play_playlink(boost::system::error_code& ec);

            virtual void async_buffering(ppbox::common::Session* session,ppbox::common::session_callback_respone const &resp);
            virtual void cancel_buffering(boost::system::error_code& ec);


            virtual void async_wait(
                boost::uint32_t wait_timer
                , ppbox::common::session_callback_respone const &resp) ;

            virtual void cancel_wait(boost::system::error_code& ec) ;

            virtual boost::system::error_code get_media_info(
                ppbox::common::MediaInfo & info) ;

            virtual boost::system::error_code get_play_info(
                ppbox::common::PlayInfo & info);

        private:
            boost::asio::deadline_timer timer_;
            MergePlayer* player_;
            Merge*        merge_;
        };

    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_PATCHER_H_
