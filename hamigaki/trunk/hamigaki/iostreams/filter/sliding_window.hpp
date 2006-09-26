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
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <vector>

namespace hamigaki { namespace iostreams {

template<class Offset, class Length>
struct literal_or_reference
{
    bool is_reference;

    union
    {
        char literal;

        struct
        {
            Offset offset;
            Length length;
        };
    };

    explicit literal_or_reference(char c)
    {
        is_reference = false;
        literal = c;
    }

    literal_or_reference(Offset off, Length len)
    {
        is_reference = true;
        offset = off;
        length = len;
    }
};

namespace detail
{

template<class Input>
class sliding_window_decompress_impl
{
    typedef typename Input::length_type length_type;
    typedef typename Input::offset_type offset_type;
    typedef literal_or_reference<offset_type,length_type> result_type;
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
    std::streamsize read(Source& src, char* s, std::streamsize n)
    {
        std::streamsize total = 0;

        if (length_)
        {
            std::streamsize amt = (std::min)
                (n, static_cast<std::streamsize>(length_));

            copy_from_window(s, amt);
            s += amt;
            n -= amt;
            total += amt;
        }

        while (n > 0)
        {
            const result_type& data = input_.get(src);
            if (data.is_reference)
            {
                offset_ = data.offset;
                length_ = data.length;
                if (!length_)
                    break;

                std::streamsize amt = (std::min)
                    (n, static_cast<std::streamsize>(length_));

                copy_from_window(s, amt);
                s += amt;
                n -= amt;
                total += amt;
            }
            else
            {
                *(s++) = push_back(data.literal);
                --n;
                ++total;
            }
        }

        return (total != 0) ? total : -1;
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

    void copy_from_window(char* s, std::streamsize n)
    {
        length_ -= n;
        for (std::streamsize i = 0; i < n; ++i)
        {
            char c = window_[(pos_ + window_size1_ - offset_) & window_size1_];
            *(s++) = push_back(c);
        }
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
        , public boost::iostreams::multichar_tag
    {};

    sliding_window_decompress(const Input& input, std::size_t window_bits)
        : pimpl_(new impl_type(input, window_bits))
    {
    }

    template<class Source>
    std::streamsize read(Source& src, char* s, std::streamsize n)
    {
        return pimpl_->read(src, s, n);
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_FILTER_SLIDING_WINDOW_HPP
