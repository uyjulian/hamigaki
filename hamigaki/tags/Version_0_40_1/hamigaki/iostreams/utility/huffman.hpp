// huffman.hpp: Huffman encoding/decoding

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_UTILITY_HUFFMAN_HPP
#define HAMIGAKI_IOSTREAMS_UTILITY_HUFFMAN_HPP

#include <boost/tuple/tuple.hpp>
#include <boost/assert.hpp>
#include <boost/integer.hpp>
#include <boost/next_prior.hpp>
#include <boost/static_assert.hpp>
#include <algorithm>
#include <queue>
#include <stdexcept>
#include <vector>

namespace hamigaki { namespace iostreams {

template<class Value, std::size_t Bits>
class huffman_encoder
{
public:
    typedef typename boost::uint_t<Bits>::least code_type;
    typedef Value value_type;

    struct node
    {
        code_type code;
        std::size_t bits;

        node() : code(0), bits(0)
        {
        }

        node(code_type code, std::size_t bits) : code(code), bits(bits)
        {
        }
    };

    typedef typename std::vector<node>::const_iterator const_iterator;

    void clear()
    {
        table_.clear();
        default_value_ = value_type();
    }

    void reserve(std::size_t n)
    {
        table_.reserve(n);
    }

    void insert(const value_type& value, code_type code, std::size_t bits)
    {
        if (static_cast<std::size_t>(value) >= table_.size())
            table_.resize(value+1);
        table_[value] = node(code, bits);
    }

    template<class OutputBitStream>
    void encode(OutputBitStream& bs, const value_type& value) const
    {
        if (table_.empty())
            return;

        if (static_cast<std::size_t>(value) >= table_.size())
            throw std::runtime_error("unknwon huffman value");

        const node& x = table_[value];
        if (x.bits == 0)
            throw std::runtime_error("unknwon huffman value");

        bs.write_bits(x.code, x.bits);
    }

    bool empty() const
    {
        return table_.empty();
    }

    std::size_t size() const
    {
        return table_.size();
    }

    const node& operator[](std::size_t index) const
    {
        return table_[index];
    }

    const_iterator begin() const
    {
        return table_.begin();
    }

    const_iterator end() const
    {
        return table_.end();
    }

    void default_value(const value_type& x)
    {
        default_value_ = x;
    }

    value_type default_value() const
    {
        return default_value_;
    }

private:
    std::vector<node> table_;
    value_type default_value_;
};

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

    void insert(code_type code, std::size_t bits, const value_type& value)
    {
        value_type pos = 0;
        std::size_t bits1 = bits-1;
        for (std::size_t i = 0; i < bits; ++i)
        {
            bool b = (code & (1 << (bits1-i))) != 0;
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

        std::size_t count_table[Bits+1];
        std::fill_n(&count_table[0], Bits+1, 0);
        for (std::size_t i = 0; i < table_.size(); ++i)
        {
            if (length_type len = table_[i])
            {
                // US-CERT Vulnerability Note VU#596848
                if (++count_table[len] > (static_cast<std::size_t>(1) << len))
                    throw std::runtime_error("bad huffman code length table");
            }
        }

        std::size_t base[Bits+1];
        std::fill_n(&base[0], Bits+1, 0);
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
                decoder.insert(base[len-1], len, i);
                ++base[len-1];
            }
        }
    }

private:
    std::vector<length_type> table_;
    length_type max_;
    std::size_t count_;
};


template<class Value>
class huffman
{
public:
    typedef Value value_type;

private:
    struct node
    {
        bool is_leaf;
        union
        {
            value_type value;
            std::size_t index[2];
        };
        std::size_t count;
        std::size_t depth;

        node(value_type x, std::size_t count)
            : is_leaf(true), count(count)
        {
            value = x;
        }

        node(std::size_t index1, std::size_t index2, std::size_t count)
            : is_leaf(false), count(count)
        {
            index[0] = index1;
            index[1] = index2;
        }

        static void set_depth(
            std::vector<node>& tree, std::size_t index, std::size_t depth)
        {
            node& cur = tree[index];
            cur.depth = depth;

            if (!cur.is_leaf)
            {
                ++depth;

                for (std::size_t i = 0; i < 2; ++i)
                    set_depth(tree, cur.index[i], depth);
            }
        }
    };

