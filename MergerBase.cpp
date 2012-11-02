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
            ppbox::data::MediaBase & media)
            : media_(media)
            , source_(NULL)
            , buffer_(NULL)
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
                ppbox::data::SourceBase::destory(source);
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

/*
        void MergerBase::change_source(SegmentStrategy * strategy)
        {
            if (segment_source_) {
                error_code ec;
                segment_source_->close(ec);
                delete segment_source_;
            }
            segment_source_ = new SegmentSource(*strategy, *source_);
            segment_source_->set_max_try(3);
            segment_source_->reset();
        }

        error_code MergerBase::seek(
            std::size_t offset, 
            error_code & ec)
        {
            if (offset < size()) {
                if (offset > (cur_offset() - cycle_buffers_->in_avail()) 
                    && offset < cur_offset()) {
                        drop(cycle_buffers_->in_avail() 
                            - (cur_offset() - offset));
                        offset = cur_offset();
                } else {
                    drop_all();
                    for (boost::uint32_t i = 0; i < strategys_.size(); ++i) {
                        if (offset < strategys_[i]->byte_size()) {
                            if (i != cur_pos_) {
                                cur_pos_ = i;
                                change_source(strategys_[cur_pos_]);
                            }
                            segment_source_->seek(offset, ec);
                            break;
                        } else {
                            offset -= strategys_[i]->byte_size();
                        }
                    }
                }
            } else {
                ec = framework::system::logic_error::out_of_range;
            }

            if (!ec || ec == boost::asio::error::would_block) {
                cur_offset(offset);
            }
            return ec;
        }
*/
        /*
        // buffer time计算的更准确需要优化忽略MP4头部位置
        boost::uint32_t MergerBase::get_buffer_time(void) const
        {
            MediaInfo info;
            error_code ec;
            media_->get_info(info, ec);
            if (!ec) {
                 return cycle_buffers_->in_avail() * info.duration / info.file_size;
            } else {
                return 0;
            }
        }
        std::size_t MergerBase::valid() const
        {
            return cur_offset();
        }

        void MergerBase::segment_infos(
            std::vector<ppbox::avformat::SegmentInfo> & segment_infos)
        {
            SegmentInfo info;
            for (boost::uint32_t i = 0; i < media_->segment_count(); ++i) {
                media_->segment_info(i, info);
                ppbox::avformat::SegmentInfo av_info;
                av_info.duration = info.duration;
                av_info.file_length = info.size;
                av_info.head_length = info.head_size;
                segment_infos.push_back(av_info);
            }
        }

        void MergerBase::cancel(error_code & ec)
        {
            segment_source_->cancel(ec);
        }

        void MergerBase::close(error_code & ec)
        {
            segment_source_->close(ec);
        }

        boost::uint64_t const & MergerBase::cur_offset(void) const
        {
            return cur_offset_;
        }

        void MergerBase::cur_offset(boost::uint64_t const & offset)
        {
            cur_offset_ = offset;
        }

        SegmentSource & MergerBase::source(void)
        {
            return *segment_source_;
        }

        MediaBase & MergerBase::media(void)
        {
            return *media_;
        }

        void MergerBase::response(error_code const & ec)
        {
            resp_(ec);
        }

        void MergerBase::add_strategy(ppbox::data::SegmentStrategy * strategy)
        {
            strategys_.push_back(strategy);
        }

        void MergerBase::add_media_merge(
            Mp4MergerImpl * merge_impl, 
            std::iostream * ios)
        {
            media_merge_impl_.merge_impl = merge_impl;
            media_merge_impl_.ios = ios;
        }
*/
    }
}
