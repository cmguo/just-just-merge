// Mp4HeadStrategy.cpp

#include "just/merge/Common.h"
#include "just/merge/mp4/Mp4HeadStrategy.h"

#include <just/avformat/mp4/Mp4Merge.h>

#include <just/data/segment/SegmentBuffer.h>

#include <util/buffers/CycleBuffers.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

namespace just
{
    namespace merge
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.merge.Mp4HeadStrategy", framework::logger::Debug);

        using just::data::invalid_size;

        Mp4HeadStrategy::Mp4HeadStrategy(
            just::data::SegmentMedia & media, 
            just::data::SegmentBuffer & buffer)
            : HeadStrategy(media)
            , buffer_(buffer)
            , ready_(false)
            , head_size_(0)
            , merge_size_(0)
        {
        }

        Mp4HeadStrategy::~Mp4HeadStrategy()
        {
        }

        bool Mp4HeadStrategy::check(
            boost::uint64_t offset, 
            just::data::SegmentBuffer & buffer, 
            boost::system::error_code & ec)
        {
            ec.clear();
            if (ready_) {
                if (offset < head_size_ && offset < buffer_.in_limit()) {
                    LOG_INFO("check offset " << offset << ", clear all data!!");
                    buffer.clear();
                    ready_ = false;
                    merge_size_ = 0;
                }
            }
            if (!ready_) {
                if (head_size_ == 0)
                    head_size_ = byte_size();
                if (media_.segment_count() == 1) {
                    merge_size_ = head_size_;
                    return true;
                }
                if (!buffer_.write_segment().valid()) {
                    ec = boost::asio::error::would_block;
                } else if (buffer_.in_position() != 0) {
                    ec = boost::asio::error::would_block;
                } else if (buffer_.out_position() < head_size_) {
                    if (!(ec = buffer_.last_error()))
                        buffer_.prepare_at_least((size_t)(head_size_ - buffer_.out_position()), ec);
                }
                if (!ec) {
                    if (merge_head(ec)) {
                        ready_ = true;
                    }
                }
            }
            return !ec;
        }

        void Mp4HeadStrategy::time_range(
            just::data::SegmentPosition const & pos, 
            just::data::SegmentRange & range)
        {
            if (merge_size_) {
                range.beg = 0;
                range.end = merge_size_ - range.big_offset;
                if (range.end > pos.head_size)
                    range.end = pos.head_size;
            } else {
                range.beg = invalid_size;
                range.end = invalid_size;
            }
        }

        bool Mp4HeadStrategy::merge_head(
            boost::system::error_code & ec)
        {
            std::vector<boost::asio::const_buffer> buffers;
            buffer_.data((size_t)head_size_, buffers);
            std::vector<boost::asio::mutable_buffer> mutable_buffers;
            for (size_t i = 0; i < buffers.size(); ++i) {
                boost::asio::mutable_buffer buffer(
                    const_cast<void *>(boost::asio::buffer_cast<void const *>(buffers[i])), 
                    boost::asio::buffer_size(buffers[i]));
                mutable_buffers.push_back(buffer);
            }
            util::buffers::CycleBuffers<std::vector<boost::asio::mutable_buffer>, boost::uint8_t> cycle_buffers(mutable_buffers);
            cycle_buffers.commit((size_t)head_size_);
            std::basic_iostream<boost::uint8_t> ios(&cycle_buffers);
            std::vector<just::data::SegmentInfo> segments;
            for (size_t i = 0; i < media_.segment_count(); ++i) {
                just::data::SegmentInfo segment;
                media_.segment_info(i, segment, ec);
                segments.push_back(segment);
            }
            if (just::avformat::mp4_merge_head(ios, ios, segments, ec)) {
                merge_size_ = cycle_buffers.out_position() - head_size_;
            }
            return !ec;
        }

    } // namespace merge
} // namespace just
