// MergeStatistic.h

#ifndef   _PPBOX_SEGMENT_MERGE_STATISTIC_H_
#define   _PPBOX_SEGMENT_MERGE_STATISTIC_H_

#include <framework/timer/TickCounter.h>

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
        {
        public:
            MergeStatistic();

            ~MergeStatistic();

            void open(
                std::string name,
                boost::uint32_t head_size,
                boost::uint32_t tail_size,
                boost::uint32_t body_size);

            void increase(
                boost::uint32_t bytes,
                boost::uint64_t offset);

            boost::system::error_code get_info_statictis(MergeStatInfo & info) const;

            boost::uint64_t const & offset(void) const;

            void close(void);

        private:
            std::string name_;
            MergeStatInfo info_;
            framework::timer::TickCounter tick_counter_;
            boost::uint32_t increase_bytes_;
        };
    }
}

#endif // END _PPBOX_SEGMENT_JOIN_STATISTIC_H_
