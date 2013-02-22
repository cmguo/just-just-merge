// BigHeadStrategy.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/mp4/BigHeadStrategy.h"

#include <ppbox/data/base/SourceError.h>

namespace ppbox
{
    namespace merge
    {

        BigHeadStrategy::BigHeadStrategy(
            ppbox::data::SegmentMedia & media)
            : SegmentStrategy(media)
        {
        }

        BigHeadStrategy::~BigHeadStrategy()
        {
        }

        bool BigHeadStrategy::next_segment(
            ppbox::data::SegmentPosition & pos, 
            boost::system::error_code & ec)
        {
            if (pos.item_context == NULL || pos.index == size_t(-1)) {
                pos.item_context = this;
                ppbox::data::MediaInfo minfo;
                media_.get_info(minfo, ec);
                ppbox::data::SegmentInfo sinfo;
                if (!media_.segment_info(0, sinfo, ec)) {
                    return false;
                }
                pos.index = 0;
                pos.size = minfo.file_size;
                pos.byte_range.before_next();
                pos.byte_range.beg = 0;
                pos.byte_range.end = sinfo.offset;
                pos.byte_range.after_next();
                pos.time_range.before_next();
                pos.time_range.beg = 0;
                pos.time_range.end = sinfo.offset;
                pos.time_range.after_next();
                ec.clear();
                return true;
            } else {
                ec = ppbox::data::source_error::no_more_segment;
                return false;
            }
        }

        bool BigHeadStrategy::get_url(
            ppbox::data::SegmentPosition const & pos, 
            framework::string::Url & url, 
            boost::system::error_code & ec)
        {
            return media_.get_url(url, ec);
        }

    } // namespace merge
} // namespace ppbox
