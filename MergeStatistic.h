// MergeStatistic.h

#ifndef   _PPBOX_SEGMENT_MERGE_STATISTIC_H_
#define   _PPBOX_SEGMENT_MERGE_STATISTIC_H_

#include "ppbox/merge/MergerBase.h"

#include <ppbox/avbase/StreamStatistic.h>

namespace ppbox
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

            boost::uint32_t head_size; // ͷ�����ݴ�С
            boost::uint32_t tail_size; // β����С��ֻҪmp4ͷ�������ں���ʱΪ����ֵ
            boost::uint64_t body_size; // �岿���ݴ�С
            boost::uint64_t cur_offset; // ��ǰ���ص�����λ��
            boost::uint32_t speed; // ��λ B/s
            boost::uint32_t area; // 0:head, 1:body
            boost::uint32_t percent; // ��Ӧ��area���ذٷֱ�
        };

        class MergeStatistic
            : public ppbox::avbase::StreamStatistic
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

#endif // END _PPBOX_SEGMENT_JOIN_STATISTIC_H_
