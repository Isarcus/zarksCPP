#include "pch.h"
#include "binary.h"

void zmath::ToBytes(uint8_t* buf, uint16_t val, Endian byteOrder)
{
	if (byteOrder == Endian::Little)
	{
		buf[0] = val;
		buf[1] = val >> 8;
	}
	else
	{
		buf[0] = val >> 8;
		buf[1] = val;
	}
}

void zmath::ToBytes(uint8_t* buf, uint32_t val, Endian byteOrder)
{
	if (byteOrder == Endian::Little)
	{
		buf[0] = val;
		buf[1] = val >> 8;
		buf[2] = val >> 16;
		buf[3] = val >> 24;
	}
	else
	{
		buf[0] = val >> 24;
		buf[1] = val >> 16;
		buf[2] = val >> 8;
		buf[3] = val;
	}
}

void zmath::ToBytes(uint8_t* buf, uint64_t val, Endian byteOrder)
{
	if (byteOrder == Endian::Little)
	{
		buf[0] = val;
		buf[1] = val >> 8;
		buf[2] = val >> 16;
		buf[3] = val >> 24;
		buf[4] = val >> 32;
		buf[5] = val >> 40;
		buf[6] = val >> 48;
		buf[7] = val >> 56;
	}
	else
	{
		buf[0] = val >> 56;
		buf[1] = val >> 48;
		buf[2] = val >> 40;
		buf[3] = val >> 32;
		buf[0] = val >> 24;
		buf[1] = val >> 16;
		buf[2] = val >> 8;
		buf[3] = val;
	}
}

void zmath::ToBytes(uint8_t* buf, float val, Endian byteOrder)
{
	uint32_t* uptr = reinterpret_cast<uint32_t*>(&val);
	uint32_t uval = *uptr;

	ToBytes(buf, uval, byteOrder);
}

void zmath::ToBytes(uint8_t* buf, double val, Endian byteOrder)
{
	uint64_t* uptr = reinterpret_cast<uint64_t*>(&val);
	uint64_t uval = *uptr;

	ToBytes(buf, uval, byteOrder);
}
