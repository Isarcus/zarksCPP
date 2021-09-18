#include <zarks/io/BitBuffer.h>
#include <zarks/io/bitdefs.h>

#include <cmath>
#include <stdexcept>
#include <cstring>

namespace zmath
{

//                 //
// BitField Struct //
//                 //

BitField::BitField(size_t value, uint8_t size)
    : value(value)
    , size(size)
{
    if (size > 8 * sizeof(size_t)) {
        throw std::runtime_error("`size` in BitField is beyond maximum allowable value!");
    } else if (value > std::pow(2, size) - 1) {
        throw std::runtime_error("Invalid size-value pair in BitField");
    }
}

//                 //
// BitBuffer Class //
//                 //

BitBuffer::BitBuffer()
    : BitBuffer(8 * DEFAULT_BYTE_CAPACITY)
{}

BitBuffer::BitBuffer(const BitBuffer& bbuf)
    : data(nullptr)
{
    *this = bbuf;
}

BitBuffer::BitBuffer(BitBuffer&& bbuf)
{
    *this = std::move(bbuf);
}

BitBuffer::BitBuffer(size_t bitCapacity)
    : capacityBytes(BitAddress(bitCapacity).Bytes())
    , data(new uint8_t[capacityBytes])
{}

BitBuffer::BitBuffer(const void* bytes, size_t sizeBytes)
    : capacityBytes(sizeBytes)
    , data(new uint8_t[capacityBytes])
    , next(sizeBytes)
{
    memcpy(data, bytes, sizeBytes);
}

BitBuffer::BitBuffer(const void* bytes, const BitAddress& size)
    : capacityBytes(size.Bytes())
    , data(new uint8_t[capacityBytes])
    , next(size)
{
    memcpy(data, bytes, size.Bytes());
}

BitBuffer::~BitBuffer()
{
    delete[] data;
}

BitBuffer& BitBuffer::operator=(const BitBuffer& bbuf)
{
    if (this != &bbuf)
    {
        delete[] data;
        capacityBytes = bbuf.SizeBytes();
        data = new uint8_t[capacityBytes];
        memcpy(data, bbuf.data, capacityBytes);
        next = bbuf.next;
    }

    return *this;
}

BitBuffer& BitBuffer::operator=(BitBuffer&& bbuf)
{
    if (this != &bbuf)
    {
        delete[] data;
        data = bbuf.data;
        capacityBytes = bbuf.capacityBytes;
        next = bbuf.next;

        bbuf.data = nullptr;
        bbuf.next = 0;
        bbuf.capacityBytes = 0;
    }

    return *this;
}

BitBuffer& operator<<(BitBuffer& bbuf, const BitField& bits)
{
    for (unsigned i = 0; i < bits.size; i++)
    {
        bbuf.Push(bits.value & BITS[i]);
    }

    return bbuf;
}

BitBuffer& operator<<(BitBuffer& bbuf, const char* c_str)
{
    const uint8_t* ptr = (const uint8_t*)c_str;
    for (; *ptr; ptr++)
    {
        bbuf << BitField(*ptr, 8);
    }

    return bbuf;
}

void BitBuffer::Push(bool bit)
{
    if (Full())
    {
        reallocArray();
    }

    At(next++) = bit;
}

void BitBuffer::Pop()
{
    if (Empty()) {
        throw std::runtime_error("Tried to Pop() from empty BitBuffer");
    } else {
        next--;
    }
}

BitBuffer::BitPointer BitBuffer::At(size_t bitNum)
{
    if (bitNum < next.Bit()) {
        return BitPointer(data, BitAddress(bitNum));
    } else {
        throw std::runtime_error("Out of bounds BitBuffer access");
    }
}

BitBuffer::BitPointer BitBuffer::At(BitAddress addr)
{
    if (addr < next) {
        return BitPointer(data, addr);
    } else {
        throw std::runtime_error("Out of bounds BitBuffer access");
    }
}

bool BitBuffer::At(size_t bitNum) const
{
    if (bitNum < next.Bit()) {
        return data[bitNum / 8] & BITS[bitNum % 8];
    } else {
        throw std::runtime_error("Out of bounds BitBuffer access");
    }
}

bool BitBuffer::At(BitAddress addr) const
{
    if (addr < next) {
        return data[addr.byteIdx] & BITS[addr.bitIdx];
    } else {
        throw std::runtime_error("Out of bounds BitBuffer access");
    }
}

BitBuffer::BitPointer BitBuffer::operator[](size_t bitNum)
{
    return BitPointer(data, BitAddress(bitNum));
}

BitBuffer::BitPointer BitBuffer::operator[](BitAddress addr)
{
    return BitPointer(data, addr);
}

bool BitBuffer::operator[](size_t bitNum) const
{
    return data[bitNum / 8] & BITS[bitNum % 8];
}

bool BitBuffer::operator[](BitAddress addr) const
{
    return data[addr.byteIdx] & BITS[addr.bitIdx];
}

size_t BitBuffer::Size() const
{
    return next.Bit();
}

size_t BitBuffer::SizeBytes() const
{
    return next.Bytes();
}

bool BitBuffer::Full() const
{
    return next.Bytes() == capacityBytes &&
           next.bitIdx == 7;
}

bool BitBuffer::Empty() const
{
    return next == BitAddress(0);
}

void BitBuffer::Reserve(size_t bits)
{
    Reserve(BitAddress(bits).Bytes());
}

void BitBuffer::ReserveBytes(size_t bytes)
{
    if (bytes > capacityBytes)
    {
        // Allocate new array and copy relevant info over to it
        uint8_t* newArr = new uint8_t[bytes];
        memcpy(newArr, data, next.Bytes());

        // Delete old array
        delete[] data;

        // Point data to new array and update capacity
        data = newArr;
        capacityBytes = bytes;
    }
}

void BitBuffer::Clear()
{
    next = BitAddress(0);
}

void BitBuffer::reallocArray()
{
    uint8_t* newData = new uint8_t[capacityBytes * 2];
    memcpy(newData, data, capacityBytes);
    delete[] data;
    data = newData;
    capacityBytes *= 2;
}

//                              //
// BitBuffer::BitAddress Struct //
//                              //

BitBuffer::BitAddress::BitAddress(size_t globalBitIdx)
    : byteIdx(globalBitIdx / 8)
    , bitIdx(globalBitIdx - byteIdx * 8)
{}

BitBuffer::BitAddress::BitAddress(size_t byteIdx, uint8_t internalBitIdx)
    : byteIdx(byteIdx)
    , bitIdx(internalBitIdx)
{}

bool BitBuffer::BitAddress::operator<(const BitAddress& addr) const
{
    return (byteIdx < addr.byteIdx) ||
           (byteIdx == addr.byteIdx && bitIdx < addr.bitIdx);
}

bool BitBuffer::BitAddress::operator>(const BitAddress& addr) const
{
    return (byteIdx > addr.byteIdx) ||
           (byteIdx == addr.byteIdx && bitIdx > addr.bitIdx);
}

bool BitBuffer::BitAddress::operator==(const BitAddress& addr) const
{
    return byteIdx == addr.byteIdx && bitIdx == addr.bitIdx;
}

bool BitBuffer::BitAddress::operator!=(const BitAddress& addr) const
{
    return byteIdx != addr.byteIdx || bitIdx != addr.bitIdx;
}

BitBuffer::BitAddress& BitBuffer::BitAddress::operator++()
{
    if (bitIdx++ == 7)
    {
        byteIdx++;
        bitIdx = 0;
    }

    return *this;
}

BitBuffer::BitAddress BitBuffer::BitAddress::operator++(int)
{
    BitAddress copy = *this;
    ++*this;
    return copy;
}

BitBuffer::BitAddress& BitBuffer::BitAddress::operator--()
{
    if (bitIdx-- == 0)
    {
        byteIdx--;
        bitIdx = 7;
    }

    return *this;
}

BitBuffer::BitAddress BitBuffer::BitAddress::operator--(int)
{
    BitAddress copy = *this;
    --*this;
    return copy;
}

size_t BitBuffer::BitAddress::Bit() const
{
    return 8 * byteIdx + bitIdx;
}

size_t BitBuffer::BitAddress::Bytes() const
{
    return byteIdx + ((bitIdx) ? 1 : 0);
}

//                             //
// BitBuffer::BitPointer Class //
//                             //

BitBuffer::BitPointer::BitPointer(uint8_t* ptr, BitAddress addr)
    : bytePtr(ptr + addr.byteIdx)
    , bitIdx(addr.bitIdx)
{}

BitBuffer::BitPointer& BitBuffer::BitPointer::operator=(bool val)
{
    if (val) {
        *bytePtr |= BITS[bitIdx];
    } else {
        *bytePtr &= ~BITS[bitIdx];
    }

    return *this;
}

BitBuffer::BitPointer::operator bool() const
{
    return *bytePtr & BITS[bitIdx];
}

} // namespace zmath
