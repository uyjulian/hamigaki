//  sliding_window.hpp: sliding window compression/decompression

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_FILTER_SLIDING_WINDOW_HPP
#define HAMIGAKI_IOSTREAMS_FILTER_SLIDING_WINDOW_HPP

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/blank.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>
#include <vector>

namespace hamigaki { namespace iostreams {

namespace detail
{

template<class Input>
class sliding_window_decompress_impl
{
    typedef typename Input::length_type length_type;
    typedef typename Input::offset_type offset_type;
    typedef std::pair<offset_type,length_type> pair_type;
    typedef std::char_traits<char> traits_type;

    static const length_type min_match_length = Input::min_match_length;

public:
    sliding_window_decompress_impl(const Input& input, std::size_t window_bits)
        : input_(input)
        , window_size_(1 << window_bits)
        , window_size1_(window_size_-1)
        , window_(window_size_, ' ')
        , pos_(0)
        , length_(0)
    {
    }

    template<class Source>
    traits_type::int_type get(Source& src)
    {
        if (length_)
            return traits_type::to_int_type(get_by_reference());

        const boost::variant<
            boost::blank,char,pair_type>& data = input_.get(src);
        if (data.which() == 1)
            return traits_type::to_int_type(push_back(boost::get<char>(data)));
        else if (data.which() == 2)
        {
            boost::tie(offset_, length_) = boost::get<pair_type>(data);
            return traits_type::to_int_type(get_by_reference());
        }
        else
            return traits_type::eof();
    }

private:
    Input input_;
    std::size_t window_size_;
    std::size_t window_size1_;
    std::vector<char> window_;
    std::size_t pos_;
    offset_type offset_;
    length_type length_;

    char push_back(char c)
    {
        window_[pos_] = c;
        pos_ = (pos_+1) & window_size1_;
        return c;
    }

    char get_by_reference()
    {
        --length_;
        return push_back(
            window_[(pos_ + window_size1_ - offset_) & window_size1_]
        );
    }
};

} // namespace detail

template<class Input>
class sliding_window_decompress
{
private:
    typedef detail::sliding_window_decompress_impl<Input> impl_type;

public:
    typedef char char_type;
    struct category
        : public boost::iostreams::input
        , public boost::iostreams::filter_tag
    {};

    sliding_window_decompress(const Input& input, std::size_t window_bits)
        : pimpl_(new impl_type(input, window_bits))
    {
    }

    template<class Source>
    std::char_traits<char>::int_type get(Source& src)
    {
        return pimpl_->get(src);
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_FILTER_SLIDING_WINDOW_HPP
