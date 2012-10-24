// Mp4Merger.h

#ifndef _PPBOX_MERGE_MP4_MERGER_H_
#define _PPBOX_MERGE_MP4_MERGER_H_

#include "ppbox/merge/MergerBase.h"

namespace ppbox
{
    namespace merge
    {

        class Mp4HeadStrategy;

        class Mp4Merger
            : public MergerBase
        {
        public:
            Mp4Merger(
                boost::asio::io_service & io_svc, 
                ppbox::data::MediaBase & media);

            virtual ~Mp4Merger();

        public:
            virtual bool seek(
                boost::uint64_t offset, 
                boost::system::error_code & ec);

        private:
            virtual void set_strategys(void);

        private:
            bool big_head_;
            Mp4HeadStrategy * head_strategy_;
        };

    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_MP4_MERGER_H_
