// BigheadMp4Merge.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/BigheadMp4Merge.h"
#include "ppbox/data/SegmentSource.h"
#include "ppbox/data/strategy/BigHeadStrategy.h"
#include "ppbox/data/strategy/BodyStrategy.h"
#include "ppbox/data/strategy/BigTailStrategy.h"
using namespace ppbox::data;

#include <boost/bind.hpp>
using namespace boost::system;

namespace ppbox
{
    namespace merge
    {
        BigheadMp4Merge::BigheadMp4Merge(
            boost::asio::io_service & io_srv, 
            boost::uint32_t buffer_size)
            : Merge(io_srv, buffer_size)
        {
        }

        BigheadMp4Merge::~BigheadMp4Merge()
        {
        }

        void BigheadMp4Merge::set_strategys(void)
        {
            boost::uint64_t total = 0;
            SegmentStrategy * strategy = new ppbox::data::BigHeadStrategy(*media_);
            total += strategy->byte_size();
            add_strategy(strategy);
            strategy = new ppbox::data::BodyStrategy(*media_);
            total += strategy->byte_size();
            add_strategy(strategy);
            MediaInfo info;
            error_code lec;
            media_->get_info(info, lec);
            assert(!lec);
            //if (total < info.file_size) {
            strategy = new ppbox::data::BigTailStrategy(*media_);
            total += strategy->byte_size();
            add_strategy(strategy);
            //}
        }
    } // namespace merge
} // namespace ppbox
