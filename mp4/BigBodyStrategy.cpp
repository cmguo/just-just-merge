// BigBodyStrategy.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/mp4/BigBodyStrategy.h"

namespace ppbox
{
    namespace merge
    {

        BigBodyStrategy::BigBodyStrategy(
            ppbox::data::SegmentMedia & media)
            : SegmentStrategy(media)
        {
        }

        BigBodyStrategy::~BigBodyStrategy()
        {
        }

        void BigBodyStrategy::byte_range(
            ppbox::data::SegmentPosition const & pos, 
            ppbox::data::SegmentRange & range)
        {
            range.beg = pos.head_size + (range.big_offset - pos.offset);
            range.end = pos.size;
        }

        void BigBodyStrategy::time_range(
            ppbox::data::SegmentPosition const & pos, 
            ppbox::data::SegmentRange & range)
        {
            range.beg = pos.head_size + (range.big_offset - pos.offset);
            range.end = pos.size;
        }

    } // namespace merge
} // namespace ppbox
