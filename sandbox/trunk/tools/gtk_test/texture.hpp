// texture.hpp: the OpenGL texture class

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HAMIGAKI_TEXTURE_HPP
#define HAMIGAKI_TEXTURE_HPP

namespace hamigaki
{

class render_context;

class texture
{
public:
    explicit texture(render_context& rc);
    ~texture();
    void set_image(int width, int height, void* data);
    void bind();

private:
    unsigned handle_;

    texture(const texture&);
    texture& operator=(const texture&);
};

} // namespace hamigaki

#endif // HAMIGAKI_TEXTURE_HPP
