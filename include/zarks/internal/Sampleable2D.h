#pragma once

#include <zarks/math/VecT.h>
#include <zarks/internal/zmath_internals.h>

#include <exception>

namespace zmath
{
	template <typename T>
	class Sampleable2D
	{
	protected:
		VecInt bounds;
		T** data;
		
		Sampleable2D();

		void BoundCheck(Vec check) const;
		void BoundCheck(VecInt check) const;

		virtual void FreeData();

		class Iterator
		{
		public:
			Iterator(Sampleable2D* source, VecInt pos);
			T& operator*() const;

			int& X();
			int& Y();
			void Shift(VecInt by);
			T& At(VecInt relativePos) const;

		private:
			VecInt pos;
			Sampleable2D* source;
		};

		class ConstIterator
		{
		public:
			ConstIterator(const Sampleable2D* source, VecInt pos);
			const T& operator*() const;

			int& X();
			int& Y();
			void Shift(VecInt by);
			const T& At(VecInt relativePos) const;
			
		private:
			VecInt pos;
			const Sampleable2D* source;
		};

	public:
		Sampleable2D(int x, int y, const T& val = T());
		Sampleable2D(VecInt bounds, const T& val = T());
		
		virtual ~Sampleable2D();

		bool ContainsCoord(Vec pos) const;
		bool ContainsCoord(VecInt pos) const;

		VecInt Bounds() const;

		void Set(int x, int y, const T& val);
		void Set(int x, int y, T&& val);
		void Set(VecInt pos, const T& val);
		void Set(VecInt pos, T&& val);

		const T& At(int x, int y) const;
		T& At(int x, int y);
		const T& At(VecInt pos) const;
		T& At(VecInt pos);

		const T* operator[](int x) const;
		T* operator[](int x);

		T Sample(VecInt pos) const;
		T Sample(Vec pos) const;

		Iterator GetIterator(VecInt pos);
		ConstIterator GetIterator(VecInt pos) const;
	};

	//              //
	// Sampleable2D //
	//              //

	template<typename T>
	inline Sampleable2D<T>::Sampleable2D()
		: data(nullptr)
	{}

	template<typename T>
	inline Sampleable2D<T>::Sampleable2D(VecInt bounds, const T& val)
		: bounds(VecInt::Max(bounds, VecInt(0, 0)))
	{
		data = alloc2d<T>(this->bounds.X, this->bounds.Y, val);
	}

	template<typename T>
	inline Sampleable2D<T>::Sampleable2D(int x, int y, const T& val)
		: Sampleable2D(VecInt(x, y), val)
	{}

	template<typename T>
	inline Sampleable2D<T>::~Sampleable2D()
	{
		free2d(data, bounds.X);
	}

	template<typename T>
	inline bool Sampleable2D<T>::ContainsCoord(Vec pos) const
	{
		return (pos >= Vec(0, 0) && pos < bounds);
	}

	template<typename T>
	inline bool Sampleable2D<T>::ContainsCoord(VecInt pos) const
	{
		return (pos >= VecInt(0, 0) && pos < bounds);
	}

	template<typename T>
	inline VecInt Sampleable2D<T>::Bounds() const
	{
		return bounds;
	}

	template<typename T>
	inline void Sampleable2D<T>::Set(int x, int y, const T& val)
	{
		BoundCheck(VecInt(x, y));

		data[x][y] = val;
	}

	template<typename T>
	inline void Sampleable2D<T>::Set(int x, int y, T&& val)
	{
		Set(x, y, val);
	}

	template<typename T>
	inline void Sampleable2D<T>::Set(VecInt pos, const T& val)
	{
		Set(pos.X, pos.Y, val);
	}

	template<typename T>
	inline void Sampleable2D<T>::Set(VecInt pos, T&& val)
	{
		Set(pos.X, pos.Y, val);
	}

	template<typename T>
	inline const T& Sampleable2D<T>::At(int x, int y) const
	{
		BoundCheck(VecInt(x, y));

		return data[x][y];
	}

	template<typename T>
	inline T& Sampleable2D<T>::At(int x, int y)
	{
		BoundCheck(VecInt(x, y));

		return data[x][y];
	}

