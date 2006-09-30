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
        value_type next[2];

        node()
        {
            next[0] = value_type();
            next[1] = value_type();
        }
    };

    typedef std::vector<node> tree_type;

public:
    huffman_decoder()
    {
        tree_.push_back(node());
    }

    void clear()
    {
        tree_.clear();
        tree_.push_back(node());
    }

    void reserve(std::size_t n)
    {
        tree_.reserve(n);
    }

    void assign(const value_type& x)
    {
        tree_.clear();
        tree_.push_back(node());
        tree_.back().next[0] = x;
    }

    void push_back(code_type code, std::size_t bits, const value_type& value)
    {
        value_type pos = 0;
        std::size_t bits1 = bits-1;
        for (std::size_t i = 0; i < bits; ++i)
        {
            bool b = code & (1 << (bits1-i));
            value_type next = tree_[pos].next[b];
            if (!next)
            {
                if (tree_.size() >= (std::numeric_limits<value_type>::max)())
                    throw std::runtime_error("huffman tree overflow");
                next = static_cast<value_type>(tree_.size());
                tree_[pos].next[b] = next;
                tree_.push_back(node());
            }
            pos = next;
        }
        tree_[pos].next[0] = value;
    }

    template<class InputBitStream>
    value_type decode(InputBitStream& bs) const
    {
        value_type pos = 0;
        while (tree_[pos].next[1])
            pos = tree_[pos].next[bs.get_bit()];
        return tree_[pos].next[0];
    }

private:
    tree_type tree_;
};

template<class Length>
class huffman_code_length_decoder
{
public:
    typedef Length length_type;

    huffman_code_length_decoder() : max_(0), count_(0)
    {
    }

    void clear()
    {
        table_.clear();
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
        decoder.reserve(count_);

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

            base[i] = (base[i-1] + count_table[i]) << 1;
        }

        for (std::size_t i = 0; i < table_.size(); ++i)
        {
            if (length_type len = table_[i])
            {
                decoder.push_back(base[len-1], len, i);
                ++base[len-1];
            }
        }
    }

private:
    std::vector<length_type> table_;
    length_type max_;
    std::size_t count_;
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_UTILITY_HUFFMAN_HPP
