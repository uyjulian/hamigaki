// png_reader.cpp: PNG reader class

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <boost/config.hpp>

#include "png_reader.hpp"
#include <istream>
#include <stdexcept>

#include <png.h>
#include <csetjmp>

#if defined(BOOST_WINDOWS) && !defined(NDEBUG)
    #include <windows.h>
#endif

namespace
{

void PNGAPI do_long_jump(png_struct* ctx, const char*)
{
    longjmp(png_jmpbuf(ctx), 1);
}

void PNGAPI process_error(png_struct*, const char* msg)
{
    throw std::runtime_error(msg);
}

#if !defined(BOOST_WINDOWS) || defined(NDEBUG)
void PNGAPI process_warning(png_struct*, const char*)
{
}
#else
void PNGAPI process_warning(png_struct*, const char* msg)
{
    ::OutputDebugStringA(msg);
}
#endif

void PNGAPI read_from_stream(png_struct* ctx, unsigned char* p, std::size_t n)
{
    std::streamsize size = static_cast<std::streamsize>(n);

    std::istream& is = *static_cast<std::istream*>(png_get_io_ptr(ctx));
    is.read(reinterpret_cast<char*>(p), size);
    if (is.gcount() != size)
        throw std::runtime_error("failed read_from_stream()");
}

} // namespace

namespace hamigaki
{

class png_reader::impl
{
public:
    explicit impl(std::istream& is)
    {
        // Note:
        // png_create_read_struct() uses setjmp() for error handling
        // So error_fn must call longjmp() in this function
        pimpl_ = ::png_create_read_struct(
            PNG_LIBPNG_VER_STRING, 0, &do_long_jump, &process_warning);

        if (!pimpl_)
            throw std::runtime_error("failed png_create_read_struct()");

        // Now, error_fn can throw a exception
        ::png_set_error_fn(pimpl_, 0, &process_error, &process_warning);

        info_ptr_ = ::png_create_info_struct(pimpl_);
        if (!info_ptr_)
        {
            ::png_destroy_read_struct(&pimpl_, 0, 0);
            throw std::runtime_error("failed png_create_info_struct()");
        }

        ::png_set_read_fn(pimpl_, &is, read_from_stream);
    }

    ~impl()
    {
        ::png_destroy_read_struct(&pimpl_, &info_ptr_, 0);
    }

    void read_info()
    {
        ::png_read_info(pimpl_, info_ptr_);
    }

    void force_rgba()
    {
        unsigned long w;
        unsigned long h;
        int bits;
        int type;

        ::png_get_IHDR(pimpl_, info_ptr_, &w, &h, &bits, &type, 0, 0, 0);

        ::png_set_strip_16(pimpl_);
        ::png_set_packing(pimpl_);

        if (type == PNG_COLOR_TYPE_PALETTE)
            ::png_set_palette_to_rgb(pimpl_);

        if (((type & PNG_COLOR_MASK_COLOR) == 0) && (bits < 8))
            ::png_set_gray_1_2_4_to_8(pimpl_);

        if (png_get_valid(pimpl_, info_ptr_, PNG_INFO_tRNS))
            ::png_set_tRNS_to_alpha(pimpl_);
        else if ((type & PNG_COLOR_MASK_ALPHA) == 0)
            ::png_set_add_alpha(pimpl_, 0xFF, PNG_FILLER_AFTER);

        ::png_read_update_info(pimpl_, info_ptr_);
    }

    unsigned long width() const
    {
        return ::png_get_image_width(pimpl_, info_ptr_);
    }

    unsigned long height() const
    {
        return ::png_get_image_height(pimpl_, info_ptr_);
    }

    unsigned long pitch() const
    {
        return ::png_get_rowbytes(pimpl_, info_ptr_);
    }

    void read_next_row(unsigned char* buf)
    {
        ::png_read_row(pimpl_, buf, 0);
    }

    void read_end()
    {
        ::png_read_end(pimpl_, 0);
    }

private:
    png_struct* pimpl_;
    png_info* info_ptr_;

    impl(const impl&);
    impl& operator=(const impl&);
};

png_reader::png_reader(std::istream& is) : pimpl_(new impl(is))
{
    pimpl_->read_info();
    pimpl_->force_rgba();
}

png_reader::~png_reader()
{
}

unsigned long png_reader::width() const
{
    return pimpl_->width();
}

unsigned long png_reader::height() const
{
    return pimpl_->height();
}

unsigned long png_reader::pitch() const
{
    return pimpl_->pitch();
}

void png_reader::read_next_row(unsigned char* buf)
{
    pimpl_->read_next_row(buf);
}

void png_reader::read_end()
{
    pimpl_->read_end();
}

} // namespace hamigaki
