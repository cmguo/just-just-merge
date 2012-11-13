// MergerBase.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/MergerBase.h"

#include <ppbox/data/SourceError.h>
#include <ppbox/data/SegmentSource.h>
#include <ppbox/data/SegmentStrategy.h>
#include <ppbox/data/SegmentBuffer.h>
#include <ppbox/data/strategy/ListStrategy.h>
using namespace ppbox::data;

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/logger/StringRecord.h>

#include <boost/bind.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("MergerBase", framework::logger::Debug);

namespace ppbox
{
    namespace merge
    {

        MergerBase::MergerBase(
            boost::asio::io_service & io_svc, 
            ppbox::data::SegmentMedia & media)
            : media_(media)
            , buffer_(NULL)
            , source_(NULL)
            , strategy_(NULL)
            , seek_pending_(false)
            , read_size_(4 * 1024)
        {
            strategy_ = new ListStrategy(media);
            ppbox::data::SourceBase * source = 
                ppbox::data::SourceBase::create(media_.get_io_service(), media_);
            boost::system::error_code ec;
            source->set_non_block(true, ec);
            source_ = new ppbox::data::SegmentSource(*strategy_, *source);
            buffer_ = new SegmentBuffer(*source_, 10 * 1024 * 1024, 10240);
        }

        MergerBase::~MergerBase()
        {
            if (buffer_) {
                delete buffer_;
                buffer_ = NULL;
            }
            if (source_) {
                ppbox::data::SourceBase * source = (ppbox::data::SourceBase *)&source_->source();
                ppbox::data::SourceBase::destroy(source);
                delete source_;
                source_ = NULL;
            }
        }

        void MergerBase::async_open(
            response_type const & resp)
        {
            resp_ = resp;
            media_.async_open(boost::bind(&MergerBase::handle_async, this, _1));
        }

        void MergerBase::cancel(
            boost::system::error_code & ec)
        {
            media_.cancel(ec);
        }

        void MergerBase::close(
            boost::system::error_code & ec)
        {
            media_.close(ec);
            source_->close(ec);
        }

        void MergerBase::handle_async(
            boost::system::error_code const & ec)
        {
            boost::system::error_code lec = ec;
            if (ec) {
                LOG_WARN("[handle_async] media open, ec: " << ec.message());
            } else {
                media_.get_info(media_info_, lec);
                if (media_info_.bitrate == 0) {
                    if (media_info_.duration != 0) {
                        media_info_.bitrate = media_info_.file_size / media_info_.duration;
                    }
                }
                set_strategys();
                byte_seek(0, lec);
                if (lec == boost::asio::error::would_block)
                    lec.clear();
            }
            response(lec);
        }

        void MergerBase::response(
            boost::system::error_code const & ec)
        {
            response_type resp;
            resp.swap(resp_);
            resp(ec);
        }

        bool MergerBase::byte_seek(
            boost::uint64_t offset, 
            boost::system::error_code & ec)
        {
            if (!strategy_->time_seek(offset, read_, ec))
                return false;
            if (read_.time_range.beg == invalid_size) {
                read_.time_range.big_offset += invalid_size;
                read_.time_range.pos = 0;
            }
            read_.byte_range.pos = read_.time_range.pos;
            if (!buffer_->seek(read_, ec)) {
                return false;
            }
            if (read_.time_range.beg == invalid_size) {
                seek_pending_ = true;
                ec = boost::asio::error::would_block;
                return false;
            } else {
                seek_pending_ = false;
                return true;
            }
        }

        bool MergerBase::reset(
            boost::system::error_code & ec)
        {
            return byte_seek(0, ec);
        }

        bool MergerBase::read(
            Sample & sample,
            boost::system::error_code & ec)
        {
            buffer_->drop_to(read_.time_range.pos, ec);
            buffer_->prepare_some(ec);

            if (seek_pending_ && !byte_seek(read_.time_range.big_pos(), ec)) {
                //if (ec == boost::asio::error::would_block) {
                //    block_on();
                //}
                return false;
            }

            ec.clear();

            while (!ec) {
                if (read_.time_range.end > read_.time_range.pos) {
                    sample.time = read_.time_range.big_pos() / media_info_.bitrate;
                    sample.size = read_size_;
                    if (sample.size > read_.time_range.end - read_.time_range.pos)
                        sample.size = (size_t)(read_.time_range.end - read_.time_range.pos);
                    assert(sample.size > 0);
                    sample.data.clear();
                    if (buffer_->fetch(read_.time_range.pos, sample.size, sample.data, ec)) {
                        read_.time_range.pos += sample.size;
                        break;
                    }
                    if (ec == boost::asio::error::eof && read_.time_range.end == invalid_size) {
                        read_.time_range.end = buffer_->read_segment().byte_range.end;
                        ec.clear();
                    }
                } else {
                    buffer_->drop_all(read_.time_range.end, ec);
                    byte_seek(read_.time_range.big_end(), ec);
                }
            }

            return !ec;
        }

        void MergerBase::media_info(
            MediaInfo & info)
        {
            boost::system::error_code ec;
            media_.get_info(info, ec);
        }

        void MergerBase::play_info(
            PlayInfo & info)
        {
            info.byte_range.beg = 0;
            info.byte_range.pos = read_.time_range.big_pos();
            info.byte_range.end = strategy_->time_size();
        }

        boost::uint64_t MergerBase::get_buffer_size()
        {
            return buffer_->in_avail();
        }

        boost::uint64_t MergerBase::get_buffer_time(
            boost::system::error_code & ec, 
            boost::system::error_code & ec_buf)
        {
            buffer_->prepare_some(ec_buf);
            return buffer_->in_avail() * media_info_.bitrate;
        }

        void MergerBase::add_strategy(
            ppbox::data::SegmentStrategy & strategy)
        {
            strategy_->insert(strategy);
        }

    } // namespace merge
} // namespace ppbox
