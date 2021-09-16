#pragma once

#include <type_traits>
#include <algorithm>
#include <cstring>

namespace zmath
{
	enum class Endian {
		Little,
		Big,
	};

	// CPU Endianness trick from Stack Overflow post:
	// https://stackoverflow.com/questions/20451688/c-serialize-double-to-binary-file-in-little-endian
	static const int __one__ = 1;
	static const Endian CPU_ENDIANNESS = (1 == *(char*)(&__one__)) ? Endian::Little : Endian::Big;

	template <typename T> 
	void ToBytes(char* buf, const T val, Endian byteOrder)
	{
		memcpy(buf, (const char*)(&val), sizeof(T));
		if (CPU_ENDIANNESS != byteOrder)
		{
			std::reverse(buf, buf);
		}
	}

	template <typename T>
	T FromBytes(const char* buf, Endian bufByteOrder)
	{
		T var;
		memcpy(&var, buf, sizeof(T));
		if (CPU_ENDIANNESS != bufByteOrder)
		{
			std::reverse(&var, &var);
		}
		return var;
	}
}
