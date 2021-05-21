#pragma once

#include <cstdint>

namespace zmath
{
	enum class Endian {
		Little,
		Big,
	};

	void ToBytes(uint8_t* buf, uint16_t val, Endian byteOrder);
	void ToBytes(uint8_t* buf, uint32_t val, Endian byteOrder);
	void ToBytes(uint8_t* buf, uint64_t val, Endian byteOrder);

	void ToBytes(uint8_t* buf, float  val, Endian byteOrder);
	void ToBytes(uint8_t* buf, double val, Endian byteOrder);

	uint16_t ToU16(char* buf, Endian byteOrder);
	uint32_t ToU32(char* buf, Endian byteOrder);
	uint64_t ToU64(char* buf, Endian byteOrder);

	float ToF32(char* buf, Endian byteOrder);
	float ToF64(char* buf, Endian byteOrder);
}