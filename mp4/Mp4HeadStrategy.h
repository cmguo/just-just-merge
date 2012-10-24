// Mp4HeadStrategy.h

#ifndef _PPBOX_MERGE_P4_HAED_STRATEGY_H_
#define _PPBOX_MERGE_P4_HAED_STRATEGY_H_

#include <ppbox/data/strategy/HeadStrategy.h>

namespace ppbox
{
    namespace data
    {
        class SegmentBuffer;
    }

    namespace merge
    {

        class Mp4HeadStrategy
            : public ppbox::data::HeadStrategy
        {
        public:
            Mp4HeadStrategy(
                ppbox::data::MediaBase & media, 
                ppbox::data::SegmentBuffer & buffer);

            virtual ~Mp4HeadStrategy();

        public:
            virtual void time_range(
                ppbox::data::SegmentPosition const & pos, 
                ppbox::data::SegmentRange & range);

        public:
            bool check(
                boost::uint64_t offset, 
                ppbox::data::SegmentBuffer & buffer, 
                boost::system::error_code & ec);

        private:
            bool merge_head(
                boost::system::error_code & ec);

        private:
            ppbox::data::SegmentBuffer & buffer_;
            bool ready_;
            boost::uint64_t head_size_;
        };

    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_P4_HAED_STRATEGY_H_
