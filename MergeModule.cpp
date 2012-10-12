// MergeModule.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/MergeModule.h"
#include "ppbox/merge/BigheadMp4Merge.h"
#include "ppbox/merge/Mp4Merge.h"

#include <boost/bind.hpp>
using namespace boost::system;

namespace ppbox
{
    namespace merge
    {

        MergeModule::MergeModule(util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<MergeModule>(daemon, "MergeModule")
        {
        }

        MergeModule::~MergeModule()
        {
        }

        boost::system::error_code MergeModule::startup()
        {
            boost::system::error_code ec;
            return ec;
        }

        void MergeModule::shutdown()
        {
        }

        void MergeModule::async_open(
            framework::string::Url const & url, 
            MergeType::Enum const & type, 
            size_t & close_token, 
            open_response_type const & resp)
        {
            static std::size_t id = 0;
            MergeInfo merge_info;
            merge_info.playlink = url;
            merge_info.id = id++;
            close_token = merge_info.id;
            merge_info.resp = resp;
            boost::uint32_t buffer_size = 10*1024*1024;
            if (type == MergeType::bighead) {
                merge_info.merge = new BigheadMp4Merge(io_svc(), buffer_size);
            } else if (type == MergeType::smallhead) {
                merge_info.merge = new Mp4Merge(io_svc(), buffer_size);
            } else if (type == MergeType::full) {
                merge_info.merge = new Merge(io_svc(), buffer_size);
            } else {
                assert(0);
            }
            merge_info.state = StateType::opening;
            merge_info.merge->async_open(
                merge_info.playlink, 
                NULL, 
                boost::bind(&MergeModule::open_callback, this, _1, merge_info.id));
            boost::mutex::scoped_lock lock(mutex_);
            merge_infos_.push_back(merge_info);
        }

        void MergeModule::open_callback(
            error_code const & ec, 
            std::size_t const & close_token)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::vector<MergeInfo>::iterator iter = 
                std::find_if(merge_infos_.begin(), merge_infos_.end(), MergeInfo::MergeFinder(close_token));
            assert(iter != merge_infos_.end());
            boost::system::error_code lec = ec;
            if ((*iter).state == StateType::canceled) {
                delete (*iter).merge;
                lec = framework::system::logic_error::operation_canceled;
                (*iter).resp(lec, NULL);
                merge_infos_.erase(iter);
            } else {
                (*iter).state = StateType::opened;
                (*iter).resp(lec, (*iter).merge);
            }
        }

        error_code MergeModule::close(
            std::size_t close_token, 
            error_code & ec)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::vector<MergeInfo>::iterator iter = 
                std::find_if(merge_infos_.begin(), merge_infos_.end(), MergeInfo::MergeFinder(close_token));
            if (iter == merge_infos_.end()) {
                ec = framework::system::logic_error::item_not_exist;
            } else {
                MergeInfo info = *iter;
                if (info.state == StateType::opening) {
                    (*iter).state = StateType::canceled;
                    info.merge->cancel(ec);
                } else if (info.state == StateType::opened) {
                    info.merge->close(ec);
                    delete info.merge;
                    merge_infos_.erase(iter);
                } else {
                    delete info.merge;
                    merge_infos_.erase(iter);
                }
            }
            return ec;
        }

        Merge * MergeModule::find(framework::string::Url const & url)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::vector<MergeInfo>::const_iterator iter = merge_infos_.begin();
            for (; iter != merge_infos_.end(); iter++) {
                if (iter->playlink.to_string() == url.to_string()) {
                    return iter->merge;
                }
            }
            return NULL;
        }

    }
}
