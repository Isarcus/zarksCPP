#pragma once

#include <cstdint>

namespace zmath
{
	enum class Endian {
		Little,
		Big,
	};

	void ToBytes(char* buf, uint16_t val, Endian byteOrder);
	void ToBytes(char* buf, uint32_t val, Endian byteOrder);
	void ToBytes(char* buf, uint64_t val, Endian byteOrder);

	void ToBytes(char* buf, float val, Endian byteOrder);
	void ToBytes(char* buf, double val, Endian byteOrder);
}