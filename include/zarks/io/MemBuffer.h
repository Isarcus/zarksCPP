#include <iostream>
#include <cstdint>

namespace zmath
{
    class MemBuffer
    {
    public:
        MemBuffer(const uint8_t* buf, size_t bytes);
        MemBuffer(const char* buf, size_t bytes);
        MemBuffer(std::istream& buf, size_t bytes);
        MemBuffer(const MemBuffer& mbuf);
        MemBuffer(MemBuffer&& mbuf);
        ~MemBuffer();

        MemBuffer& operator=(const MemBuffer& mbuf);
        MemBuffer& operator=(MemBuffer&& mbuf);

        // Read `n` bytes into `buf`.
        // @return the number of bytes successfully read. This will always be
        //         equal to `n` unless the end of the stream was surpassed, in
        //         which case only the remaining unread bytes of the stream
        //         will be read.
        size_t read(uint8_t* buf, size_t n);
        size_t read(char* buf, size_t n);

        // @return the current stream position.
        size_t pos() const;

        // @return the total number of bytes in the stream.
        size_t size() const;

        // @return the number of bytes left to read in the stream.
        size_t left() const;

        // @return The next byte in the stream. This function throws
        //         std::runtime_error if the end of the stream has been
        //         reached.
        uint8_t peek() const;

        // @return Whether the stream has no bytes left to read. Equivalent to
        //         calling `left() == 0`
        bool done() const;

        // @param n how many bytes to shift the current stream position by.
        // @param forwards moves stream position forwards if true, backwards
        //        if false.
        // @return The actual number of bytes the stream position was shifted
        //         by. This will always be equal to `n` unless
        //         `forwards && n > left()` or `!forwards && n > pos()`
        size_t skip(size_t n, bool forwards);

        // @param n the absolute stream position to move to.
        // @return The actual stream position after calling. This will always
        //         be equal to `n` unless `n > size()`.
        size_t skipto(size_t n);

        // Move the stream position to the very beginning. 
        // Equivalent to calling `skipto(0)`;
        void reset();

    private:
        uint8_t* data;
        const uint8_t* end;
        const uint8_t* current;
        // Invariant: data <= current <= end
    };
} // namespace zmath
