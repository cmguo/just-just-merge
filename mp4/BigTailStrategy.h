// BigTailStrategy.h

#ifndef _JUST_MERGE_MP4_BIG_TAIL_STRATEGY_H_
#define _JUST_MERGE_MP4_BIG_TAIL_STRATEGY_H_

#include <just/data/segment/SegmentStrategy.h>

namespace just
{
    namespace merge
    {

        class BigTailStrategy
            : public just::data::SegmentStrategy
        {
        public:
            BigTailStrategy(
                just::data::SegmentMedia & media);

            virtual ~BigTailStrategy();

        public:
            virtual bool next_segment(
                just::data::SegmentPosition & info, 
                boost::system::error_code & ec);

            virtual bool get_url(
                just::data::SegmentPosition const & pos, 
                framework::string::Url & url, 
                boost::system::error_code & ec);
        };

    } // namespace merge
} // namespace just

#endif // End _JUST_DATA_BIG_TAIL_STRATEGY_
