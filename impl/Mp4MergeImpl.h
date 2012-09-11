// MP4MergeImpl.h

#ifndef _PPBOX_MERGE_MP4_MERGE_IMPL_H_
#define _PPBOX_MERGE_MP4_MERGE_IMPL_H_

#include <ppbox/avformat/mp4/Ap4HeadMerge.h>

namespace ppbox
{
    namespace merge
    {
        class Mp4MergeImpl
        {
        public:
            Mp4MergeImpl();

            virtual ~Mp4MergeImpl();

            boost::system::error_code meger_small_head(
                std::iostream & ios,
                std::ostream & os, 
                std::vector<ppbox::avformat::SegmentInfo> & segment_infos,
                boost::uint32_t& head_size,
                boost::system::error_code & ec);

            boost::system::error_code vod_valid_segment_info(
                std::istream & is,
                std::vector<ppbox::avformat::SegmentInfo> & segment_infos,
                boost::system::error_code & ec);

        };
    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_MP4_MERGE_IMPL_H_
