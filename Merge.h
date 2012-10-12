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
        class MediaBase;
        class SourceBase;
        class SegmentStrategy;
        class SegmentSource;
    }

    namespace merge
    {
        // �������ý��Э��ʵ�ֵĺϲ���
        // Ŀǰֻ֧��mp4
        // ����һ������ĺϲ��࣬���û���漰��ý��ϲ��߼����Բ��ó�ʼ��merge_impl
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

            void async_open(
                framework::string::Url const & playlink, 
                std::iostream * ios, 
                response_type const & resp);

            std::size_t read(
                std::vector<boost::asio::const_buffer> & buffers,
                boost::system::error_code & ec);

            boost::system::error_code seek(
                std::size_t offset, 
                boost::system::error_code & ec);

            boost::uint32_t get_buffer_time(void) const;

            std::size_t valid() const;

            std::size_t size();

            void cancel(boost::system::error_code & ec);

            void close(boost::system::error_code & ec);

            ppbox::data::SegmentSource & source(void);

            ppbox::data::MediaBase & media(void);

        protected:
            void add_strategy(ppbox::data::SegmentStrategy * strategy);

        private:
            virtual void set_strategys(void);

            boost::uint64_t const & cur_offset(void) const;

            void cur_offset(boost::uint64_t const & offset);

            void response(boost::system::error_code const & ec);

            void add_media_merge(
                Mp4MergeImpl * merge_impl, 
                std::iostream * ios);

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

            void change_source(ppbox::data::SegmentStrategy * strategy);

        protected:
            // opened by merge
            ppbox::data::MediaBase * media_;

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
            // �Ѿ��������ݵ�ƫ����
            boost::uint64_t cur_offset_;

            // Э�鼶�ϲ���
            MediaMergeImpl media_merge_impl_;
            framework::string::Url playlink_;
            // opened by SegmentSource
            ppbox::data::SourceBase * source_;
            ppbox::data::SegmentSource * segment_source_;
            std::vector<ppbox::data::SegmentStrategy *> strategys_;
            boost::uint32_t cur_pos_;

            response_type resp_;
            framework::memory::PrivateMemory private_memory_;
        };
    }
}

#endif // End _PPBOX_MERGE_MERGE_H_ 
