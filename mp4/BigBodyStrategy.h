// BigBodyStrategy.h

#ifndef _JUST_MERGE_MP4_BIG_BODY_STRATEGY_H_
#define _JUST_MERGE_MP4_BIG_BODY_STRATEGY_H_

#include <just/data/segment/SegmentStrategy.h>

namespace just
{
    namespace merge
    {

        class BigBodyStrategy
            : public just::data::SegmentStrategy
        {
        public:
            BigBodyStrategy(
                just::data::SegmentMedia & media);

            virtual ~BigBodyStrategy();

        private:
            virtual void byte_range(
                just::data::SegmentPosition const & pos, 
                just::data::SegmentRange & range);

            virtual void time_range(
                just::data::SegmentPosition const & pos, 
                just::data::SegmentRange & range);
        };

    } // namespace data
} // namespace just

#endif // _JUST_MERGE_MP4_BIG_BODY_STRATEGY_H_
