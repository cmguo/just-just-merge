// MergerBase.h

#ifndef _PPBOX_MERGE_MERGER_BASE_H_
#define _PPBOX_MERGE_MERGER_BASE_H_

#include "ppbox/merge/MergeStatistic.h"

#include <ppbox/data/SegmentPosition.h>
#include <ppbox/avformat/Format.h>

#include <boost/asio/buffer.hpp>

namespace ppbox
{
    namespace data
    {
        class MediaBase;
        class SegmentStrategy;
        class ListStrategy;
        class SegmentSource;
        class SegmentBuffer;
    }

    namespace merge
    {

        class MergerBase
            : public MergeStatistic
        {
        public:
            typedef boost::function<void (
                boost::system::error_code const &)
            >  response_type;

        public:
            MergerBase(
                boost::asio::io_service & io_svc, 
                ppbox::data::MediaBase & media);

            virtual ~MergerBase();

        public:
            virtual void async_open(
                response_type const & resp);

            virtual void cancel(
                boost::system::error_code & ec);

            virtual void close(
                boost::system::error_code & ec);

            virtual bool seek(
                boost::uint64_t offset, 
                boost::system::error_code & ec);

            virtual boost::uint64_t total_size();

            bool read(
                ppbox::avformat::Sample & sample,
                boost::system::error_code & ec);

            boost::uint64_t get_current();

            boost::uint64_t get_buffer_size();

            boost::uint64_t get_buffer_time(
                boost::system::error_code & ec);

            boost::uint64_t valid() const;

        public:
            ppbox::data::MediaBase const & media(void)
            {
                return media_;
            }

            ppbox::data::SegmentSource const & source(void)
            {
                return *source_;
            }

        protected:
            virtual void set_strategys() = 0;

            void add_strategy(
                ppbox::data::SegmentStrategy & strategy);

        private:
            boost::uint64_t const & cur_offset(void) const;

            void cur_offset(boost::uint64_t const & offset);

            void response(
                boost::system::error_code const & ec);

            void open_callback(
                boost::system::error_code const & ec);

        protected:
            ppbox::data::MediaBase & media_;
            ppbox::data::SegmentBuffer * buffer_;

        private:
            ppbox::data::SegmentSource * source_;
            ppbox::data::ListStrategy * strategy_;

            bool seek_pending_;

            ppbox::data::SegmentPosition read_;
            boost::uint32_t read_size_; // 读取一次，最大的读取数据大小

            response_type resp_;
        };

    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_MERGER_BASE_H_
