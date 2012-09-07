// BigheadMp4Merge.h

#ifndef   _PPBOX_SEGMENT_BIGHEAD_MP4_Merge_H_
#define   _PPBOX_SEGMENT_BIGHEAD_MP4_Merge_H_

#include "ppbox/merge/Merge.h"

namespace ppbox
{
    namespace merge
    {

        class BigheadMp4Merge
            : public Merge
        {
        public:
            BigheadMp4Merge(
                boost::asio::io_service & io_srv, 
                boost::uint32_t buffer_size);

            ~BigheadMp4Merge();

             void async_open(
                framework::string::Url const & playlink, 
                response_type const & resp);

        private:
            void open_callback(boost::system::error_code const & ec);

        private:
            response_type resp_;

        };
    }
}

#endif // END _PPBOX_SEGMENT_BIGHEAD_MP4_Merge_H_
