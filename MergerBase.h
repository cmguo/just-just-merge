// MergerBase.h

#ifndef _PPBOX_MERGE_MERGER_BASE_H_
#define _PPBOX_MERGE_MERGER_BASE_H_

#include "ppbox/merge/MergeBase.h"

#include <framework/configure/Config.h>

namespace ppbox
{
    namespace merge
    {

        class MergerBase
        {
        public:
            typedef boost::function<void (
                boost::system::error_code const &)
            >  response_type;

        public:
            MergerBase(
                boost::asio::io_service & io_svc);

            virtual ~MergerBase();

        public:
            virtual void async_open(
                response_type const & resp) = 0;

            virtual void cancel(
                boost::system::error_code & ec) = 0;

            virtual void close(
                boost::system::error_code & ec) = 0;

        public:
            virtual bool byte_seek(
                boost::uint64_t offset, 
                boost::system::error_code & ec) = 0;

            virtual bool reset(
                boost::system::error_code & ec) = 0;

            virtual bool read(
                Sample & sample,
                boost::system::error_code & ec) = 0;

            virtual bool free(
                Sample & sample,
                boost::system::error_code & ec) = 0;

            virtual bool fill_data(
                boost::system::error_code & ec) = 0;

        public:
            virtual void media_info(
                MediaInfo & info) const = 0;

            virtual void stream_info(
                std::vector<StreamInfo> & streams) const = 0;

            virtual void stream_status(
                StreamStatus & info) const = 0;

            virtual void data_stat(
                DataStat & stat) const = 0;

        public:
            boost::asio::io_service & get_io_service() const
            {
                return io_svc_;
            };

            framework::configure::Config & get_config()
            {
                return config_;
            }

        protected:
            framework::configure::Config config_;

        private:
            boost::asio::io_service & io_svc_;
        };

    } // namespace merge
} // namespace ppbox

#endif // _PPBOX_MERGE_MERGER_BASE_H_
