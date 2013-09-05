// MergeBase.h

#ifndef _PPBOX_MERGE_MERGE_BASE_H_
#define _PPBOX_MERGE_MERGE_BASE_H_

#include <ppbox/data/base/MediaInfo.h>
#include <ppbox/data/base/StreamStatus.h>
#include <ppbox/data/base/DataStat.h>
#include <ppbox/data/segment/SegmentInfo.h>

#include <ppbox/avbase/StreamInfo.h>
#include <ppbox/avbase/Sample.h>

namespace ppbox
{
    namespace merge
    {

        using ppbox::data::MediaInfo;
        using ppbox::data::StreamStatus;
        using ppbox::data::DataStat;
        using ppbox::data::SegmentInfo;

        using ppbox::avbase::Sample;
        using ppbox::avbase::StreamInfo;

    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_MERGE_BASE_H_
