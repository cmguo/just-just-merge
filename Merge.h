// Merge.h

#ifndef   _PPBOX_MERGE_MERGE_H_
#define   _PPBOX_MERGE_MERGE_H_

#include "ppbox/merge/MergeStatistic.h"
#include <framework/string/Url.h>
#include <boost/asio/buffer.hpp>

#include <util/stream/Sink.h>
#include <framework/memory/PrivateMemory.h>
#include <util/buffers/CycleBuffers.h>

namespace ppbox
{
    namespace data
    {
        class SourceStrategy;
        class SegmentSource;
    }

    namespace merge
    {
        typedef boost::function<void (
            boost::system::error_code const &)
        >  response_type;

        class Merge
            : public MergeStatistic
        {
        public:
            Merge(
                boost::asio::io_service & io_srv, 
                boost::uint32_t buffer_size);

            ~Merge();

            virtual void async_open(
                framework::string::Url const & playlink, 
                response_type const & resp);

            virtual std::size_t read(
                std::vector<boost::asio::const_buffer> & buffers,
                boost::system::error_code & ec);

            virtual boost::system::error_code seek(
                std::size_t offset, 
                boost::system::error_code & ec);

            virtual boost::uint32_t get_buffer_time(void) const;

            virtual std::size_t valid() const;

            virtual std::size_t size() const;

            virtual void cancel(boost::system::error_code & ec);

            virtual void close(boost::system::error_code & ec);

        protected:
            boost::uint64_t const & cur_offset(void) const;

            void cur_offset(boost::uint64_t const & offset);

            ppbox::data::SegmentSource * source(void);

            void response(response_type const & resp);

            void response(boost::system::error_code const & ec);

            void add_strategy(ppbox::data::SourceStrategy * strategy);

            std::vector<ppbox::data::SourceStrategy *> const & 
                strategys(void) const;

        private:
            void open_callback(boost::system::error_code const & ec);

            boost::system::error_code prepare(
                boost::system::error_code & ec);

            void drop(std::size_t size);

            void drop_all(void);

            void buffer_copy(
                util::buffers::CycleBuffers<boost::asio::mutable_buffers_1>::ConstBuffers const & from,
                std::vector<boost::asio::const_buffer> & to);

        private:
            boost::asio::io_service & io_srv_;
            char * buffer_;
            boost::uint32_t buffer_size_;
            boost::uint32_t prepare_size_;

            // buffer
            util::buffers::CycleBuffers<
                boost::asio::mutable_buffers_1 > * cycle_buffers_;
            boost::uint32_t read_size_;
            boost::uint64_t cur_offset_;

            ppbox::data::SegmentSource * segment_source_;
            std::vector<ppbox::data::SourceStrategy *> strategys_;
            boost::uint32_t cur_pos_;

            response_type resp_;
            framework::memory::PrivateMemory private_memory_;
        };
    }
}

#endif // End _PPBOX_MERGE_MERGE_H_ 
