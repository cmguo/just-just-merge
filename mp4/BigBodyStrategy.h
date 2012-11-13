// BigBodyStrategy.h

#ifndef _PPBOX_MERGE_MP4_BIG_BODY_STRATEGY_H_
#define _PPBOX_MERGE_MP4_BIG_BODY_STRATEGY_H_

#include <ppbox/data/SegmentStrategy.h>

namespace ppbox
{
    namespace merge
    {

        class BigBodyStrategy
            : public ppbox::data::SegmentStrategy
        {
        public:
            BigBodyStrategy(
                ppbox::data::SegmentMedia & media);

            virtual ~BigBodyStrategy();

        private:
            virtual void byte_range(
                ppbox::data::SegmentPosition const & pos, 
                ppbox::data::SegmentRange & range);

            virtual void time_range(
                ppbox::data::SegmentPosition const & pos, 
                ppbox::data::SegmentRange & range);
        };

    } // namespace data
} // namespace ppbox

#endif // _PPBOX_MERGE_MP4_BIG_BODY_STRATEGY_H_
