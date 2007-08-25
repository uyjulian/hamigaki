// sliding_window.hpp: sliding window compression/decompression

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_FILTER_SLIDING_WINDOW_HPP
#define HAMIGAKI_IOSTREAMS_FILTER_SLIDING_WINDOW_HPP

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <algorithm>
#include <cstring>
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

template<class Output>
class sliding_window_compress_impl
{
    typedef typename Output::length_type length_type;
    typedef typename Output::offset_type offset_type;
    typedef std::pair<offset_type,length_type> pair_type;

    static const length_type min_match_length = Output::min_match_length;
    static const length_type max_match_length = Output::max_match_length;

#if defined(HAMIGAKI_IOSTREAMS_USE_HASH)
    static const std::size_t hash_table_size = 0x8000;
    static const std::size_t hash_table_mask = 0x7FFF;
    static const std::size_t nil = ~static_cast<std::size_t>(0);
#endif

public:
    sliding_window_compress_impl(const Output& output, std::size_t window_bits)
        : output_(output)
        , window_size_(1 << window_bits)
        , window_size2_(window_size_ <<1 )
        , window_(window_size2_ + max_match_length, ' ')
        , pos_(0)
        , end_(0)
        , last_(0, 0)
#if defined(HAMIGAKI_IOSTREAMS_USE_HASH)
        , hash_chain_(window_size2_ + max_match_length)
        , hash_table_(hash_table_size, nil)
        , hash_value_(0)
#endif
    {
    }

    template<class Sink>
    std::streamsize write(Sink& sink, const char* s, std::streamsize n)
    {
        std::streamsize total = 0;

        while (true)
        {
            while (end_ - pos_ >= max_match_length)
            {
                pair_type res = search(max_match_length);
                if (res.second < min_match_length)
                    res.second = 1;

                if (!last_.second)
                {
                    ++pos_;
                    last_ = res;
                }
                else if (
                    (last_.second >= min_match_length) &&
                    (last_.second >= res.second))
                {
                    output_.put(sink, last_.first, last_.second);
#if defined(HAMIGAKI_IOSTREAMS_USE_HASH)
                    --last_.second;
                    while (--last_.second)
                    {
                        ++pos_;
                        const unsigned char uc =
                            static_cast<unsigned char>(window_[pos_ + 2]);
                        hash_value_ =
                            ((hash_value_ << 5) ^ uc) & hash_table_mask;

                        std::size_t prev = hash_table_[hash_value_];
                        hash_table_[hash_value_] = pos_;
                        hash_chain_[pos_] = prev;
                    }
                    ++pos_;
#else
                    pos_ += (last_.second - 1);
#endif
                    last_.second = 0;
                }
                else
                {
                    output_.put(sink, window_[pos_-1]);
                    ++pos_;
                    last_ = res;
                }

                if (pos_ >= window_size2_)
                    slide();
            }

            if (total < n)
            {
                std::streamsize amt = (std::min)(n-total,
                    static_cast<std::streamsize>(window_.size() - end_));
                std::memcpy(&window_[end_], s+total, amt);
                end_ += amt;
                total += amt;
#if defined(HAMIGAKI_IOSTREAMS_USE_HASH)
                if (pos_ == 0)
                {
                    hash_value_ =
                        (
                            (static_cast<unsigned char>(window_[0]) << 5) ^
                            (static_cast<unsigned char>(window_[1])     )
                        ) & hash_table_mask;
                }
#endif
            }
            else
                break;
        }

        return total;
    }

