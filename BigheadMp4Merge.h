// BigheadMp4Merge.h

#ifndef   _PPBOX_MERGE_BIGHEAD_MP4_Merge_H_
#define   _PPBOX_MERGE_BIGHEAD_MP4_Merge_H_

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

        private:
            virtual void set_strategys(void);

        };
    }
}

#endif // END _PPBOX_MERGE_BIGHEAD_MP4_Merge_H_
