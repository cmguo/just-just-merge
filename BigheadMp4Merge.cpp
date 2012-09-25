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

        void BigheadMp4Merge::async_open(
            framework::string::Url const & playlink, 
            std::iostream * ios, 
            response_type const & resp)
        {
            resp_ = resp;
            source()->async_open(
                playlink, 
                0, 
                boost::uint64_t(-1), 
                boost::bind(&BigheadMp4Merge::open_callback, this, _1));
        }

        void BigheadMp4Merge::open_callback(error_code const & ec)
        {
            std::size_t total = 0;
            if (!ec) {
                Strategy * strategy = Strategy::create(
                    "cdnh", 
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
                        "cdnt", 
                        *source()->media());
                    assert(strategy);
                    total += strategy->size();
                    add_strategy(strategy);
                }
            }
            resp_(ec);
        }
    } // namespace merge
} // namespace ppbox
