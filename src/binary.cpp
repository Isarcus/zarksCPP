#include <zarks/math/binary.h>

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
		buf[4] = val >> 24;
		buf[5] = val >> 16;
		buf[6] = val >> 8;
		buf[7] = val;
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

uint16_t zmath::ToU16(char* buf, Endian byteOrder)
{
	uint16_t val = 0;

	uint8_t* ubuf = (uint8_t*)buf;

	if (byteOrder == Endian::Little)
	{
		for (int i = 0; i < 2; i++) val += (1 << (i*8)) * ubuf[i];
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			uint64_t placeVal = 1;
			placeVal <<= (8 - 8*i);

			val += placeVal * ubuf[i];
		}
	}

	return val;
}

uint32_t zmath::ToU32(char* buf, Endian byteOrder)
{
	uint32_t val = 0;

	uint8_t* ubuf = (uint8_t*)buf;

	if (byteOrder == Endian::Little)
	{
		for (int i = 0; i < 4; i++) val += (1 << (i * 8)) * ubuf[i];
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			uint64_t placeVal = 1;
			placeVal <<= (24 - 8*i);

			val += placeVal * ubuf[i];
		}
	}

	return val;
}

uint64_t zmath::ToU64(char* buf, Endian byteOrder)
{
	uint16_t val = 0;

	uint8_t* ubuf = (uint8_t*)buf;

	if (byteOrder == Endian::Little)
	{
		for (int i = 0; i < 8; i++) val += (1 << (i * 8)) * ubuf[i];
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			uint64_t placeVal = 1;
			placeVal <<= (56 - 8*i);

			val += placeVal * ubuf[i];
		}
	}

	return val;
}

float zmath::ToF32(char* buf, Endian byteOrder)
{
	uint32_t val = ToU32(buf, byteOrder);
	float* uptr = reinterpret_cast<float*>(&val);

	return *uptr;
}

float zmath::ToF64(char* buf, Endian byteOrder)
{
	uint64_t val = ToU64(buf, byteOrder);
	double* uptr = reinterpret_cast<double*>(&val);

	return *uptr;
}
