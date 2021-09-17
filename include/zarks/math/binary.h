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

    template <typename var_T, typename buf_T,
              typename std::enable_if<
                 std::is_same<buf_T, char>::value ||
                 std::is_same<buf_T, unsigned char>::value
			  >::type* = nullptr>
	void ToBytes(buf_T* buf, const var_T val, Endian byteOrder)
	{
		memcpy(buf, (const char*)(&val), sizeof(var_T));
		if (CPU_ENDIANNESS != byteOrder)
		{
			std::reverse(buf, buf);
		}
	}

	template <typename var_T, typename buf_T = char,
              typename std::enable_if<
                 std::is_same<buf_T, char>::value ||
                 std::is_same<buf_T, unsigned char>::value
			  >::type* = nullptr>
	var_T FromBytes(const buf_T* buf, Endian bufByteOrder)
	{
		var_T var;
		memcpy(&var, buf, sizeof(var_T));
		if (CPU_ENDIANNESS != bufByteOrder)
		{
			std::reverse(&var, &var);
		}
		return var;
	}

	// Returns the original value of the pointer when passed in
	// after incrementing the pointer by 'bytesForward'.
	template <typename T, typename std::enable_if<sizeof(T) == 1>::type* = nullptr>
	T* AdvancePtr(T*& ptr, int bytesForward)
	{
		ptr += bytesForward;
		return ptr - bytesForward;
	}
}
