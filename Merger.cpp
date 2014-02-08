// Merger.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/Merger.h"

#include <ppbox/data/base/Error.h>
#include <ppbox/data/segment/SegmentSource.h>
#include <ppbox/data/segment/SegmentStrategy.h>
#include <ppbox/data/segment/SegmentBuffer.h>
#include <ppbox/data/strategy/ListStrategy.h>
using namespace ppbox::data;

#include <util/stream/UrlSource.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/logger/StringRecord.h>

#include <boost/bind.hpp>

namespace ppbox
{
    namespace merge
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.merge.Merger", framework::logger::Debug);

        Merger::Merger(
            boost::asio::io_service & io_svc, 
            ppbox::data::SegmentMedia & media)
            : MergerBase(io_svc)
            , media_(media)
            , buffer_(NULL)
            , source_(NULL)
            , strategy_(NULL)
            , open_state_(closed)
            , seek_pending_(false)
            , read_size_(4 * 1024)
        {
        }

        Merger::~Merger()
        {
        }

        void Merger::async_open(
            response_type const & resp)
        {
            resp_ = resp;
            boost::system::error_code ec;
            handle_async(ec);
        }

        void Merger::cancel(
            boost::system::error_code & ec)
        {
            if (source_) {
                source_->cancel(ec);
            }
            media_.cancel(ec);
        }

        void Merger::close(
            boost::system::error_code & ec)
        {
            media_.close(ec);

            if (buffer_) {
                delete buffer_;
                buffer_ = NULL;
            }
            if (source_) {
                source_->close(ec);
                util::stream::UrlSource * source = const_cast<util::stream::UrlSource *>(&source_->source());
                util::stream::UrlSourceFactory::destroy(source);
                delete source_;
                source_ = NULL;
            }
            open_state_ = closed;
        }

        void Merger::handle_async(
            boost::system::error_code const & ecc)
        {
            boost::system::error_code ec = ecc;
            if (ec) {
                LOG_WARN("[handle_async] media open, ec: " << ec.message());
                response(ec);
                return;
            }

            switch(open_state_) {
                case closed:
                    {
                        strategy_ = new ListStrategy(media_);
                        util::stream::UrlSource * source = 
                            util::stream::UrlSourceFactory::create(get_io_service(), media_.get_protocol(), ec);
                        if (source == NULL) {
                            source = util::stream::UrlSourceFactory::create(get_io_service(), media_.segment_protocol(), ec);
                        }
                        if (source) {
                            boost::system::error_code ec;
                            source->set_non_block(true, ec);
                            source_ = new ppbox::data::SegmentSource(*strategy_, *source);
                            source_->set_time_out(5000);
                            buffer_ = new SegmentBuffer(*source_, 10 * 1024 * 1024, 10240);
                        } else {
                            media_.get_io_service().post(
                                boost::bind(&Merger::response, this, ec));
                            break;
                        }
                    }
                    open_state_ = media_open;
                    media_.async_open(boost::bind(&Merger::handle_async, this, _1));
                    break;
                case media_open:
                    open_state_ = merger_open;
                    media_.get_info(media_info_, ec);
                    if (media_info_.bitrate == 0) {
                        if (media_info_.duration != 0) {
                            media_info_.bitrate = (boost::uint32_t)(media_info_.file_size * 8000 / media_info_.duration);
                        }
                    }
                    set_strategys();
                case merger_open:
                    buffer_->pause_stream();
                    if (!ec && byte_seek(0, ec)) {
                        buffer_->set_track_count(1);
                        open_state_ = opened;
                        response(ec);
                    } else if (ec == boost::asio::error::would_block) {
                        buffer_->async_prepare_some(0, 
                            boost::bind(&Merger::handle_async, this, _1));
                    } else {
                        open_state_ = opened;
                        response(ec);
                    }
                    break;
                default:
                    assert(0);
            }
        }

        void Merger::response(
            boost::system::error_code const & ec)
        {
            response_type resp;
            resp.swap(resp_);
            resp(ec);
        }

        bool Merger::byte_seek(
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
                LOG_INFO("[byte_seek] offset: " << offset 
                    << " segment: " << read_.index
                    << " position: " << read_.byte_range.pos);
                seek_pending_ = false;
                return true;
            }
        }

        bool Merger::reset(
            boost::system::error_code & ec)
        {
            return byte_seek(0, ec);
        }

        bool Merger::read(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (sample.memory) {
               buffer_->putback(sample.memory);
               sample.memory = NULL;
            }
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
                    sample.time = read_.time_range.big_pos() * 8000 / media_info_.bitrate;
                    sample.size = read_size_;
                    if (sample.size > read_.time_range.end - read_.time_range.pos)
                        sample.size = (size_t)(read_.time_range.end - read_.time_range.pos);
                    assert(sample.size > 0);
                    sample.data.clear();
                    if ((sample.memory = buffer_->fetch(0, read_.time_range.pos, sample.size, false, sample.data, ec))) {
                        read_.time_range.pos += sample.size;
                        break;
                    }
                    if (ec == boost::asio::error::eof && read_.time_range.end == invalid_size) {
                        read_.time_range.end = buffer_->read_segment().byte_range.end;
                        ec.clear();
                    }
                } else {
                    buffer_->read_next(read_.time_range.end, ec);
                    byte_seek(read_.time_range.big_end(), ec);
                }
            }

            return !ec;
        }

        bool Merger::free(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (sample.memory) {
                buffer_->putback(sample.memory);
                sample.memory = NULL;
            }
            ec.clear();
            return true;
        }

        bool Merger::fill_data(
            boost::system::error_code & ec)
        {
            buffer_->prepare_some(ec);
            return !ec;
        }

        void Merger::media_info(
            MediaInfo & info) const
        {
            boost::system::error_code ec;
            media_.get_info(info, ec);
            info.file_size = strategy_->time_size();
        }

        void Merger::stream_info(
            std::vector<StreamInfo> & streams) const
        {
            // the interface is reserved for future use
            // return nothing now
        }

        void Merger::stream_status(
            StreamStatus & info) const
        {
            info.byte_range.beg = 0;
            info.byte_range.end = strategy_->time_size();
            info.byte_range.pos = read_.time_range.big_pos();
            info.byte_range.buf = buffer_->out_position();

            info.time_range.beg = 0;
            info.time_range.end = ppbox::data::invalid_size;
            info.time_range.pos = info.byte_range.pos * 8000 / media_info_.bitrate;
            info.time_range.buf = info.byte_range.buf * 8000 / media_info_.bitrate;

            info.buf_ec = buffer_->last_error();
        }

        void Merger::data_stat(
            DataStat & stat) const
        {
            stat = *source_;
        }

        void Merger::add_strategy(
            ppbox::data::SegmentStrategy & strategy)
        {
            strategy_->insert(strategy);
        }

    } // namespace merge
} // namespace ppbox
