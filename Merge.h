// Merge.h

#ifndef   _PPBOX_MERGE_MERGE_H_
#define   _PPBOX_MERGE_MERGE_H_

#include "ppbox/merge/MergeStatistic.h"
#include "ppbox/merge/impl/Mp4MergeImpl.h"

#include <framework/string/Url.h>
#include <boost/asio/buffer.hpp>

#include <util/stream/Sink.h>
#include <framework/memory/PrivateMemory.h>
#include <util/buffers/CycleBuffers.h>

namespace ppbox
{
    namespace data
    {
        class Strategy;
        class SegmentSource;
    }

    namespace merge
    {
        // 根据相关媒体协议实现的合并器
        // 目前只支持mp4
        // 对于一个具体的合并类，如果没有涉及到媒体合并逻辑可以不用初始化merge_impl
        struct MediaMergeImpl
        {
            MediaMergeImpl()
                : merge_impl(NULL)
                , ios(NULL)
                , finished(false)
            {
            }

            Mp4MergeImpl * merge_impl;
            std::iostream * ios;
            bool finished;
        };

        typedef boost::function<void (
            boost::system::error_code const &)
        >  response_type;

        class Merge
            : public MergeStatistic
        {
        public:
            Merge(
                boost::asio::io_service & io_srv, 
                boost::uint32_t buffer_size);

            ~Merge();

            virtual void async_open(
                framework::string::Url const & playlink, 
                std::iostream * ios, 
                response_type const & resp);

            std::size_t read(
                std::vector<boost::asio::const_buffer> & buffers,
                boost::system::error_code & ec);

            virtual boost::system::error_code seek(
                std::size_t offset, 
                boost::system::error_code & ec);

            virtual boost::uint32_t get_buffer_time(void) const;

            virtual std::size_t valid() const;

            virtual std::size_t size() const;

            virtual void cancel(boost::system::error_code & ec);

            virtual void close(boost::system::error_code & ec);

            ppbox::data::SegmentSource * source(void);

        protected:
            boost::uint64_t const & cur_offset(void) const;

            void cur_offset(boost::uint64_t const & offset);

            void response(response_type const & resp);

            void response(boost::system::error_code const & ec);

            void add_strategy(ppbox::data::Strategy * strategy);

            void add_media_merge(
                Mp4MergeImpl * merge_impl, 
                std::iostream * ios);

            std::vector<ppbox::data::Strategy *> const & 
                strategys(void) const;

        private:
            void open_callback(boost::system::error_code const & ec);

            boost::system::error_code prepare(
                boost::system::error_code & ec);

            void drop(std::size_t size);

            void drop_all(void);

            void buffer_copy(
                util::buffers::CycleBuffers<boost::asio::mutable_buffers_1>::ConstBuffers const & from,
                std::vector<boost::asio::const_buffer> & to);

            void segment_infos(
                std::vector<ppbox::avformat::SegmentInfo> & segment_infos);

        protected:
            // 协议级合并器
            MediaMergeImpl media_merge_impl_;

        private:
            boost::asio::io_service & io_srv_;
            char * buffer_;
            boost::uint32_t buffer_size_;
            boost::uint32_t prepare_size_;

            // buffer
            util::buffers::CycleBuffers<
                boost::asio::mutable_buffers_1 > * cycle_buffers_;
            std::vector<boost::uint8_t> tmp_buffer_;
            boost::uint32_t read_size_;
            boost::uint32_t head_size_;
            // 已经下载数据的偏移量
            boost::uint64_t cur_offset_;

            ppbox::data::SegmentSource * segment_source_;
            std::vector<ppbox::data::Strategy *> strategys_;
            boost::uint32_t cur_pos_;

            response_type resp_;
            framework::memory::PrivateMemory private_memory_;
        };
    }
}

#endif // End _PPBOX_MERGE_MERGE_H_ 
