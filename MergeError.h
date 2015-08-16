// MergeError.h

#ifndef _JUST_MERGE_MERGE_ERROR_H_
#define _JUST_MERGE_MERGE_ERROR_H_

namespace just
{
    namespace merge
    {

        namespace error {

            enum errors
            {
                already_open = 1,   // 已经打开
                not_open,           // 未打开
                format_not_match, 
                end_of_file, 
            };

            namespace detail {
                class merge_category
                    : public boost::system::error_category
                {
                public:
                    merge_category()
                    {
                        register_category(*this);
                    }

                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "merge";
                    }

                    std::string message(int value) const
                    {
                        switch(value)
                        {
                        case error::already_open:
                            return "merge: already open";
                        case error::not_open:
                            return "merge: not open";
                        case error::format_not_match:
                            return "merge: format not match";
                        case error::end_of_file:
                            return "merge: end of file";
                        default:
                            return "merge: unknown error";
                        }
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::merge_category instance;
                return instance;
            }

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

        } // namespace error

    } // namespace merge
} // namespace just

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<just::merge::error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using just::merge::error::make_error_code;
#endif

    }
}

#endif // _JUST_DATA_SOURCE_ERROR_H_