	template<typename T>
	inline const T& Sampleable2D<T>::At(VecInt pos) const
	{
		return At(pos.X, pos.Y);
	}

	template<typename T>
	inline T& Sampleable2D<T>::At(VecInt pos)
	{
		return At(pos.X, pos.Y);
	}

	template<typename T>
	inline const T* Sampleable2D<T>::operator[](int x) const
	{
		return data[x];
	}

	template<typename T>
	inline T* Sampleable2D<T>::operator[](int x)
	{
		return data[x];
	}

	template<typename T>
	inline T Sampleable2D<T>::Sample(VecInt pos) const
	{
		return At(pos.X, pos.Y);
	}

	template<typename T>
	inline T Sampleable2D<T>::Sample(Vec pos) const
	{
		BoundCheck(pos);
		if (pos == (VecInt)pos)
		{
			return At(pos);
		}

		const VecInt min = pos.Floor();
		const VecInt max = pos.Ceil();
		const Vec within = pos - min;

		T y0 = interp5(data[min.X][min.Y], data[max.X, min.Y], within.X);
		T y1 = interp5(data[min.X][max.Y], data[max.X, max.Y], within.X);
		T z = interp5(y0, y1, within.Y);

		return T(z);
	}

	template<typename T>
	inline typename Sampleable2D<T>::Iterator Sampleable2D<T>::GetIterator(VecInt pos)
	{
		return Iterator(this, pos);
	}

	template<typename T>
	inline typename Sampleable2D<T>::ConstIterator Sampleable2D<T>::GetIterator(VecInt pos) const
	{
		return ConstIterator(this, pos);
	}

	template<typename T>
	inline void Sampleable2D<T>::BoundCheck(Vec check) const
	{
		if (!ContainsCoord(check))
		{
			throw std::runtime_error("Out of bounds grid access!");
		}
	}

	template<typename T>
	inline void Sampleable2D<T>::BoundCheck(VecInt check) const
	{
		if (!ContainsCoord(check))
		{
			throw std::runtime_error("Out of bounds grid access!");
		}
	}

	template<typename T>
	inline void Sampleable2D<T>::FreeData()
	{
		free2d<T>(data, bounds.X);
		bounds = VecInt(0, 0);
	}

	//          //
	// Iterator //
	//          //

	template<typename T>
	inline Sampleable2D<T>::Iterator::Iterator(Sampleable2D* source, VecInt pos)
		: source(source)
		, pos(pos)
	{}

	template<typename T>
	inline T& Sampleable2D<T>::Iterator::operator*() const
	{
		return source->At(pos);
	}

	template<typename T>
	inline int& Sampleable2D<T>::Iterator::X()
	{
		return pos.X;
	}

	template<typename T>
	inline int& Sampleable2D<T>::Iterator::Y()
	{
		return pos.Y;
	}

	template<typename T>
	inline void Sampleable2D<T>::Iterator::Shift(VecInt by)
	{
		pos += by;
	}

	template<typename T>
	inline T& Sampleable2D<T>::Iterator::At(VecInt relativePos) const
	{
		return source->At(pos + relativePos);
	}

	//               //
	// ConstIterator //
	//               //

	template<typename T>
	inline Sampleable2D<T>::ConstIterator::ConstIterator(const Sampleable2D* source, VecInt pos)
		: source(source)
		, pos(pos)
	{}

	template<typename T>
	inline const T& Sampleable2D<T>::ConstIterator::operator*() const
	{
		return source->At(pos);
	}

	template<typename T>
	inline int& Sampleable2D<T>::ConstIterator::X()
	{
		return pos.X;
	}

	template<typename T>
	inline int& Sampleable2D<T>::ConstIterator::Y()
	{
		return pos.Y;
	}

	template<typename T>
	inline void Sampleable2D<T>::ConstIterator::Shift(VecInt by)
	{
		pos += by;
	}

	template<typename T>
	inline const T& Sampleable2D<T>::ConstIterator::At(VecInt relativePos) const
	{
		return source->At(pos + relativePos);
	}

} // namespace zmath
