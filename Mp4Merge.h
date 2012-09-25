// Mp4Merge.h

#ifndef   _PPBOX_MERGE_MP4_MERGE_H_
#define   _PPBOX_MERGE_MP4_MERGE_H_

#include "ppbox/merge/Merge.h"
#include "ppbox/merge/impl/Mp4MergeImpl.h"

#include <ppbox/avformat/mp4/Ap4HeadMerge.h>

namespace ppbox
{
    namespace merge
    {

        class Mp4Merge
            : public Merge
        {
        public:
            Mp4Merge(
                boost::asio::io_service & io_srv, 
                boost::uint32_t buffer_size);

            ~Mp4Merge();

             void async_open(
                framework::string::Url const & playlink, 
                std::iostream * ios, 
                response_type const & resp);

        private:
            void open_callback(boost::system::error_code const & ec);

        private:
            std::iostream * ios_;
            response_type resp_;

        };
    }
}

#endif // END _PPBOX_MERGE_MP4_MERGE_H_
