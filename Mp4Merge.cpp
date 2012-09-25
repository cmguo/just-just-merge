// Mp4Merge.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/Mp4Merge.h"
#include "ppbox/merge/impl/Mp4MergeImpl.h"
#include "ppbox/data/SourceError.h"
#include "ppbox/data/SegmentSource.h"
#include "ppbox/data/strategy/HeadStrategy.h"
#include "ppbox/data/strategy/BodyStrategy.h"
using namespace ppbox::data;

#include <boost/bind.hpp>
using namespace boost::system;

namespace ppbox
{
    namespace merge
    {
        // TODO: 要考虑播放和下载两种情况
        Mp4Merge::Mp4Merge(
            boost::asio::io_service & io_srv, 
            boost::uint32_t buffer_size)
            : Merge(io_srv, buffer_size)
        {
        }

        Mp4Merge::~Mp4Merge()
        {
        }

        void Mp4Merge::async_open(
            framework::string::Url const & playlink, 
            std::iostream * ios, 
            response_type const & resp)
        {
            resp_ = resp;
            ios_ = ios;
            source()->async_open(
                playlink, 
                0, 
                boost::uint64_t(-1), 
                boost::bind(&Mp4Merge::open_callback, this, _1));
        }

        void Mp4Merge::open_callback(error_code const & ec)
        {
            if (!ec) {
                Strategy * strategy = Strategy::create(
                    "head", 
                    *source()->media());
                assert(strategy);
                add_strategy(strategy);
                strategy = Strategy::create(
                    "body", 
                    *source()->media());
                add_strategy(strategy);
                Mp4MergeImpl * mp4_merge_impl = new Mp4MergeImpl;
                add_media_merge(mp4_merge_impl, ios_);
            }
            resp_(ec);
        }

    } // namespace merge
} // namespace ppbox
