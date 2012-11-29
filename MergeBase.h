// MergeBase.h

#ifndef _PPBOX_MERGE_MERGE_BASE_H_
#define _PPBOX_MERGE_MERGE_BASE_H_

#include <ppbox/data/base/MediaInfo.h>
#include <ppbox/data/base/StreamStatus.h>
#include <ppbox/data/segment/SegmentInfo.h>
#include <ppbox/avformat/Format.h>

namespace ppbox
{
    namespace merge
    {

        using ppbox::data::MediaInfo;
        using ppbox::data::StreamStatus;
        using ppbox::data::SegmentInfo;

        using ppbox::avformat::Sample;

    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_MERGE_BASE_H_
