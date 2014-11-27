// MergeStatistic.h

#ifndef   _JUST_SEGMENT_MERGE_STATISTIC_H_
#define   _JUST_SEGMENT_MERGE_STATISTIC_H_

#include "just/merge/MergerBase.h"

#include <just/avbase/StreamStatistic.h>

namespace just
{
    namespace merge
    {
        struct MergeStatInfo
        {
            MergeStatInfo()
                : head_size(0)
                , tail_size(0)
                , body_size(0)
                , cur_offset(0)
                , speed(0)
                , area(boost::uint32_t(-1))
                , percent(0)
            {
            }

            boost::uint32_t head_size; // 头部数据大小
            boost::uint32_t tail_size; // 尾部大小，只要mp4头部数据在后面时为非零值
            boost::uint64_t body_size; // 体部数据大小
            boost::uint64_t cur_offset; // 当前下载的数据位置
            boost::uint32_t speed; // 单位 B/s
            boost::uint32_t area; // 0:head, 1:body
            boost::uint32_t percent; // 对应的area下载百分比
        };

        class MergeStatistic
            : public just::avbase::StreamStatistic
        {
        public:
            MergeStatistic(
                MergerBase & merger);

            virtual void update_stat(
                boost::system::error_code & ec);

        private:
            MergerBase & merger_;
        };
    }
}

#endif // END _JUST_SEGMENT_JOIN_STATISTIC_H_
