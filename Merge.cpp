// Join.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/Merge.h"
#include "ppbox/data/SourceError.h"
#include "ppbox/data/SegmentSource.h"

using namespace ppbox::data;
using namespace boost::asio;
using namespace boost::system;

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/logger/StringRecord.h>
#include <boost/bind.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Join", 0);

namespace ppbox
{
    namespace merge
    {
        Merge::Merge(
            boost::asio::io_service & io_srv, 
            boost::uint32_t buffer_size)
            : io_srv_(io_srv)
            , buffer_size_(framework::memory::PrivateMemory::align_page(buffer_size))
            , prepare_size_(10240)
            , read_size_(0)
            , head_size_(0)
            , cur_offset_(0)
            , segment_source_(new SegmentSource(io_srv_))
            , cur_pos_(boost::uint32_t(-1))
        {
            buffer_ = (char*)private_memory_.alloc_block(buffer_size_);
            mutable_buffers_1 buffers(buffer_, buffer_size_);
            cycle_buffers_ = new util::buffers::CycleBuffers< 
                boost::asio::mutable_buffers_1 >(buffers);
            tmp_buffer_.resize(prepare_size_);
        }

        Merge::~Merge()
        {
            if (cycle_buffers_) {
                delete cycle_buffers_;
                cycle_buffers_ = NULL;
            }
            if (segment_source_) {
                delete segment_source_;
                segment_source_ = NULL;
            }
            for (boost::uint32_t i = 0; i < strategys_.size(); i++) {
                Strategy::destory(strategys_[i]);
            }
        }

        void Merge::async_open(
            framework::string::Url const & playlink, 
            std::iostream * ios, 
            response_type const & resp)
        {
            response(resp);
            segment_source_->async_open(
                playlink, 
                0, 
                boost::uint64_t(-1), 
                boost::bind(&Merge::open_callback, this, _1));
        }

        void Merge::open_callback(boost::system::error_code const & ec)
        {
            if (ec) {
                LOG_WARN("[open_callback] ec: " << ec.message());
            } else {
                Strategy * strategy = Strategy::create(
                    "full", 
                    *segment_source_->media());
                add_strategy(strategy);
                segment_source_->set_strategy(strategys()[cur_pos_]);
                segment_source_->reset();
            }
            response(ec);
        }

        std::size_t Merge::read(
            std::vector<boost::asio::const_buffer> & buffers,
            boost::system::error_code & ec)
        {
            prepare(ec);
            boost::uint32_t receives = 0;
            if (!ec) {
                if (media_merge_impl_.ios && !media_merge_impl_.finished) {
                    receives = media_merge_impl_.ios->readsome(
                        (char*)&tmp_buffer_.at(0), 
                        prepare_size_);
                    buffers.push_back(boost::asio::buffer(tmp_buffer_, receives));
                    if (media_merge_impl_.ios->eof()) {
                        media_merge_impl_.finished = true;
                    }
                } else {
                    drop(read_size_);
                    std::size_t size = cycle_buffers_->in_avail();
                    if (size == 0) {
                        read_size_ = 0;
                    } else {
                        if (prepare_size_ < size) {
                            read_size_ = prepare_size_;
                        } else {
                            read_size_ = size;
                        }
                        buffer_copy(cycle_buffers_->data(read_size_), buffers);
                    }
                    receives = read_size_;
                }
            } else {
                read_size_ = 0;
                receives = read_size_;
            }
            return receives;
        }

