// texture.cpp: the OpenGL texture class

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <boost/config.hpp>

#include "texture.hpp"
#include "render_context.hpp"
#include <stdexcept>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#endif
#include <GL/gl.h>

namespace hamigaki
{

texture::texture(render_context& rc) : handle_(0)
{
    rc.select();
    ::glGenTextures(1, &handle_);
    if (::glGetError() != GL_NO_ERROR)
        throw std::runtime_error("glGenTextures() failed");
}

texture::~texture()
{
    ::glDeleteTextures(1, &handle_);
}

void texture::set_image(int width, int height, void* data)
{
    ::glBindTexture(GL_TEXTURE_2D, handle_);
    ::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    ::glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data);
    ::glBindTexture(GL_TEXTURE_2D, 0);
}

void texture::bind()
{
    ::glBindTexture(GL_TEXTURE_2D, handle_);
}

} // namespace hamigaki
