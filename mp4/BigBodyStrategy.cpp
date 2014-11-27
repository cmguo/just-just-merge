// BigBodyStrategy.cpp

#include "just/merge/Common.h"
#include "just/merge/mp4/BigBodyStrategy.h"

namespace just
{
    namespace merge
    {

        BigBodyStrategy::BigBodyStrategy(
            just::data::SegmentMedia & media)
            : SegmentStrategy(media)
        {
        }

        BigBodyStrategy::~BigBodyStrategy()
        {
        }

        void BigBodyStrategy::byte_range(
            just::data::SegmentPosition const & pos, 
            just::data::SegmentRange & range)
        {
            range.beg = pos.head_size + (range.big_offset - pos.offset);
            range.end = pos.size;
        }

        void BigBodyStrategy::time_range(
            just::data::SegmentPosition const & pos, 
            just::data::SegmentRange & range)
        {
            range.beg = pos.head_size + (range.big_offset - pos.offset);
            range.end = pos.size;
        }

    } // namespace merge
} // namespace just
