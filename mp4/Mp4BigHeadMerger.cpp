// Mp4BigHeadMerger.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/mp4/Mp4BigHeadMerger.h"
#include "ppbox/merge/mp4/BigHeadStrategy.h"
#include "ppbox/merge/mp4/BigBodyStrategy.h"
#include "ppbox/merge/mp4/BigTailStrategy.h"

namespace ppbox
{
    namespace merge
    {

        Mp4BigHeadMerger::Mp4BigHeadMerger(
            boost::asio::io_service & io_svc, 
            ppbox::data::MediaBase & media)
            : MergerBase(io_svc, media)
        {
        }

        Mp4BigHeadMerger::~Mp4BigHeadMerger()
        {
        }

        void Mp4BigHeadMerger::set_strategys(void)
        {
            ppbox::data::SegmentStrategy * strategy = NULL;
            strategy = new BigHeadStrategy(media_);
            add_strategy(*strategy);
            strategy = new BigBodyStrategy(media_);
            add_strategy(*strategy);
            strategy = new BigTailStrategy(media_);
            add_strategy(*strategy);
        }

    } // namespace merge
} // namespace ppbox
