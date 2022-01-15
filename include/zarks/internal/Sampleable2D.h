#pragma once

#include <zarks/math/VecT.h>
#include <zarks/internal/zmath_internals.h>

#include <exception>
#include <cstring>

namespace zmath
{
	template <typename T>
	class Sampleable2D
	{
	protected:
		VecInt bounds;
		size_t capacity;
		T* data;

		void BoundCheck(Vec check) const;
		void BoundCheck(VecInt check) const;

		size_t idx_of(int x, int y) const;
		size_t idx_of(VecInt vec) const;
		T& at_itl(int x, int y);
		const T& at_itl(int x, int y) const;
		T& at_itl(VecInt vec);
		const T& at_itl(VecInt vec) const;

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
		Sampleable2D();
		Sampleable2D(int x, int y, const T& val = T());
		Sampleable2D(VecInt bounds, const T& val = T());

		Sampleable2D(const Sampleable2D& s);
		Sampleable2D(Sampleable2D&& s);
		Sampleable2D& operator=(const Sampleable2D& s);
		Sampleable2D& operator=(Sampleable2D&& s);
		
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

		const T& operator()(int x, int y) const;
		T& operator()(int x, int y);
		const T& operator()(VecInt pos) const;
		T& operator()(VecInt pos);

		void Clear(T val);
		void FillBorder(int thickness, T val);
		void Fill(VecInt min, VecInt max, T val);

		template <typename FUNC>
		void Apply(FUNC f);
		template <typename FUNC>
		void ApplyCoords(FUNC f);

		void CopyInRange(const Sampleable2D& samp, VecInt min, VecInt max, VecInt to = VecInt(0, 0));
		void CopyNotInRange(const Sampleable2D& samp, VecInt min, VecInt max, VecInt to = VecInt(0, 0));

		T Sample(VecInt pos) const;
		T Sample(Vec pos) const;

		void FlipVertical();
		void FlipHorizontal();

		Iterator GetIterator(VecInt pos);
		ConstIterator GetIterator(VecInt pos) const;
	};

	//              //
	// Sampleable2D //
	//              //

	template<typename T>
	inline Sampleable2D<T>::Sampleable2D()
		: bounds(0, 0)
		, capacity(0)
		, data(nullptr)
	{}

	template<typename T>
	inline Sampleable2D<T>::Sampleable2D(VecInt bounds, const T& val)
		: bounds(VecInt::Max(bounds, VecInt(0, 0)))
		, capacity(bounds.Area())
		, data(new T[capacity])
	{
		Clear(val);
	}

	template<typename T>
	inline Sampleable2D<T>::Sampleable2D(int x, int y, const T& val)
		: Sampleable2D(VecInt(x, y), val)
	{}

	template<typename T>
	inline Sampleable2D<T>::Sampleable2D(const Sampleable2D& s)
		: bounds(s.bounds)
		, capacity(s.bounds.Area())
		, data(new T[capacity])
	{
		memcpy(data, s.data, capacity * sizeof(T));
	}

	template<typename T>
	inline Sampleable2D<T>::Sampleable2D(Sampleable2D&& s)
		: bounds(s.bounds)
		, capacity(s.capacity)
		, data(s.data)
	{
		s.bounds = VecInt(0, 0);
		s.capacity = 0;
		s.data = nullptr;
	}

	template<typename T>
	inline Sampleable2D<T>& Sampleable2D<T>::operator=(const Sampleable2D& s)
	{
		if (this != &s)
		{
			size_t reqCapacity = s.bounds.Area();
			if (capacity < reqCapacity)
			{
				delete[] data;
				data = new T[reqCapacity];
				capacity = reqCapacity;
			}
			memcpy(data, s.data, reqCapacity * sizeof(T));
			bounds = s.bounds;
		}

		return *this;
	}

	template<typename T>
	inline Sampleable2D<T>& Sampleable2D<T>::operator=(Sampleable2D&& s)
	{
		if (this != &s)
		{
			delete[] data;
			data = s.data;
			capacity = s.capacity;
			bounds = s.bounds;

			s.data = nullptr;
			s.capacity = 0;
			s.bounds = VecInt(0, 0);
		}

		return *this;
	}

	template<typename T>
	inline Sampleable2D<T>::~Sampleable2D()
	{
		bounds = VecInt(0, 0);
		capacity = 0;
		delete[] data;
		data = nullptr;
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

		data[idx_of(x, y)] = val;
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

		return data[idx_of(x, y)];
	}

	template<typename T>
	inline T& Sampleable2D<T>::At(int x, int y)
	{
		BoundCheck(VecInt(x, y));

		return data[idx_of(x, y)];
	}

	template<typename T>
	inline const T& Sampleable2D<T>::At(VecInt pos) const
	{
		BoundCheck(pos);
		return data[idx_of(pos)];
	}

	template<typename T>
	inline T& Sampleable2D<T>::At(VecInt pos)
	{
		BoundCheck(pos);
		return data[idx_of(pos)];
	}

	template<typename T>
	inline const T& Sampleable2D<T>::operator()(int x, int y) const
	{
		return data[idx_of(x, y)];
	}

	template<typename T>
	inline T& Sampleable2D<T>::operator()(int x, int y)
	{
		return data[idx_of(x, y)];
	}

	template<typename T>
	inline const T& Sampleable2D<T>::operator()(VecInt pos) const
	{
		return data[idx_of(pos)];
	}

	template<typename T>
	inline T& Sampleable2D<T>::operator()(VecInt pos)
	{
		return data[idx_of(pos)];
	}

	template <typename T>
	inline void Sampleable2D<T>::Clear(T val)
	{
		size_t size = bounds.Area();
		for (size_t i = 0; i < size; i++)
		{
			data[i] = val;
		}
	}

