// comment.hpp: utilities for vorbis comment

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_VORBIS_COMMENT_HPP
#define HAMIGAKI_AUDIO_VORBIS_COMMENT_HPP

#include <hamigaki/iterator/first_iterator.hpp>
#include <algorithm>
#include <cstring>
#include <string>
#include <utility>

namespace hamigaki { namespace audio { namespace vorbis {

namespace detail
{

struct split_commet :
    std::unary_function<
        const char*,
        std::pair<const std::string,std::string>
    >
{
    std::pair<const std::string,std::string>
    operator()(const char* field) const
    {
        const char* delim = std::strchr(field, '=');

        return std::pair<const std::string,std::string>(
            std::string(field, delim),
            std::string(delim+1));
    }
};

} // namespace detail

class comment_iterator
    : public boost::transform_iterator<detail::split_commet,const char**>
{
    typedef boost::transform_iterator<
        detail::split_commet,const char**> base_type;

public:
    comment_iterator()
    {
    }

    explicit comment_iterator(const char** ptr)
        : base_type(ptr, detail::split_commet())
    {
    }
};

inline std::string comment_value(
    const std::pair<const char**,const char**>& comments,
    const std::string& name)
{
    const comment_iterator end(comments.second);

    comment_iterator i =
        std::find(
            hamigaki::make_first_iterator(
                comment_iterator(comments.first)
            ),
            hamigaki::make_first_iterator(end),
            name
        ).base();

    if (i == end)
        return std::string();
    else
        return i->second;
}

} } } // End namespaces vorbis, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_VORBIS_COMMENT_HPP
