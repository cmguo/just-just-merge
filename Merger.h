// Merger.h

#ifndef _JUST_MERGE_MERGER_H_
#define _JUST_MERGE_MERGER_H_

#include "just/merge/MergerBase.h"
#include "just/merge/MergeStatistic.h"

#include <just/data/segment/SegmentPosition.h>

namespace just
{
    namespace data
    {
        class SegmentStrategy;
        class ListStrategy;
        class SegmentSource;
        class SegmentBuffer;
    }

    namespace merge
    {

        class Merger
            : public MergerBase
            , public MergeStatistic
        {
        public:
            Merger(
                boost::asio::io_service & io_svc, 
                just::data::SegmentMedia & media);

            virtual ~Merger();

        public:
            virtual void async_open(
                response_type const & resp);

            virtual void cancel(
                boost::system::error_code & ec);

            virtual void close(
                boost::system::error_code & ec);

        public:
            virtual bool byte_seek(
                boost::uint64_t offset, 
                boost::system::error_code & ec);

            virtual bool reset(
                boost::system::error_code & ec);

            virtual bool read(
                Sample & sample,
                boost::system::error_code & ec);

            virtual bool free(
                Sample & sample,
                boost::system::error_code & ec);

            virtual bool fill_data(
                boost::system::error_code & ec);

        public:
            virtual void media_info(
                MediaInfo & info) const;

            virtual void stream_info(
                std::vector<StreamInfo> & streams) const;

            virtual void stream_status(
                StreamStatus & info) const;

            virtual void data_stat(
                DataStat & stat) const;

        public:
            just::data::SegmentMedia const & media()
            {
                return media_;
            }

            just::data::SegmentSource const & source()
            {
                return *source_;
            }

        protected:
            virtual void set_strategys() = 0;

            void add_strategy(
                just::data::SegmentStrategy & strategy);

        private:
            enum StateEnum            {                closed,                media_open,                merger_open,                opened,            };
        private:
            void response(
                boost::system::error_code const & ec);

            void handle_async(
                boost::system::error_code const & ec);

        protected:
            just::data::SegmentMedia & media_;
            just::data::MediaInfo media_info_;
            just::data::SegmentBuffer * buffer_;

        private:
            just::data::SegmentSource * source_;
            just::data::ListStrategy * strategy_;

            StateEnum open_state_;
            bool seek_pending_;

            just::data::SegmentPosition read_;
            boost::uint32_t read_size_; // 读取一次，最大的读取数据大小

            response_type resp_;
        };

    } // namespace merge
} // namespace just

#endif // _JUST_MERGE_MERGER_H_
