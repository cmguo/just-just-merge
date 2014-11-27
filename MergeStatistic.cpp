// MergeStatistic.cpp

#include "just/merge/Common.h"
#include "just/merge/MergeStatistic.h"

namespace just
{
    namespace merge
    {
        MergeStatistic::MergeStatistic(
            MergerBase & merger)
            : merger_(merger)
        {
        }

        void MergeStatistic::update_stat(
            boost::system::error_code & ec)
        {
            merger_.stream_status(*this);
        }

    }
}
