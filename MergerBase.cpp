// MergerBase.cpp

#include "just/merge/Common.h"
#include "just/merge/MergerBase.h"

namespace just
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
} // namespace just
