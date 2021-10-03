#pragma once

#include <type_traits>
#include <algorithm>
#include <cstring>

#define ENABLE_IF_CHAR( char_t )                             \
			  typename std::enable_if<                       \
				  std::is_same<char_t, char>::value ||       \
				  std::is_same<char_t, unsigned char>::value \
		      >::type * = nullptr

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
	template <typename var_T, typename buf_T, ENABLE_IF_CHAR(buf_T)>
	void ToBytes(buf_T *buf, const var_T val, Endian byteOrder)
	{
		memcpy(buf, (const char*)(&val), sizeof(var_T));
		if (CPU_ENDIANNESS != byteOrder)
		{
			std::reverse(buf, buf + sizeof(var_T));
		}
	}

	// Deserialize a value from a binary buffer.
	// @param buf the buffer to deserialize from.
	// @param bufByteOrder the endianness with which
	//        the buffer was originally written to.
	// @return A deserialized variable of type var_T.
	template <typename var_T, typename buf_T = char, ENABLE_IF_CHAR(buf_T)>
	var_T FromBytes(const buf_T* buf, Endian bufByteOrder)
	{
		var_T var;
		memcpy(&var, buf, sizeof(var_T));
		if (CPU_ENDIANNESS != bufByteOrder)
		{
			std::reverse((char*)(&var), (char*)(&var) + sizeof(var_T));
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
	template <typename var_T, typename buf_T, ENABLE_IF_CHAR(buf_T)>
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
	template <typename var_T, typename buf_T = char, ENABLE_IF_CHAR(buf_T)>
	var_T ReadBuf(const buf_T*& buf, Endian bufByteOrder)
	{
		return FromBytes<var_T>(AdvancePtr(buf, sizeof(var_T)), bufByteOrder);
	}

	// @param buf the buffer to copy from.
	// @param n the number of bytes to copy from `buf`.
	// @return a dynamically allocated array with the first `n` bytes of byf.
	template <typename ret_T = uint8_t, typename buf_T, ENABLE_IF_CHAR(ret_T), ENABLE_IF_CHAR(buf_T)>
	ret_T* bufcpy(const buf_T* buf, size_t n)
	{
		ret_T* newbuf = new ret_T[n];
		memcpy(newbuf, buf, n);
		return newbuf;
	}
}
