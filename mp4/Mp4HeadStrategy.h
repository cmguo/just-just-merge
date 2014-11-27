// Mp4HeadStrategy.h

#ifndef _JUST_MERGE_MP4_HAED_STRATEGY_H_
#define _JUST_MERGE_MP4_HAED_STRATEGY_H_

#include <just/data/strategy/HeadStrategy.h>

namespace just
{
    namespace data
    {
        class SegmentBuffer;
    }

    namespace merge
    {

        class Mp4HeadStrategy
            : public just::data::HeadStrategy
        {
        public:
            Mp4HeadStrategy(
                just::data::SegmentMedia & media, 
                just::data::SegmentBuffer & buffer);

            virtual ~Mp4HeadStrategy();

        public:
            virtual void time_range(
                just::data::SegmentPosition const & pos, 
                just::data::SegmentRange & range);

        public:
            bool check(
                boost::uint64_t offset, 
                just::data::SegmentBuffer & buffer, 
                boost::system::error_code & ec);

        private:
            bool merge_head(
                boost::system::error_code & ec);

        private:
            just::data::SegmentBuffer & buffer_;
            bool ready_;
            boost::uint64_t head_size_;
            boost::uint64_t merge_size_;
        };

    } // namespace merge
} // namespace just

#endif // _JUST_MERGE_MP4_HAED_STRATEGY_H_
