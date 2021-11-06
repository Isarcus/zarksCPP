#pragma once

#include <cstdint>
#include <cstdio>

namespace zmath
{
    // BitField object. Represents a set of `size` bits.
    typedef struct BitField
    {
        // Create a new BitField.
        // @param value the value of this bitField. If `value` is greater
        //        than 2^(size-1), this function will throw std::runtime_error.
        // @param size the number of bits this BitField should take up.
        //        if `size` is greater than sizeof(size_t), this function
        //        will throw std::runtime_error.
        BitField(size_t value, uint8_t size);

        size_t value;
        uint8_t size;
    } BitField;

    class BitBuffer
    {
    private:
        // Forward declaration of BitPointer
        struct BitPointer;
        struct BitAddress;

    public:
        // Instantiate an empty BitBuffer.
        BitBuffer();
        // Copy constructor
        BitBuffer(const BitBuffer& bbuf);
        // Move constructor
        BitBuffer(BitBuffer&& bbuf);
        // Instantiate an empty BitBuffer.
        // @param bitCapacity the minimum number of bits to reserve space for.
        BitBuffer(size_t bitCapacity);
        // Instantiate a BitBuffer with data in it
        BitBuffer(const void* bytes, size_t sizeBytes);
        // Instantiate a BitBuffer with data in it
        BitBuffer(const void* bytes, const BitAddress& size);

        ~BitBuffer();

        // Copy assignment
        BitBuffer& operator=(const BitBuffer& bbuf);
        // Move assignment
        BitBuffer& operator=(BitBuffer&& bbuf);

        // Append a BitField to the BitBuffer.
        friend BitBuffer& operator<<(BitBuffer& bbuf, const BitField& bits);

        // Read a sequence of characters into the BitBuffer.
        // @param c_str a c-style (null-terminated) string.
        friend BitBuffer& operator<<(BitBuffer& bbuf, const char* c_str);

        // Appends a bit to the end of the BitBuffer
        void Push(bool bit);
        // Shrinks the BitBuffer at the end by one bit
        void Pop();

        // Access a bit, with bound-checking
        BitPointer At(size_t bitNum);
        // Access a bit, with bound-checking
        BitPointer At(BitAddress addr);
        // Access a bit, with bound-checking
        bool At(size_t bitNum) const;
        // Access a bit, with bound-checking
        bool At(BitAddress addr) const;

        // Access a bit, without bound-checking
        BitPointer operator[](size_t bitNum);
        // Access a bit, without bound-checking
        BitPointer operator[](BitAddress addr);
        // Access a bit, without bound-checking
        bool operator[](size_t bitNum) const;
        // Access a bit, without bound-checking
        bool operator[](BitAddress addr) const;

        // @return a pointer to the internal data array.
        uint8_t* Data();
        // @return a const pointer to the internal data array.
        const uint8_t* Data() const;

        // Read an unsigned value of several bits
        // @param startBit the bit at which to begin reading.
        // @param readBits the number of bits to read. This function will
        //        throw std::runtime_error if this value is greater than
        //        BITS_IN_SIZE.
        // @param leastToGreatest true if the least significant bit comes
        //        first, false if the most significant but comes first.
        size_t Read(size_t startBit, uint8_t readBits, bool leastToGreatest = true) const;
        // Read an unsigned value of several bits
        // @param start the address at which to begin reading.
        // @param readBits the number of bits to read. This function will
        //        throw std::runtime_error if this value is greater than
        //        BITS_IN_SIZE.
        // @param leastToGreatest true if the least significant bit comes
        //        first, false if the most significant but comes first.
        size_t Read(BitAddress start, uint8_t readBits, bool leastToGreatest = true) const;

        // @return the current size of the buffer in bits.
        size_t Size() const;
        // @return the current size of the buffer in bytes, rounded up.
        size_t SizeBytes() const;
        // @return the current capacity of the buffer in bits.
        size_t Capacity() const;
        // @return the current capacity of the buffer in bytes.
        size_t CapacityBytes() const;
        // @return whether the underlying array is currently utilizing
        //         the entire capacity
        bool Full() const;
        // @return true if underlying array is empty
        bool Empty() const;

        // Reserve at least `bits` bits' worth of space.
        void Reserve(size_t bits);
        // Reserve at least `bytes` bytes` worth of space.
        void ReserveBytes(size_t bytes);

        // Clear the bit buffer.
        void Clear();

        // BitAddress represents the location of a single bit in a BitBuffer.
        typedef struct BitAddress
        {
            // @param globalBitIdx the global bit that this address
            //        should refer to.
            BitAddress(size_t globalBitIdx = 0);

            // @param byteIdx the index of the byte that the desired
            //        bit is a part of.
            // @param internalBitIdx the bit of the byte in question.
            //        Any value not in the range [0, 7] will cause
            //        this constructor to throw std::runtime_error.
            BitAddress(size_t byteIdx, uint8_t internalBitIdx);

            bool operator<(const BitAddress& addr) const;
            bool operator>(const BitAddress& addr) const;
            bool operator==(const BitAddress& addr) const;
            bool operator!=(const BitAddress& addr) const;

            BitAddress& operator++();
            BitAddress operator++(int);
            BitAddress& operator--();
            BitAddress operator--(int);

            // @return The global index of this BitAddress's bit.
            //         Equivalent to 8*byteIdx + bitIdx.
            size_t Bit() const;

            // @return the minimum of bytes required to store an array
            //         containing this address. Equivalent to byteIdx +
            //         (bitIdx ? 1 : 0)
            size_t Bytes() const;

            size_t byteIdx;
            unsigned bitIdx : 3;
        } BitAddress;

    private:
        size_t capacityBytes;
        uint8_t* data;
        BitAddress next;

        // Reallocates underlying array to be twice its current size
        void reallocArray();

        // BitPointer is a means by which to get or set the value of one
        // bit in a BitBuffer.
        class BitPointer
        {
        public:
            BitPointer(uint8_t* ptr, BitAddress addr);

            BitPointer &operator=(bool val);
            operator bool() const;

        private:
            uint8_t* bytePtr;
            unsigned bitIdx : 3;
        };

        static constexpr size_t DEFAULT_BYTE_CAPACITY = 8;
    };

} // namespace zmath
