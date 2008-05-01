// character_repository.hpp: the repository for character classes

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "character_repository.hpp"
#include <hamigaki/functional.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <algorithm>
#include <fstream>
#include <vector>
#include <stdexcept>

namespace algo = boost::algorithm;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

namespace
{

game_character_class load_character(const std::string& filename)
{
    std::ifstream file(filename.c_str(), std::ios_base::binary);
    io::filtering_stream<io::input> is;
    is.push(io::zlib_decompressor());
    is.push(file);
    boost::archive::binary_iarchive ia(is);
    game_character_class c;
    ia >> c;
    return c;
}

} // namespace

class character_repository::impl
{
private:
    typedef std::vector<game_character_class> table_type;

public:
    impl()
    {
        fs::directory_iterator it((fs::current_path()));
        fs::directory_iterator end;

        table_type tmp;
        std::locale loc("");
        for ( ; it != end; ++it)
        {
            const std::string& leaf = it->path().leaf();
            if (algo::iends_with(leaf, ".agc-yh", loc))
                tmp.push_back(load_character(leaf));
        }

        std::sort(
            tmp.begin(), tmp.end(),
            hamigaki::mem_var_less(&game_character_class::id)
        );

        table_.swap(tmp);
    }

    const game_character_class& operator[](const hamigaki::uuid& id) const
    {
        game_character_class tmp;
        tmp.id = id;

        table_type::const_iterator it =
            std::lower_bound(
                table_.begin(), table_.end(),
                tmp, hamigaki::mem_var_less(&game_character_class::id)
            );

        if ((it == table_.end()) || (it->id != id))
        {
            std::string msg;
            msg += "unknown character class ";
            msg += id.to_string();
            throw std::runtime_error(msg);
        }

        return *it;
    }

private:
    table_type table_;
};

character_repository::character_repository()
    : pimpl_(new impl)
{
}

character_repository::~character_repository()
{
}

const game_character_class&
character_repository::operator[](const hamigaki::uuid& id) const
{
    return (*pimpl_)[id];
}
