// Mp4Merge.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/impl/Mp4MergeImpl.h"
#include "ppbox/merge/impl/Ap4ReadStreamByteStream.h"
#include "ppbox/merge/impl/Ap4WriteStreamByteStream.h"
#include "ppbox/merge/MergeError.h"

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/logger/StringRecord.h>
using namespace framework::logger;
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Mp4MergeImpl", 0);

namespace ppbox
{
    namespace merge
    {
        Mp4MergeImpl::Mp4MergeImpl()
        {
        }

       Mp4MergeImpl::~Mp4MergeImpl()
        {
        }

        error_code Mp4MergeImpl::meger_small_head(
            std::iostream & ios,
            std::ostream & os, 
            std::vector<ppbox::avformat::SegmentInfo> & segment_infos,
            boost::uint32_t & head_size,
            boost::system::error_code & ec)
        {
            //if (1 == segment_infos.size()) {
            //    return boost::system::error_code();
            //}

            // ios copy to memory
            std::string string_buf;
            std::stringstream contents(string_buf, std::ios::in|std::ios::out|std::ios::binary);
            ios.seekg(0, std::ios::beg);
            contents << ios.rdbuf(); // refine: error handle

            boost::system::error_code lec;
            boost::uint64_t offset = 0;
            AP4_ByteStream* obj_stream = NULL;
            AP4_Result result = AP4_ReadStreamByteStream::Create(contents, 
                offset, 
                segment_infos[0].head_length,
                obj_stream);

            ppbox::avformat::Ap4HeadMerge obj_mp4_head;
            obj_mp4_head.stream_ = obj_stream;
            obj_mp4_head.CheckMp4Head(ec);
            if (ec) {
                return ec;
            }
            obj_mp4_head.SetContentLength(segment_infos[0].file_length);
            offset = segment_infos[0].head_length;
            boost::uint64_t segment_head_size = 0;
            for (boost::uint32_t i = 1; i < segment_infos.size(); ++i) {
                segment_head_size = segment_infos[i].head_length;
                ppbox::avformat::Ap4HeadMerge segment_mp4_head;
                result = AP4_ReadStreamByteStream::Create(
                    ios,
                    offset,
                    segment_infos[i].head_length,
                    segment_mp4_head.stream_);

                segment_mp4_head.CheckMp4Head(ec);
                if (ec) {
                    break;
                }
                segment_mp4_head.SetContentLength(segment_infos[i].file_length);
                ec = obj_mp4_head.Merge(segment_mp4_head);
                if (ec) {
                    break;
                }
                offset += segment_infos[i].head_length;
            }

            if (!ec) {
                AP4_ByteStream * ap4_ostream = NULL;
                result = AP4_WriteStreamByteStream::Create(os, 0, 0, ap4_ostream);

                result = AP4_FileWriter::PptvWrite(*obj_mp4_head.file_,
                    *ap4_ostream,
                    obj_mp4_head.content_length_
                    - obj_mp4_head.head_size_
                    + obj_mp4_head.mdat_head_size_);
                if (AP4_SUCCESS != result) {
                    ec = ppbox::merge::error::write_mp4_head_error;
                }

            }
            head_size = obj_mp4_head.head_size_;
            return ec;
        }

        error_code Mp4MergeImpl::vod_valid_segment_info(
            std::istream & is,
            std::vector<ppbox::avformat::SegmentInfo> & segment_infos,
            boost::system::error_code & ec)
        {
            is.seekg(0, std::ios::end);
            boost::uint32_t big_head_size = is.tellg();
            is.seekg(0, std::ios::beg);
            AP4_ByteStream* obj_stream = NULL;
            AP4_ReadStreamByteStream::Create(is, 
                0, 
                big_head_size,
                obj_stream);

            ppbox::avformat::Ap4HeadMerge mp4_head;
            mp4_head.stream_ = obj_stream;
            mp4_head.CheckMp4Head(ec);
            boost::uint64_t duration_sum = 0;
            if (!ec) {
                for (boost::uint32_t i = 0; i < segment_infos.size()-1; ++i) {
                    duration_sum += segment_infos[i].duration;
                    boost::uint32_t diff_offset;
                    mp4_head.FindMinOffset(duration_sum, diff_offset, i, segment_infos, ec);
                    if (ec) {
                        break;
                    }
                    segment_infos[i].file_length -= diff_offset;
                    //segment_infos[i].file_length -= 34259;
                    //diff_offset = diff_offset;
                }
            }
            return ec;
        }

    } // namespace Mp4Merge
} // namespace ppbox