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

	// Returns the original value of the pointer when passed in,
	// after incrementing the pointer by 'bytesForward' bytes.
	template <typename T, typename std::enable_if<sizeof(T) == 1>::type* = nullptr>
	T* AdvancePtr(T*& ptr, int bytesForward)
	{
		ptr += bytesForward;
		return ptr - bytesForward;
	}

	// Serialize a value to a binary buffer.
	// @param buf the buffer to write to. Must have at least
	//        sizeof(var_T) free space, or using this function
	//        may result in undefined behavior.
	// @param val the variable to serialize.
	// @param byteOrder the desired endianness with which to
	//        serialize `val`.
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

	// Deserialize a value from a binary buffer.
	// @param buf the buffer to deserialize from.
	// @param bufByteOrder the endianness with which
	//        the buffer was originally written to.
	// @return A deserialized variable of type var_T.
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

	// Serialize a value to a binary buffer, and 
	//  advance the buffer along.
	// @param buf the buffer to write to. Before returning,
	//        this buffer will be advanced by sizeof(var_T)
	//        bytes.
	// @param val the value to write to the buffer.
	// @param bufByteOrder the endianness with which to
	//        serialize `val`.
	template <typename var_T, typename buf_T,
		typename std::enable_if<
			std::is_same<buf_T, char>::value ||
			std::is_same<buf_T, unsigned char>::value
		>::type* = nullptr>
	void WriteBuf(const buf_T*& buf, var_T val, Endian bufByteOrder)
	{
		ToBytes(AdvancePtr(buf, sizeof(var_T)), val, bufByteOrder);
	}

	// Deserialize a value from a binary buffer, and 
	//  advance the buffer along.
	// @param buf the buffer to deserialize from. Before
	//        returning, this buffer will be advanced by
	//        sizeof(var_T) bytes.
	// @param bufByteOrder the endianness with which the
	//        buffer was originally written to.
	// @return A deserialized variable of type var_T.
	template <typename var_T, typename buf_T = char,
			typename std::enable_if<
				std::is_same<buf_T, char>::value ||
				std::is_same<buf_T, unsigned char>::value
			>::type* = nullptr>
	var_T ReadBuf(const buf_T*& buf, Endian bufByteOrder)
	{
		return FromBytes<var_T>(AdvancePtr(buf, sizeof(var_T)), bufByteOrder);
	}
}
