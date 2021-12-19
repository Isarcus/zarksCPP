#include <zarks/io/MemBuffer.h>
#include <zarks/io/binary.h>

namespace zmath
{

MemBuffer::MemBuffer(const char* buf, size_t bytes)
    : MemBuffer(reinterpret_cast<const uint8_t*>(buf), bytes)
{}

MemBuffer::MemBuffer(const uint8_t* buf, size_t bytes)
    : data(bufcpy(buf, bytes))
    , end(data + bytes)
    , current(data)
{}

MemBuffer::MemBuffer(std::istream& buf, size_t bytes)
    : data(new uint8_t[bytes])
    , end(data + bytes)
    , current(data)
{
    buf.read((char*)data, bytes);
}

MemBuffer::MemBuffer(const MemBuffer& mbuf)
    : MemBuffer(mbuf.data, mbuf.size())
{}

MemBuffer::MemBuffer(MemBuffer&& mbuf)
    : data(mbuf.data)
    , end(mbuf.end)
    , current(mbuf.current)
{
    mbuf.data = nullptr;
    mbuf.end = nullptr;
    mbuf.current = nullptr;
}

MemBuffer::~MemBuffer()
{
    delete[] data;
    data = nullptr;
    end = nullptr;
    current = nullptr;
}

MemBuffer& MemBuffer::operator=(const MemBuffer& mbuf)
{
    if (this != &mbuf)
    {
        delete[] data;
        data = bufcpy(mbuf.data, mbuf.size());
        end = data + mbuf.size();
        current = mbuf.current;
    }

    return *this;
}

MemBuffer& MemBuffer::operator=(MemBuffer&& mbuf)
{
    if (this != &mbuf)
    {
        delete[] data;
        data = mbuf.data;
        end = mbuf.end;
        current = mbuf.current;

        mbuf.data = nullptr;
        mbuf.end = nullptr;
        mbuf.current = nullptr;
    }

    return *this;
}

size_t MemBuffer::read(uint8_t* buf, size_t n)
{
    n = std::min(n, left());
    memcpy(buf, current, n);
    current += n;

    return n;
}

size_t MemBuffer::read(char* buf, size_t n)
{
    n = std::min(n, left());
    memcpy(buf, current, n);
    current += n;

    return n;
}


size_t MemBuffer::pos() const
{
    return current - data;
}

size_t MemBuffer::size() const
{
    return end - data;
}

size_t MemBuffer::left() const
{
    return end - current;
}

uint8_t MemBuffer::peek() const
{
    if (done())
    {
        throw std::runtime_error("Tried to peek() from a fully read MemBuffer");
    }
    else
    {
        return *current;
    }
}

bool MemBuffer::done() const
{
    return current == end;
}

size_t MemBuffer::skip(size_t n, bool forwards)
{
    if (forwards)
    {
        n = std::min(n, left());
        current += n;
    }
    else
    {
        n = std::min(n, pos());
        current -= n;
    }

    return n;
}

size_t MemBuffer::skipto(size_t n)
{
    n = std::min(n, size());
    current = data + n;

    return n;
}

void MemBuffer::reset()
{
    current = data;
}

} // namespace zmath
