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
}