        error_code Merge::prepare(
            error_code & ec)
        {
            assert(segment_source_->is_open(ec));
            std::size_t bytes_received = 0;
            if (media_merge_impl_.merge_impl) {
                assert(media_merge_impl_.ios);
                bytes_received = segment_source_->read_some(
                    boost::asio::buffer(tmp_buffer_), 
                    ec);
                if (!ec) {
                    media_merge_impl_.ios->write((char*)&tmp_buffer_, bytes_received);
                    ec = boost::asio::error::would_block;
                } else if (ec == ppbox::data::source_error::no_more_segment) {
                    std::vector<ppbox::avformat::SegmentInfo> segments;
                    segment_infos(segments);
                    media_merge_impl_.merge_impl->meger_small_head(
                        *media_merge_impl_.ios, 
                        *media_merge_impl_.ios, 
                        segments, 
                        head_size_, 
                        ec);
                    delete media_merge_impl_.merge_impl;
                    media_merge_impl_.merge_impl = NULL;
                    segment_source_->set_strategy(strategys()[++cur_pos_]);
                    segment_source_->reset();
                    ec.clear();
                }
            } else {
                bytes_received = segment_source_->read_some(cycle_buffers_->prepare(), ec);
                if (!ec) {
                    cur_offset(cur_offset() + bytes_received);
                    cycle_buffers_->commit(bytes_received);
                } else {
                    if (ec == ppbox::data::source_error::no_more_segment) {
                        if (cur_pos_ < strategys().size()) {
                            segment_source_->set_strategy(strategys()[++cur_pos_]);
                            segment_source_->reset();
                            ec = boost::asio::error::would_block;
                        } else {
                            ec = boost::asio::error::eof;
                        }
                    }
                }
            }
            return ec;
        }

        void Merge::drop(std::size_t size)
        {
            cycle_buffers_->consume(size);
        }

        void Merge::drop_all(void)
        {
            // cycle_buffers_->reset();
        }

        error_code Merge::seek(
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
                    for (boost::uint32_t i = 0; i < strategys().size(); ++i) {
                        if (offset < strategys()[i]->size()) {
                            if (i != cur_pos_) {
                                cur_pos_ = i;
                                segment_source_->set_strategy(strategys()[cur_pos_]);
                            }
                            segment_source_->seek(offset, ec);
                            break;
                        } else {
                            offset -= strategys()[i]->size();
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

        // buffer time计算的更准确需要优化忽略MP4头部位置
        boost::uint32_t Merge::get_buffer_time(void) const
        {
            MediaInfo info;
            error_code ec;
            segment_source_->media()->get_info(info, ec);
            if (!ec) {
                 return cycle_buffers_->in_avail() * info.duration / info.file_size;
            } else {
                return 0;
            }
        }

        void Merge::buffer_copy(
            util::buffers::CycleBuffers<boost::asio::mutable_buffers_1>::ConstBuffers const & from,
            std::vector<boost::asio::const_buffer> & to)
        {
            to.clear();
            for (util::buffers::CycleBuffers<
                boost::asio::mutable_buffers_1>::const_buffer_iterator const_iter = from.begin();
            const_iter != from.end();
            const_iter++) {
                to.push_back(*const_iter);
            }
        }

        std::size_t Merge::valid() const
        {
            return cur_offset();
        }

        std::size_t Merge::size() const
        {
            std::size_t length = 0;
            for (boost::uint32_t i = 0; i < strategys().size(); ++i) {
                length += strategys()[i]->size();
            }
            return length;
        }

        void Merge::segment_infos(
            std::vector<ppbox::avformat::SegmentInfo> & segment_infos)
        {
            SegmentInfo info;
            for (boost::uint32_t i = 0; i < source()->media()->segment_count(); ++i) {
                source()->media()->segment_info(i, info);
                ppbox::avformat::SegmentInfo av_info;
                av_info.duration = info.duration;
                av_info.file_length = info.size;
                av_info.head_length = info.head_size;
                segment_infos.push_back(av_info);
            }
        }

        void Merge::cancel(error_code & ec)
        {
            source()->cancel(ec);
        }

        void Merge::close(error_code & ec)
        {
            source()->close(ec);
        }

        boost::uint64_t const & Merge::cur_offset(void) const
        {
            return cur_offset_;
        }

        void Merge::cur_offset(boost::uint64_t const & offset)
        {
            cur_offset_ = offset;
        }

        SegmentSource * Merge::source(void)
        {
            return segment_source_;
        }

        void Merge::response(response_type const & resp)
        {
            resp_ = resp;
        }

        void Merge::response(error_code const & ec)
        {
            resp_(ec);
        }

        std::vector<ppbox::data::Strategy *> const & 
            Merge::strategys(void) const
        {
            return strategys_;
        }

        void Merge::add_strategy(ppbox::data::Strategy * strategy)
        {
            strategys_.push_back(strategy);
        }

        void Merge::add_media_merge(
            Mp4MergeImpl * merge_impl, 
            std::iostream * ios)
        {
            media_merge_impl_.merge_impl = merge_impl;
            media_merge_impl_.ios = ios;
        }

    }
}