	template <typename T>
	inline void Sampleable2D<T>::FillBorder(int thickness, T val)
	{
		thickness = std::min(thickness, bounds.Min());
		// Left
		Fill({ 0, 0 }, { thickness, bounds.Y }, val);
		// Right
		Fill({ bounds.X - thickness, 0 }, { bounds.X, bounds.Y }, val);
		// Top (no corners)
		Fill({ thickness, bounds.Y - thickness }, { bounds.X - thickness, bounds.Y }, val);
		// Bottom (no corners)
		Fill({ thickness, 0 }, { bounds.X - thickness, thickness }, val);
	}

	template <typename T>
	inline void Sampleable2D<T>::Fill(VecInt min, VecInt max, T val)
	{
		min = VecInt::Max(min, VecInt(0, 0));
		max = VecInt::Min(max, bounds);
		for (int x = min.X; x < max.X; x++)
		{
			for (int y = min.Y; y < max.Y; y++)
			{
				at_itl(x, y) = val;
			}
		}
	}

	template <typename T>
	template <typename FUNC>
	void Sampleable2D<T>::Apply(FUNC f)
	{
		size_t len = bounds.Area();
		for (size_t i = 0; i < len; i++)
		{
			data[i] = f(i);
		}
	}

	template <typename T>
	template <typename FUNC>
	void Sampleable2D<T>::ApplyCoords(FUNC f)
	{
		size_t len = bounds.Area();
		for (int x = 0; x < bounds.X; x++)
		{
			for (int y = 0; y < bounds.Y; y++)
			{
				at_itl(x, y) = f(x, y);
			}
		}
	}

	template<typename T>
	void Sampleable2D<T>::CopyInRange(const Sampleable2D<T>& samp, VecInt min, VecInt max, VecInt to)
	{
		VecInt setCoord = Vec::Max(Vec(0, 0), to);
		for (int x = min.X; x < max.X && setCoord.X < bounds.X; x++, setCoord.X++)
		{
			setCoord.Y = to.Y;
			for (int y = min.Y; y < max.Y && setCoord.Y < bounds.Y; y++, setCoord.Y++)
			{
				VecInt coord(x, y);
				Set(setCoord, samp.At(coord));
			}
		}
	}

	template<typename T>
	void Sampleable2D<T>::CopyNotInRange(const Sampleable2D<T>& samp, VecInt min, VecInt max, VecInt to)
	{
		const VecInt otherBounds(samp.bounds);
		VecInt otherCoord(0, 0);
		for (int x = to.X; x < bounds.X && otherCoord.X < otherBounds.X; x++, otherCoord.X++)
		{
			otherCoord.Y = 0;
			for (int y = to.Y; y < bounds.Y && otherCoord.Y < otherBounds.Y; y++, otherCoord.Y++)
			{
				// If y has reached minimum x bound, skip to max y bound
				if (otherCoord.Y >= min.Y && otherCoord.Y < max.Y &&
					otherCoord.X >= min.X && otherCoord.X < max.X)
				{
					y += max.Y - otherCoord.Y - 1;
					otherCoord.Y = max.Y - 1;
					continue;
				}

				Set(x, y, samp.At(otherCoord));
			}
		}
	}

	template<typename T>
	inline T Sampleable2D<T>::Sample(VecInt pos) const
	{
		return At(pos.X, pos.Y);
	}

	template<typename T>
	inline T Sampleable2D<T>::Sample(Vec pos) const
	{
		if (pos == pos.Floor())
		{
			return At(pos);
		}

		const VecInt min = Vec::Max(pos.Floor(), Vec(0, 0));
		const VecInt max = Vec::Min(min + Vec(1, 1), bounds - Vec(1, 1));
		const Vec within = pos - min;

		T y0 = interp5(data[idx_of(min.X, min.Y)], data[idx_of(max.X, min.Y)], within.X);
		T y1 = interp5(data[idx_of(min.X, max.Y)], data[idx_of(max.X, max.Y)], within.X);
		T z = interp5(y0, y1, within.Y);

		return T(z);
	}

	template<typename T>
	void  Sampleable2D<T>::FlipVertical()
	{
		for (int x = 0; x < bounds.X; x++)
		{
			for (int y = 0; y < bounds.Y / 2; y++)
			{
				std::swap(data[idx_of(x, y)], data[idx_of(x, bounds.Y - 1 - y)]);
			}
		}
	}

	template<typename T>
	void  Sampleable2D<T>::FlipHorizontal()
	{
		for (int x = 0; x < bounds.X / 2; x++)
		{
			for (int y = 0; y < bounds.Y; y++)
			{
				std::swap(data[idx_of(x, y)], data[idx_of(bounds.X - 1 - x, y)]);
			}
		}
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
	inline size_t Sampleable2D<T>::idx_of(int x, int y) const
	{
		return x * bounds.Y + y;
	}

	template<typename T>
	inline size_t Sampleable2D<T>::idx_of(VecInt vec) const
	{
		return vec.X * bounds.Y + vec.Y;
	}

	template<typename T>
	inline T& Sampleable2D<T>::at_itl(int x, int y)
	{
		return data[idx_of(x, y)];
	}

	template<typename T>
	inline const T& Sampleable2D<T>::at_itl(int x, int y) const
	{
		return data[idx_of(x, y)];
	}

	template<typename T>
	inline T& Sampleable2D<T>::at_itl(VecInt vec)
	{
		return data[idx_of(vec)];
	}

	template<typename T>
	inline const T& Sampleable2D<T>::at_itl(VecInt vec) const
	{
		return data[idx_of(vec)];
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
