// DemuxerModule.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/MergeModule.h"
#include "ppbox/merge/Version.h"
#include "ppbox/merge/MergerBase.h"
#include "ppbox/merge/MergeError.h"
#include "ppbox/merge/Mp4Merger.h"

#include "ppbox/data/MediaBase.h"
#include "ppbox/data/SourceBase.h"

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

#include <boost/bind.hpp>
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.merge.MergeModule", framework::logger::Debug);

namespace ppbox
{
    namespace merge
    {

        struct MergeModule::MergeInfo
        {
            enum StatusEnum
            {
                closed, 
                opening, 
                canceled, 
                opened, 
            };

            size_t id;
            StatusEnum status;
            MergerBase * merger;
            framework::string::Url play_link;
            MergeModule::open_response_type resp;
            error_code ec;

            MergeInfo(
                MergerBase * merger)
                : status(closed)
                , merger(merger)
            {
                static size_t sid = 0;
                id = ++sid;
            }

            struct Finder
            {
                Finder(
                    size_t id)
                    : id_(id)
                {
                }

                bool operator()(
                    MergeInfo const * info)
                {
                    return info->id == id_;
                }

            private:
                size_t id_;
            };
        };

        MergeModule::MergeModule(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<MergeModule>(daemon, "DemuxerModule")
        {
        }

        MergeModule::~MergeModule()
        {
        }

        error_code MergeModule::startup()
        {
            error_code ec;
            return ec;
        }

        void MergeModule::shutdown()
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::vector<MergeInfo *>::iterator iter = mergers_.begin();
            for (size_t i = mergers_.size() - 1; i != (size_t)-1; --i) {
                error_code ec;
                close_locked(mergers_[i], false, ec);
            }
            /*while (!mergers_.empty()) {
                cond_.wait(lock);
            }*/
        }

        struct SyncResponse
        {
            SyncResponse(
                error_code & ec, 
                MergerBase *& merger, 
                boost::condition_variable & cond, 
                boost::mutex & mutex)
                : ec_(ec)
                , merger_(merger)
                , cond_(cond)
                , mutex_(mutex)
                , is_return_(false)
            {
            }

            void operator()(
                error_code const & ec, 
                MergerBase * merger)
            {
                boost::mutex::scoped_lock lock(mutex_);
                ec_ = ec;
                merger_ = merger;
                is_return_ = true;

                cond_.notify_all();
            }

            void wait(
                boost::mutex::scoped_lock & lock)
            {
                while (!is_return_)
                    cond_.wait(lock);
            }

        private:
            error_code & ec_;
            MergerBase *& merger_;
            boost::condition_variable & cond_;

            boost::mutex & mutex_;
            bool is_return_;
        };

        MergerBase * MergeModule::open(
            framework::string::Url const & play_link, 
            size_t & close_token, 
            error_code & ec)
        {
            MergerBase * merger = NULL;
            SyncResponse resp(ec, merger, cond_, mutex_);
            MergeInfo * info = create(play_link, boost::ref(resp), ec);
            close_token = info->id;
            boost::mutex::scoped_lock lock(mutex_);
            mergers_.push_back(info);
            if (!ec) {
                async_open(lock, info);
                resp.wait(lock);
            }
            return merger;
        }

        void MergeModule::async_open(
            framework::string::Url const & play_link, 
            size_t & close_token, 
            open_response_type const & resp)
        {
            error_code ec;
            MergeInfo * info = create(play_link, resp, ec);
            close_token = info->id;
            boost::mutex::scoped_lock lock(mutex_);
            mergers_.push_back(info);
            if (ec) {
                io_svc().post(boost::bind(resp, ec, info->merger));
            } else {
                async_open(lock, info);
            }
        }

