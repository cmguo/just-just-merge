// BigHeadStrategy.h

#ifndef _PPBOX_MERGE_MP4_BIG_HAED_STRATEGY_H_
#define _PPBOX_MERGE_MP4_BIG_HAED_STRATEGY_H_

#include <ppbox/data/segment/SegmentStrategy.h>

namespace ppbox
{
    namespace merge
    {

        class BigHeadStrategy
            : public ppbox::data::SegmentStrategy
        {
        public:
            BigHeadStrategy(
                ppbox::data::SegmentMedia & media);

            virtual ~BigHeadStrategy();

        public:
            virtual bool next_segment(
                ppbox::data::SegmentPosition & info, 
                boost::system::error_code & ec);

            virtual bool get_url(
                ppbox::data::SegmentPosition & pos, 
                boost::system::error_code & ec);
        };

    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_MP4_BIG_HAED_STRATEGY_H_