    template<class Sink>
    bool flush(Sink& sink)
    {
        while (pos_ < end_)
        {
            length_type max_len = end_-pos_;

            pair_type res = search(max_len);
            if (res.second < min_match_length)
                res.second = 1;

            if (!last_.second)
            {
                ++pos_;
                last_ = res;
            }
            else if (
                (last_.second >= min_match_length) &&
                (last_.second >= res.second))
            {
                output_.put(sink, last_.first, last_.second);
#if defined(HAMIGAKI_IOSTREAMS_USE_HASH)
                --last_.second;
                while (--last_.second)
                {
                    ++pos_;
                    const unsigned char uc =
                        static_cast<unsigned char>(window_[pos_ + 2]);
                    hash_value_ =
                        ((hash_value_ << 5) ^ uc) & hash_table_mask;

                    std::size_t prev = hash_table_[hash_value_];
                    hash_table_[hash_value_] = pos_;
                    hash_chain_[pos_] = prev;
                }
                ++pos_;
#else
                pos_ += (last_.second - 1);
#endif
                last_.second = 0;
            }
            else
            {
                output_.put(sink, window_[pos_-1]);
                ++pos_;
                last_ = res;
            }

            if (pos_ >= window_size2_)
                slide();
        }

        if (last_.second && (pos_ == end_))
            output_.put(sink, window_[pos_-1]);

        return output_.flush(sink);
    }

private:
    Output output_;
    std::size_t window_size_;
    std::size_t window_size2_;
    std::vector<char> window_;
    std::size_t pos_;
    std::size_t end_;
    pair_type last_;
#if defined(HAMIGAKI_IOSTREAMS_USE_HASH)
    std::vector<std::size_t> hash_chain_;
    std::vector<std::size_t> hash_table_;
    std::size_t hash_value_;
#endif

    // Note: too slow
    pair_type search(length_type max_len)
    {
#if defined(HAMIGAKI_IOSTREAMS_USE_HASH)
        const char* str = &window_[pos_];
        const unsigned char uc = static_cast<unsigned char>(str[2]);
        hash_value_ = ((hash_value_ << 5) ^ uc) & hash_table_mask;
        std::size_t index = hash_table_[hash_value_];
        length_type length = 0;
        offset_type offset = 0;
        while (index != nil)
        {
            if (pos_ - index >= window_size_)
                break;

            const char* cur = &window_[index];
            const char* end = std::mismatch(cur, cur+max_len, str).first;
            length_type len = static_cast<length_type>(end - cur);
            if (len > length)
            {
                length = len;
                offset = str-cur - 1;
            }
            index = hash_chain_[index];
        }

        std::size_t prev = hash_table_[hash_value_];
        hash_table_[hash_value_] = pos_;
        hash_chain_[pos_] = prev;

        return pair_type(offset, length);
#else
        const char* last = &window_[pos_];
        const char* start =
            (pos_ >= window_size_)
            ? last - window_size_ : &window_[0];

        length_type length = 0;
        offset_type offset = 0;
        while (const void* p = std::memchr(start, *last, last-start))
        {
            const char* cur = static_cast<const char*>(p);

            const char* end =
                std::mismatch(cur, cur+max_len, last).first;

            length_type len = static_cast<length_type>(end - cur);
            if (len >= length)
            {
                length = len;
                offset = last-cur - 1;
            }
            start = cur + 1;
        }
        return pair_type(offset, length);
#endif
    }

    void slide()
    {
        std::memmove(
            &window_[0],
            &window_[window_size_],
            window_size_+max_match_length
        );
        pos_ -= window_size_;
        end_ -= window_size_;
#if defined(HAMIGAKI_IOSTREAMS_USE_HASH)
        for (std::size_t i = 0; i < hash_table_size; ++i)
        {
            std::size_t* ptr = &hash_table_[i];
            std::size_t index = *ptr;
            while (index != nil)
            {
                if (index < window_size_)
                {
                    *ptr = nil;
                    break;
                }
                else
                {
                    *ptr = index - window_size_;
                    index = hash_chain_[index];
                    ptr = &hash_chain_[*ptr];
                }
            }
            *ptr = nil;
        }
#endif
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

template<class Output>
class sliding_window_compress
{
private:
    typedef detail::sliding_window_compress_impl<Output> impl_type;

public:
    typedef char char_type;
    struct category
        : public boost::iostreams::output
        , public boost::iostreams::filter_tag
        , public boost::iostreams::multichar_tag
        , public boost::iostreams::flushable_tag
    {};

    sliding_window_compress(const Output& output, std::size_t window_bits)
        : pimpl_(new impl_type(output, window_bits))
    {
    }

    template<class Sink>
    bool flush(Sink& sink)
    {
        return pimpl_->flush(sink);
    }

    template<class Sink>
    std::streamsize write(Sink& sink, const char* s, std::streamsize n)
    {
        return pimpl_->write(sink, s, n);
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_FILTER_SLIDING_WINDOW_HPP
