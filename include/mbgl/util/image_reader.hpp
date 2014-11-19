#ifndef MBGL_UTIL_IMAGE_READER_HPP
#define MBGL_UTIL_IMAGE_READER_HPP

#include "noncopyable.hpp"
// stl
#include <stdexcept>
#include <string>

namespace mbgl { namespace util {

class image_reader_exception : public std::exception
{
private:
    std::string message_;
public:
    image_reader_exception(std::string const& message)
        : message_(message) {}

    ~image_reader_exception() throw() {}

    virtual const char* what() const throw()
    {
        return message_.c_str();
    }
};

struct image_reader : private noncopyable
{
    virtual unsigned width() const=0;
    virtual unsigned height() const=0;
    virtual bool has_alpha() const=0;
    virtual bool premultiplied_alpha() const=0;
    virtual void read(unsigned x,unsigned y, unsigned width, unsigned height, char* image)=0;
    virtual ~image_reader() {}
};

image_reader* get_image_reader(char const* data, size_t size);

}}

#endif