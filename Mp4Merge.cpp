// Mp4Merge.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/Mp4Merge.h"
#include "ppbox/merge/impl/Mp4MergeImpl.h"
#include "ppbox/data/SourceError.h"
#include "ppbox/data/SegmentSource.h"
#include "ppbox/data/strategy/HeadSourceStrategy.h"
#include "ppbox/data/strategy/BodySourceStrategy.h"
using namespace ppbox::data;

#include <boost/bind.hpp>
using namespace boost::system;

namespace ppbox
{
    namespace merge
    {
        // TODO: 要考虑播放和下载两种情况
        Mp4Merge::Mp4Merge(
            boost::asio::io_service & io_srv, 
            boost::uint32_t buffer_size)
            : Merge(io_srv, buffer_size)
            , smallhead_strategy_(NULL)
            , head_size_(0)
            , body_size_(0)
            , remain_head_size_(0)
            , small_head_total_size_(0)
            , using_body_strategy_(false)
            , download_stream_(NULL)
        {
            read_head_buffer_.resize(10240);
        }

        Mp4Merge::~Mp4Merge()
        {
            if (smallhead_strategy_) {
                SourceStrategy::destory(smallhead_strategy_);
            }
        }

        void Mp4Merge::async_open(
            framework::string::Url const & playlink, 
            response_type const & resp)
        {
            resp_ = resp;
            source()->async_open(
                playlink, 
                0, 
                boost::uint64_t(-1), 
                boost::bind(&Mp4Merge::open_callback, this, _1));
        }

        void Mp4Merge::open_callback(error_code const & ec)
        {
            if (!ec) {
                smallhead_strategy_ = SourceStrategy::create(
                    "head", 
                    source()->segments(), 
                    source()->video_info());
                assert(smallhead_strategy_);
                boost::system::error_code lec;
                source()->set_non_block(true, lec);
                source()->set_strategy(smallhead_strategy_);
                source()->reset();
                small_head_total_size_ = smallhead_strategy_->size();
            }
            resp_(ec);
        }

        std::size_t Mp4Merge::read(
            std::vector<boost::asio::const_buffer> & buffers,
            error_code & ec)
        {
            std::size_t size =  0;
            if (head_size_ > 0) {
                if (remain_head_size_ > 0) {
                        remain_head_size_ > 10240 ? 10240 : remain_head_size_;
                    ios_.read((char*)&read_head_buffer_.at(0), size);
                    buffers.clear();
                    buffers.push_back(boost::asio::buffer(&read_head_buffer_.at(0), size));
                } else {
                    if (using_body_strategy_) {
                        size = Merge::read(buffers, ec);
                    } else {
                        SourceStrategy * strategy = SourceStrategy::create(
                            "body", 
                            source()->segments(), 
                            source()->video_info());
                        assert(strategy);
                        body_size_ = strategy->size();
                        source()->set_non_block(true, ec);
                        source()->set_strategy(strategy);
                        source()->reset();
                        add_strategy(strategy);
                        size = Merge::read(buffers, ec);
                    }
                }
            } else {
                std::size_t receives = source()->read_some(
                    boost::asio::buffer(read_head_buffer_), 
                    ec);
                if (ec) {
                    if (ec == ppbox::data::source_error::no_more_segment) {
                        // merge small head
                        std::vector<ppbox::avformat::SegmentInfo> segment_infos;
                        transfer_infos(segment_infos);
                        mp4_merge_impl_.meger_small_head(
                            ios_, 
                            ios_, 
                            segment_infos, 
                            head_size_, 
                            ec);
                        if (!ec) {
                            remain_head_size_ = head_size_;
                        }
                    }
                } else {
                    ios_.write((char*)&read_head_buffer_.at(0), receives);
                }
                size = 0;
            }
            return size;
        }

        error_code Mp4Merge::seek(
            std::size_t offset, 
            error_code & ec)
        {
            if (head_size_) {
                if (offset < head_size_) {
                    using_body_strategy_ = false;
                    remain_head_size_ = head_size_ - offset;
                    ios_.seekg(offset);
                } else if (offset < (head_size_ + body_size_)) {
                    source()->seek(offset - head_size_, ec);
                    remain_head_size_ = 0;
                }
            } else {
                ec = framework::system::logic_error::processing;
            }
            return ec;
        }

        std::size_t Mp4Merge::valid() const
        {
            return cur_offset();
        }

        std::size_t Mp4Merge::size() const
        {
            if (head_size_) {
                MergeInfo info;
                get_info_statictis(info);
                return info.head_size + Merge::size();
            } else {
                return 0;
            }
        }

        void Mp4Merge::set_out_stream(std::iostream & ios)
        {
            download_stream_ = &ios;
        }

        void Mp4Merge::transfer_infos(
            std::vector<ppbox::avformat::SegmentInfo> & segment_infos)
        {
            for (boost::uint32_t i = 0; 
                i < source()->segments().size(); 
                i++ ) {
                    ppbox::avformat::SegmentInfo info;
                    info.duration = source()->segments()[i].duration;
                    info.file_length = source()->segments()[i].size;
                    info.head_length = source()->segments()[i].head_size;
                    segment_infos.push_back(info);
            }
        }

    } // namespace merge
} // namespace ppbox