    struct value_length
    {
        value_type value;
        std::size_t length;

        explicit value_length(value_type value) : value(value)
        {
        }

        bool operator<(const value_length& rhs) const
        {
            if (length == rhs.length)
                return value < rhs.value;
            else
                return length < rhs.length;
        }
    };

    class node_count_greator
    {
    public:
        explicit node_count_greator(const std::vector<node>& tree) : tree_(tree)
        {
        }

        bool operator()(std::size_t lhs, std::size_t rhs) const
        {
            return tree_[lhs].count > tree_[rhs].count;
        }

    private:
        const std::vector<node>& tree_;
    };

    class table_count_greator
    {
    public:
        explicit table_count_greator(const std::vector<std::size_t>& table)
            : table_(table)
        {
        }

        bool operator()(const value_length& lhs, const value_length& rhs) const
        {
            if (table_[lhs.value] == table_[rhs.value])
                return lhs.value < rhs.value;
            else
                return table_[lhs.value] > table_[rhs.value];
        }

    private:
        const std::vector<std::size_t>& table_;
    };

public:
    void clear()
    {
        table_.clear();
    }

    void reserve(std::size_t n)
    {
        table_.reserve(n);
    }

    void insert(const value_type& value)
    {
        if (static_cast<std::size_t>(value) >= table_.size())
            table_.resize(value+1);
        ++(table_[value]);
    }

    template<std::size_t Bits>
    void make_encoder(huffman_encoder<Value,Bits>& encoder) const
    {
        encoder.clear();
        encoder.reserve(table_.size());

        std::vector<node> tree;
        std::vector<value_length> order;

        for (std::size_t i = 0; i < table_.size(); ++i)
        {
            if (table_[i])
            {
                tree.push_back(node(i, table_[i]));
                order.push_back(value_length(i));
            }
        }

        if (tree.empty())
            return;
        else if (tree.size() == 1)
        {
            encoder.default_value(tree.front().value);
            return;
        }

        typedef std::priority_queue<
            std::size_t,
            std::vector<std::size_t>,
            node_count_greator
        > queue_type;

        queue_type queue((node_count_greator(tree)));

        const std::size_t count = tree.size();
        for (std::size_t i = 0; i < count; ++i)
            queue.push(i);

        while (true)
        {
            std::size_t a = queue.top();
            queue.pop();
            std::size_t b = queue.top();
            queue.pop();

            tree.push_back(node(a, b, tree[a].count + tree[b].count));
            if (queue.empty())
                break;
            queue.push(tree.size()-1);
        }

        node::set_depth(tree, tree.size()-1, 0);

        std::size_t length_count[Bits+1];
        std::fill_n(length_count, Bits+1, 0);
        int over = 0;
        for (std::size_t i = 0; i < count; ++i)
        {
            std::size_t depth = tree[i].depth;
            if (depth <= Bits)
                ++length_count[depth];
            else
                ++over;
        }

        while (over-- > 0)
        {
            for (std::size_t i = Bits-1; i > 0; --i)
            {
                if (length_count[i])
                {
                    --length_count[i];
                    length_count[i+1] += 2;
                }
            }
        }

        std::size_t base[Bits+1];
        std::fill_n(&base[0], Bits+1, 0);
        for (std::size_t i = 1; i <= Bits; ++i)
            base[i] = (base[i-1] + length_count[i]) << 1;

        std::sort(order.begin(), order.end(), table_count_greator(table_));
        std::size_t index = 0;
        for (std::size_t i = 1; i <= Bits; ++i)
        {
            std::size_t count = length_count[i];
            for (std::size_t j = 0; j < count; ++j)
                order[index++].length = i;
        }
        std::sort(order.begin(), order.end());

        index = 0;
        for (std::size_t i = 1; i <= Bits; ++i)
        {
            std::size_t count = length_count[i];
            for (std::size_t j = 0; j < count; ++j)
            {
                encoder.insert(order[index++].value, base[i-1], i);
                ++base[i-1];
            }
        }
    }

private:
    std::vector<std::size_t> table_;
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_UTILITY_HUFFMAN_HPP
