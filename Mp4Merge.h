// Mp4Merge.h

#ifndef   _PPBOX_MERGE_MP4_MERGE_H_
#define   _PPBOX_MERGE_MP4_MERGE_H_

#include "ppbox/merge/Merge.h"
#include "ppbox/merge/impl/Mp4MergeImpl.h"

#include <ppbox/avformat/mp4/Ap4HeadMerge.h>

namespace ppbox
{
    namespace merge
    {

        class Mp4Merge
            : public Merge
        {
        public:
            Mp4Merge(
                boost::asio::io_service & io_srv, 
                boost::uint32_t buffer_size);

            ~Mp4Merge();

             void async_open(
                framework::string::Url const & playlink, 
                response_type const & resp);

             std::size_t read(
                 std::vector<boost::asio::const_buffer> & buffers,
                 boost::system::error_code & ec);

             boost::system::error_code seek(
                 std::size_t offset, 
                 boost::system::error_code & ec);

             std::size_t valid() const;

             std::size_t size() const;

             void set_out_stream(std::iostream & ios);

        private:
            void open_callback(boost::system::error_code const & ec);

            void transfer_infos(
                std::vector<ppbox::avformat::SegmentInfo> & segment_infos);

        private:
            std::stringstream ios_;
            ppbox::data::SourceStrategy * smallhead_strategy_;
            boost::uint32_t head_size_;
            boost::uint32_t body_size_;
            boost::uint32_t remain_head_size_;
            boost::uint32_t small_head_total_size_;
            bool using_body_strategy_;

            // buffer
            std::vector<boost::uint8_t> read_head_buffer_;
            std::iostream * download_stream_;
            // tool
            std::vector<ppbox::avformat::SegmentInfo> segment_infos_;
            Mp4MergeImpl mp4_merge_impl_;

            response_type resp_;

        };
    }
}

#endif // END _PPBOX_MERGE_MP4_MERGE_H_
