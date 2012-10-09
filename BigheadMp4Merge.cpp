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
            Strategy * strategy = Strategy::create(
                "bigh", 
                *source()->media());
            assert(strategy);
            total += strategy->size();
            add_strategy(strategy);
            strategy = Strategy::create(
                "body", 
                *source()->media());
            assert(strategy);
            total += strategy->size();
            add_strategy(strategy);
            MediaInfo info;
            error_code lec;
            source()->media()->get_info(info, lec);
            assert(!lec);
            if (total < info.file_size) {
                strategy = Strategy::create(
                    "bigt", 
                    *source()->media());
                assert(strategy);
                total += strategy->size();
                add_strategy(strategy);
            }
        }
    } // namespace merge
} // namespace ppbox
