// MergerBase.cpp

#include "ppbox/merge/Common.h"
#include "ppbox/merge/MergerBase.h"

namespace ppbox
{
    namespace merge
    {

        MergerBase::MergerBase(
            boost::asio::io_service & io_svc)
            : io_svc_(io_svc)
        {
        }

        MergerBase::~MergerBase()
        {
        }

    } // namespace merge
} // namespace ppbox
