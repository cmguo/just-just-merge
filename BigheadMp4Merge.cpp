// BigheadMp4Merge.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/BigheadMp4Merge.h"
#include "ppbox/data/SegmentSource.h"
#include "ppbox/data/strategy/CdnHeadStrategy.h"
#include "ppbox/data/strategy/BodySourceStrategy.h"
#include "ppbox/data/strategy/CdnTailStrategy.h"
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
                SourceStrategy * strategy = SourceStrategy::create(
                    "cdnh", 
                    source()->segments(), 
                    source()->video_info());
                assert(strategy);
                total += strategy->size();
                add_strategy(strategy);
                strategy = SourceStrategy::create(
                    "body", 
                    source()->segments(), 
                    source()->video_info());
                assert(strategy);
                total += strategy->size();
                add_strategy(strategy);
                if (total < source()->video_info().file_size) {
                    strategy = SourceStrategy::create(
                        "cdnt", 
                        source()->segments(), 
                        source()->video_info());
                    assert(strategy);
                    total += strategy->size();
                    add_strategy(strategy);
                }
            }
            resp_(ec);
        }
    } // namespace merge
} // namespace ppbox
