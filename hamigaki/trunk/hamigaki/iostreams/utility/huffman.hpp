//  huffman.hpp: Huffman encoding/decoding

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_UTILITY_HUFFMAN_HPP
#define HAMIGAKI_IOSTREAMS_UTILITY_HUFFMAN_HPP

#include <hamigaki/iostreams/bit_stream.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/assert.hpp>
#include <boost/integer.hpp>
#include <boost/next_prior.hpp>
#include <boost/static_assert.hpp>
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace hamigaki { namespace iostreams {

template<class Value, std::size_t Bits>
class huffman_decoder
{
public:
    typedef typename boost::uint_t<Bits>::least code_type;
    typedef Value value_type;

private:
    struct node
    {
        code_type code;
        value_type value;

        node(code_type code, value_type value)
            : code(code), value(value)
        {
        }

        explicit node(value_type value) : value(value)
        {
        }
    };

    typedef std::vector<node> tree_type;

    struct node_code_less
    {
        bool operator()(const node& lhs, const node& rhs) const
        {
            return lhs.code < rhs.code;
        }
    };

    class node_bit_less
    {
    public:
        explicit node_bit_less(std::size_t bit) : mask_((1 << (Bits-1)) >> bit)
        {
        }

        bool operator()(const node& lhs, const node& rhs) const
        {
            return (lhs.code & mask_) < (rhs.code & mask_);
        }

    private:
        code_type mask_;
    };

public:
    huffman_decoder() : ready_(false)
    {
    }

    void clear()
    {
        tree_.clear();
        ready_ = false;
    }

    void reserve(std::size_t n)
    {
        tree_.reserve(n);
    }

    void assign(const value_type& x)
    {
        tree_.clear();
        tree_.push_back(node(x));
        ready_ = true;
    }

    void push_back(code_type code, const value_type& value)
    {
        if (ready_)
            throw std::runtime_error("huffman tree already setup");

        tree_.push_back(node(code, value));
    }

    void sort()
    {
        if (ready_)
            throw std::runtime_error("huffman tree already setup");

        std::sort(tree_.begin(), tree_.end(), node_code_less());
        ready_ = true;
    }

    template<class InputBitStream>
    value_type decode(InputBitStream& bs) const
    {
        typedef typename tree_type::const_iterator iter_type;

        BOOST_ASSERT(ready_);
        BOOST_ASSERT(!tree_.empty());

        code_type code = 0;
        std::size_t i = 0;
        iter_type first = tree_.begin();
        iter_type last = tree_.end();
        for ( ; boost::next(first) != last; ++i)
        {
            code <<= 1;
            code |= static_cast<code_type>(bs.get_bit());

            boost::tie(first, last) =
                std::equal_range(
                    first, last,
                    node(code << ((Bits-1)-i), value_type()),
                    node_bit_less(i));

            if (first == last)
                throw std::runtime_error("unknwon huffman code");
        }
        return first->value;
    }

private:
    tree_type tree_;
    bool ready_;
};

template<class Length>
class huffman_code_length_decoder
{
public:
    typedef Length length_type;

    huffman_code_length_decoder() : max_(0), count_(0)
    {
    }

    void reserve(std::size_t n)
    {
        table_.reserve(n);
    }

    void push_back(length_type x)
    {
        table_.push_back(x);

        if (x > max_)
            max_ = x;

        ++count_;
    }

    void skip(std::size_t n)
    {
        table_.resize(table_.size()+n);
    }

    template<class Value, std::size_t Bits>
    void decode(huffman_decoder<Value,Bits>& decoder) const
    {
        typedef typename huffman_decoder<Value,Bits>::code_type code_type;

        decoder.clear();

        // US-CERT Vulnerability Note VU#381508
        if (max_ > static_cast<length_type>(Bits))
            throw std::runtime_error("too long huffman code length found");

        if (table_.empty())
        {
            decoder.assign(Value());
            return;
        }
        else if (count_ == 1)
        {
            decoder.assign(table_.front());
            return;
        }

        // TODO
        BOOST_STATIC_ASSERT(sizeof(std::size_t)*CHAR_BIT > Bits);

        std::vector<std::size_t> count_table(max_+1);
        for (std::size_t i = 0; i < table_.size(); ++i)
        {
            if (length_type len = table_[i])
            {
                // US-CERT Vulnerability Note VU#596848
                if (++count_table[len] > (static_cast<std::size_t>(1) << len))
                    throw std::runtime_error("bad huffman code length table");
            }
        }

        std::vector<code_type> base(max_+1);
        for (std::size_t i = 1; i <= max_; ++i)
        {
            // US-CERT Vulnerability Note VU#596848
            if (std::size_t count = count_table[i])
            {
                std::size_t max_val =
                    (static_cast<std::size_t>(1) << i) - base[i-1];
                if (count > max_val)
                    throw std::runtime_error("bad huffman code length table");
            }

            base[i] = (base[i-1]<<1) + (count_table[i] << 1);
        }

        for (std::size_t i = 0; i < table_.size(); ++i)
        {
            if (length_type len = table_[i])
            {
                decoder.push_back(base[len-1] << (Bits-len), i);
                ++base[len-1];
            }
        }
        decoder.sort();
    }

private:
    std::vector<length_type> table_;
    length_type max_;
    std::size_t count_;
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_UTILITY_HUFFMAN_HPP
