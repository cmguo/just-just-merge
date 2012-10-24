// Mp4BigHeadMerger.h

#ifndef _PPBOX_MERGE_MP4_BIG_HEAD_MERGER_H_
#define _PPBOX_MERGE_MP4_BIG_HEAD_MERGER_H_

#include "ppbox/merge/MergerBase.h"

namespace ppbox
{
    namespace merge
    {

        class Mp4BigHeadMerger
            : public MergerBase
        {
        public:
            Mp4BigHeadMerger(
                boost::asio::io_service & io_svc, 
                ppbox::data::MediaBase & media);

            ~Mp4BigHeadMerger();

        private:
            virtual void set_strategys(void);
        };

    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_MP4_BIG_HEAD_MERGER_H_
