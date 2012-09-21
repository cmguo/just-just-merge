// MergePlayer.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/MergePlayer.h"
#include "ppbox/merge/Merge.h"

#include <framework/logger/StreamRecord.h>
using namespace framework::logger;

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
using namespace boost::system;

namespace ppbox
{
    namespace merge
    {
        MergePlayer::MergePlayer()
            : merge_(NULL)
            , session_(NULL)
        {
        }


        MergePlayer::~MergePlayer()
        {

        }


        void MergePlayer::operator ()()
        {
            boost::system::error_code ec;
            assert(exit_);
            exit_ = false;
            playing_ = true;

            ec = (NULL == session_)?buffering():playing();

            exit_ = true;

            resp_(ec);
        }

        void MergePlayer::set(
            Merge *merge_tmp,
            ppbox::common::session_callback_respone const &resp,
            ppbox::common::Session* session)
        {
            merge_ = merge_tmp;
            resp_ = resp;
            session_ = session;
        }

        boost::system::error_code MergePlayer::buffering()
        {
            boost::system::error_code ec;

            while(exit_)
            {
                ec.clear();

                for (boost::int32_t ii = 0; ii < 10; ++ii)
                {
                    merge_->get_buffer_time();
                    if (ec/* == boost::asio::error::would_block*/)
                    {
                        break;
                    }
                    //demuxer_->get_buffer_time(ec,ec1);
                }
                boost::this_thread::sleep(boost::posix_time::milliseconds(100));
            }

            return ec;
        }

        boost::system::error_code MergePlayer::playing()
        {
            boost::system::error_code ec;

            {
                boost::uint32_t seek = session_->playlist_[0].beg_;
                if (seek != boost::uint32_t(-1))
                {
                    merge_->seek(seek,ec);
                }
                if(ec)
                    return ec;
            }


            while(!exit_)
            {
                if (!playing_)
                {
                    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
                }
                else
                {
                    ec.clear();
                    std::vector<boost::asio::const_buffer> buffers;
                    
                    std::size_t len=merge_->read(buffers,ec);
                    if(!ec)
                    {
                        size_t isend = 0;
                        isend = session_->sinks_[boost::uint32_t(-1)]->write_some(buffers,ec);
                    }else if(ec == boost::asio::error::would_block)
                    {//¶ÁÊý¾Ý would_block
                        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
                    }
                    else
                    {
                        break;
                    }
                }
            }

            return ec;
        }

    } // namespace mux
} // namespace ppbox
