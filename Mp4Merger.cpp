// Mp4Merger.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/Mp4Merger.h"

#include "ppbox/merge/mp4/BigHeadStrategy.h"
#include "ppbox/merge/mp4/BigBodyStrategy.h"
#include "ppbox/merge/mp4/BigTailStrategy.h"
#include "ppbox/merge/mp4/Mp4HeadStrategy.h"

#include <ppbox/data/strategy/BodyStrategy.h>

namespace ppbox
{
    namespace merge
    {

        Mp4Merger::Mp4Merger(
            boost::asio::io_service & io_svc, 
            ppbox::data::SegmentMedia & media)
            : Merger(io_svc, media)
            , big_head_(false)
            , head_strategy_(NULL)
        {
            config_.register_module("Mp4Merger")
                << CONFIG_PARAM_NAME_RDWR("big_head", big_head_);
        }

        Mp4Merger::~Mp4Merger()
        {
        }

        bool Mp4Merger::byte_seek(
            boost::uint64_t offset, 
            boost::system::error_code & ec)
        {
            if (!big_head_ 
                && !head_strategy_->check(offset, *buffer_, ec) 
                && ec != boost::asio::error::would_block) {
                    return false;
            }
            return Merger::byte_seek(offset, ec);
        }

        void Mp4Merger::set_strategys(void)
        {
            ppbox::data::SegmentStrategy * strategy = NULL;
            if (big_head_) {
                strategy = new BigHeadStrategy(media_);
                add_strategy(*strategy);
                strategy = new BigBodyStrategy(media_);
                add_strategy(*strategy);
                strategy = new BigTailStrategy(media_);
                add_strategy(*strategy);
            } else {
                strategy = head_strategy_ = new Mp4HeadStrategy(media_, *buffer_);
                add_strategy(*strategy);
                strategy = new ppbox::data::BodyStrategy(media_);
                add_strategy(*strategy);
            }
        }

    } // namespace merge
} // namespace ppbox
