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
            , cur_offset_(0)
            , segment_source_(new SegmentSource(io_srv_))
            , cur_pos_(0)
        {
            buffer_ = (char*)private_memory_.alloc_block(buffer_size_);
            mutable_buffers_1 buffers(buffer_, buffer_size_);
            cycle_buffers_ = new util::buffers::CycleBuffers< 
                boost::asio::mutable_buffers_1 >(buffers);
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
                SourceStrategy::destory(strategys_[i]);
            }
        }

        void Merge::async_open(
            framework::string::Url const & playlink, 
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
                SourceStrategy * strategy = SourceStrategy::create(
                    "full", 
                    segment_source_->segments(),
                    segment_source_->video_info());
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
            if (!ec || ec == boost::asio::error::would_block) {
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
            } else {
                read_size_ = 0;
            }
            return read_size_;
        }

        error_code Merge::prepare(
            error_code & ec)
        {
            assert(segment_source_->is_open(ec));
            std::size_t bytes_received = 
            segment_source_->read_some(cycle_buffers_->prepare(), ec);
            cur_offset() += bytes_received;
            if (!ec) {
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
                cur_offset() = offset;
            }
            return ec;
        }

        boost::uint32_t Merge::get_buffer_time(void) const
        {
            return  cycle_buffers_->in_avail() * 
                segment_source_->video_info().duration / 
                segment_source_->video_info().file_size;
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
            return 0;
        }

        std::size_t Merge::size() const
        {
            std::size_t length = 0;
            for (boost::uint32_t i = 0; i < strategys().size(); ++i) {
                length += strategys()[i]->size();
            }
            return length;
        }

        void Merge::cancel(error_code & ec)
        {
            source()->cancel(0, ec);
        }

        void Merge::close(error_code & ec)
        {
            source()->close(0, ec);
        }

        boost::uint64_t & Merge::cur_offset(void)
        {
            return cur_offset_;
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

        void Merge::add_strategy(ppbox::data::SourceStrategy * strategy)
        {
            strategys_.push_back(strategy);
        }

    }
}
