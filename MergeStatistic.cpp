// MergeStatistic.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/MergeStatistic.h"

namespace ppbox
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
