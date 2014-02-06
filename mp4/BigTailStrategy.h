// BigTailStrategy.h

#ifndef _PPBOX_MERGE_MP4_BIG_TAIL_STRATEGY_H_
#define _PPBOX_MERGE_MP4_BIG_TAIL_STRATEGY_H_

#include <ppbox/data/segment/SegmentStrategy.h>

namespace ppbox
{
    namespace merge
    {

        class BigTailStrategy
            : public ppbox::data::SegmentStrategy
        {
        public:
            BigTailStrategy(
                ppbox::data::SegmentMedia & media);

            virtual ~BigTailStrategy();

        public:
            virtual bool next_segment(
                ppbox::data::SegmentPosition & info, 
                boost::system::error_code & ec);

            virtual bool get_url(
                ppbox::data::SegmentPosition const & pos, 
                framework::string::Url & url, 
                boost::system::error_code & ec);
        };

    } // namespace merge
} // namespace ppbox

#endif // End _PPBOX_DATA_BIG_TAIL_STRATEGY_
