// BigHeadStrategy.h

#ifndef _JUST_MERGE_MP4_BIG_HAED_STRATEGY_H_
#define _JUST_MERGE_MP4_BIG_HAED_STRATEGY_H_

#include <just/data/segment/SegmentStrategy.h>

namespace just
{
    namespace merge
    {

        class BigHeadStrategy
            : public just::data::SegmentStrategy
        {
        public:
            BigHeadStrategy(
                just::data::SegmentMedia & media);

            virtual ~BigHeadStrategy();

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

#endif // _JUST_MERGE_MP4_BIG_HAED_STRATEGY_H_
