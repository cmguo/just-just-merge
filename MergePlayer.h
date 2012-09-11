// MergePlayer.h

#ifndef _PPBOX_MERGE_PLAYER_H_
#define _PPBOX_MERGE_PLAYER_H_

#include <ppbox/common/Dispatcher.h>

namespace ppbox
{
    namespace merge
    {  
        class Merge;
        class MergePlayer : public ppbox::common::PlayInterface
        {
        public:

            MergePlayer();
            virtual ~MergePlayer();
            virtual boost::system::error_code doing();
            void set(
                Merge *merge_tmp,
                ppbox::common::session_callback_respone const &resp,
                ppbox::common::Session* session =NULL);
        private:
            boost::system::error_code buffering();
            boost::system::error_code playing();
    
        private:
            Merge* merge_;
            ppbox::common::Session* session_;
            ppbox::common::session_callback_respone resp_;
        };
    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_PLAYER_H_
