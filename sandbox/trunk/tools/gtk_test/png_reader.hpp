// png_reader.hpp: PNG reader class

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HAMIGAKI_PNG_IMAGE_HPP
#define HAMIGAKI_PNG_IMAGE_HPP

#include <boost/shared_ptr.hpp>
#include <iosfwd>

namespace hamigaki
{

class png_reader
{
public:
    explicit png_reader(std::istream& is);
    ~png_reader();
    unsigned long width() const;
    unsigned long height() const;
    unsigned long pitch() const;
    void read_next_row(unsigned char* buf);
    void read_end();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

} // namespace hamigaki

#endif // HAMIGAKI_PNG_IMAGE_HPP