        error_code MergeModule::close(
            size_t id, 
            error_code & ec)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::vector<MergeInfo *>::const_iterator iter = 
                std::find_if(mergers_.begin(), mergers_.end(), MergeInfo::Finder(id));
            //assert(iter != mergers_.end());
            if (iter == mergers_.end()) {
                ec = framework::system::logic_error::item_not_exist;
            } else {
                close_locked(*iter, false, ec);
            }
            return ec;
        }

        MergeModule::MergeInfo * MergeModule::create(
            framework::string::Url const & play_link, 
            open_response_type const & resp, 
            error_code & ec)
        {
            ppbox::data::SegmentMedia * media = 
                (ppbox::data::SegmentMedia *)ppbox::data::MediaBase::create(io_svc(), play_link);
            //MergerBase * merger = new MergerBase(io_svc(), *media);
            MergerBase * merger = new Mp4Merger(io_svc(), *media);
            boost::mutex::scoped_lock lock(mutex_);
            MergeInfo * info = new MergeInfo(merger);
            info->play_link = play_link;
            info->resp = resp;
            return info;
        }

        void MergeModule::async_open(
            boost::mutex::scoped_lock & lock, 
            MergeInfo * info)
        {
            MergerBase * merger = info->merger;
            lock.unlock();
            merger->async_open(
                boost::bind(&MergeModule::handle_open, this, _1, info));
            lock.lock();
            info->status = MergeInfo::opening;
        }

        void MergeModule::handle_open(
            error_code const & ecc,
            MergeInfo * info)
        {
            boost::mutex::scoped_lock lock(mutex_);

            error_code ec = ecc;
            
            MergerBase * merger = info->merger;

            open_response_type resp;
            resp.swap(info->resp);
            //要放在close_locked之前对info->ec赋值
            info->ec = ec;
            if (info->status == MergeInfo::canceled) {
                close_locked(info, true, ec);
                ec = boost::asio::error::operation_aborted;
            } else {
                info->status = MergeInfo::opened;
            }

            lock.unlock();

            resp(ec, merger);
        }

        error_code MergeModule::close_locked(
            MergeInfo * info, 
            bool inner_call, 
            error_code & ec)
        {
            assert(!inner_call || info->status == MergeInfo::opening || info->status == MergeInfo::canceled);
            if (info->status == MergeInfo::closed) {
                ec = error::not_open;
            } else if (info->status == MergeInfo::opening) {
                info->status = MergeInfo::canceled;
                cancel(info, ec);
            } else if (info->status == MergeInfo::canceled) {
                if (inner_call) {
                    info->status = MergeInfo::closed;
                    ec.clear();
                } else {
                    ec = error::not_open;
                }
            } else if (info->status == MergeInfo::opened) {
                info->status = MergeInfo::closed;
            }
            if (info->status == MergeInfo::closed) {
                close(info, ec);
                destory(info);
            }
            return ec;
        }

        error_code MergeModule::cancel(
            MergeInfo * info, 
            error_code & ec)
        {
            MergerBase * merger = info->merger;
            merger->cancel(ec);
            cond_.notify_all();
            return ec;
        }

        error_code MergeModule::close(
            MergeInfo * info, 
            error_code & ec)
        {
            MergerBase * merger = info->merger;
            merger->close(ec);
            return ec;
        }

        void MergeModule::destory(
            MergeInfo * info)
        {
            MergerBase * merger = info->merger;
            delete merger;
            merger = NULL;
            mergers_.erase(
                std::remove(mergers_.begin(), mergers_.end(), info), 
                mergers_.end());
            delete info;
            info = NULL;
            cond_.notify_all();
        }

        MergerBase * MergeModule::find(
            framework::string::Url const & play_link)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::vector<MergeInfo *>::const_iterator iter = mergers_.begin();
            for (size_t i = mergers_.size() - 1; i != (size_t)-1; --i) {
                if ((*iter)->play_link == play_link) {
                    return (*iter)->merger;
                }
            }
            return NULL;
        }

    } // namespace merge
} // namespace ppbox
