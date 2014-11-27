// Mp4Merger.h

#ifndef _JUST_MERGE_MP4_MERGER_H_
#define _JUST_MERGE_MP4_MERGER_H_

#include "just/merge/Merger.h"

namespace just
{
    namespace merge
    {

        class Mp4HeadStrategy;

        class Mp4Merger
            : public Merger
        {
        public:
            Mp4Merger(
                boost::asio::io_service & io_svc, 
                just::data::SegmentMedia & media);

            virtual ~Mp4Merger();

        public:
            virtual bool byte_seek(
                boost::uint64_t offset, 
                boost::system::error_code & ec);

        private:
            virtual void set_strategys(void);

        private:
            bool big_head_;
            Mp4HeadStrategy * head_strategy_;
        };

    } // namespace merge
} // namespace just

#endif // _JUST_MERGE_MP4_MERGER_H_
