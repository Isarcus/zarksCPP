#pragma once

#include <zarks/math/VecT.h>
#include <zarks/math/RecT.h>
#include <zarks/internal/zmath_internals.h>

#include <exception>
#include <cstring>
#include <sstream>
#include <type_traits>

namespace zmath
{
    class Indices;

    template <typename>
    class IndicesRef;

    template <typename>
    class IndicesConstRef;

    template <typename T>
    class Mat2D
    {
    protected:
        VecInt bounds;
        size_t capacity;
        T* data;

        void assertContains(Vec check) const;
        void assertContains(VecInt check) const;

        template <typename W>
        void assertSameSize(const Mat2D<W>& mat) const;

        VecInt pos_of(int idx) const;
        size_t idx_of(int x, int y) const;
        size_t idx_of(VecInt vec) const;
        T& at_itl(int x, int y);
        const T& at_itl(int x, int y) const;
        T& at_itl(VecInt vec);
        const T& at_itl(VecInt vec) const;

        //           //
        // Iterators //
        //           //

        class IteratorBase
        {
        public:
            IteratorBase(Mat2D* source, T* datum);

            VecInt Pos() const;

            void Shift(VecInt by);
            T& At(VecInt relativePos) const;

            IteratorBase& operator++();
            IteratorBase operator++(int);

            bool operator==(const IteratorBase& iter);
            bool operator!=(const IteratorBase& iter);

        protected:
            Mat2D* source;
            T* datum;
        };

        class Iterator : public IteratorBase
        {
        public:
            Iterator(Mat2D* source, T* datum);

            T& operator*() const;
            Iterator& operator++();
            Iterator operator++(int);
        };

        class ConstIterator : public IteratorBase
        {
        public:
            ConstIterator(const Mat2D* source, const T* datum);

            const T& operator*() const;
            ConstIterator& operator++();
            ConstIterator operator++(int);
        };

    public:
        Mat2D();
        Mat2D(int x, int y, const T& val = T());
        Mat2D(VecInt bounds, const T& val = T());

        Mat2D(const Mat2D& s);
        Mat2D(Mat2D&& s);

        template <typename FUNC = T(*)(), std::enable_if_t<std::is_invocable_v<FUNC>, bool> = true>
        Mat2D(int x, int y, FUNC f);
        template <typename FUNC = T(*)(), std::enable_if_t<std::is_invocable_v<FUNC>, bool> = true>
        Mat2D(VecInt bounds, FUNC f);
        template <typename FUNC = T(*)(int, int), std::enable_if_t<std::is_invocable_v<FUNC, int, int>, bool> = true>
        Mat2D(int x, int y, FUNC f);
        template <typename FUNC = T(*)(int, int), std::enable_if_t<std::is_invocable_v<FUNC, int, int>, bool> = true>
        Mat2D(VecInt bounds, FUNC f);
        template <typename W, typename FUNC = W(*)(W), std::enable_if_t<std::is_invocable_v<FUNC, W>, bool> = true>
        Mat2D(const Mat2D<W>& s, FUNC f);

        Mat2D& operator=(const Mat2D& s);
        Mat2D& operator=(Mat2D&& s);
        
        virtual ~Mat2D();

        Indices operator==(T val) const;
        Indices operator!=(T val) const;
        Indices operator< (T val) const;
        Indices operator> (T val) const;
        Indices operator<=(T val) const;
        Indices operator>=(T val) const;

        bool ContainsCoord(Vec pos) const;
        bool ContainsCoord(VecInt pos) const;

        VecInt Bounds() const;
        size_t Area() const;

        void Set(int x, int y, T val);
        void Set(VecInt pos, T val);

        const T& At(int x, int y) const;
        T& At(int x, int y);
        const T& At(VecInt pos) const;
        T& At(VecInt pos);

        const T& operator()(int x, int y) const;
        T& operator()(int x, int y);
        const T& operator()(VecInt pos) const;
        T& operator()(VecInt pos);

        IndicesRef<T> operator()(const Indices& indices);
        IndicesRef<T> operator()(Indices&& indices);

        IndicesConstRef<T> operator()(const Indices& indices) const;
        IndicesConstRef<T> operator()(Indices&& indices) const;

        template <typename LESS = std::less<T>>
        T GetMin(LESS less = LESS()) const;
        template <typename LESS = std::less<T>>
        T GetMax(LESS less = LESS()) const;
        template <typename LESS = std::less<T>>
        std::pair<T, T> GetMinMax(LESS less = LESS()) const;

        void Resize(int x, int y, T clearVal = T());
        void Resize(VecInt newBounds, T clearVal = T());
        void Clear(T val);
        void Replace(T val, T with);
        void FillBorder(int thickness, T val);
        void Fill(VecInt min, VecInt max, T val);
        void Paste(const Mat2D& mat, VecInt at);

        // Apply a function to each element in this. The passed-in function
        // must return a type that is implicitly convertible to T. The
        // function's arguments may fall into any one of the following
        // categories:
        // 1. ()            : No inputs
        // 2. (T)           : A function of the current datum
        // 3. (int, int)    : A function of coordinates only
        // 4. (T, int, int) : A function of the current datum and coordinates
        template <typename FUNC = T(*)(), std::enable_if_t<std::is_invocable_v<FUNC>, bool> = true>
        void Apply(FUNC f);
        template <typename FUNC = T(*)(T), std::enable_if_t<std::is_invocable_v<FUNC, T>, bool> = true>
        void Apply(FUNC f);
        template <typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, int, int>, bool> = true>
        void Apply(FUNC f);
        template <typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, T, int, int>, bool> = true>
        void Apply(FUNC f);

        // Apply a function to each element in this, based on another Mat2D.
        // The datum type of the passed-in Mat2D does not need to match that
        // of the called Mat2D. Where W represents the datum type of the passed
        // object, The function's arguments may fall into any one of the
        // following categories:
        // 1. (W)              : A function of the corresponding datum of the
        //                       passed Mat2D
        // 2. (T, W)           : A function of the current datum, as well as
        //                       the corresponding datum of the passed Mat2D
        // 3. (T, W, int, int) : A function of the current datum, the
        //                       corresponding datum of the passed Mat2D, and
        //                       the current coordinate.
        template <typename W, typename FUNC = W(*)(W), std::enable_if_t<std::is_invocable_v<FUNC, W>, bool> = true>
        void ApplySample(const Mat2D<W>& mat, FUNC f);
        template <typename W, typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, T, W>, bool> = true>
        void ApplySample(const Mat2D<W>& mat, FUNC f);
        template <typename W, typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, T, W, int, int>, bool> = true>
        void ApplySample(const Mat2D<W>& mat, FUNC f);

        void CopyInRange(const Mat2D& mat, VecInt min, VecInt max, VecInt to = VecInt(0, 0));
        void CopyNotInRange(const Mat2D& mat, VecInt min, VecInt max, VecInt to = VecInt(0, 0));

        T Sample(VecInt pos) const;
        T Sample(Vec pos) const;

        void FlipVertical();
        void FlipHorizontal();

        Iterator GetIterator(VecInt pos);
        ConstIterator GetIterator(VecInt pos) const;

        Iterator begin();
        ConstIterator begin() const;

        Iterator end();
        ConstIterator end() const;
    };

    //         //
    // Indices //
    //         //

    class Indices : public Mat2D<bool>
    {
    public:
        template <typename T>
        Indices(const Mat2D<T>& source, bool(*func)(T, T), T val);

        Indices operator&& (const Indices& rhs) const;
        Indices operator|| (const Indices& rhs) const;
        Indices operator^ (const Indices& rhs) const;
        Indices operator&& (Indices&& rhs) const;
        Indices operator|| (Indices&& rhs) const;
        Indices operator^ (Indices&& rhs) const;

        template <typename T>
        static bool EQ(T val1, T val2);
        template <typename T>
        static bool NE(T val1, T val2);
        template <typename T>
        static bool LT(T val1, T val2);
        template <typename T>
        static bool GT(T val1, T val2);
        template <typename T>
        static bool LE(T val1, T val2);
        template <typename T>
        static bool GE(T val1, T val2);
    };

    template <typename T>
    class IndicesRefBase
    {
    protected:
        Indices indices;
        Mat2D<T>& ref;

        IndicesRefBase(const Indices& indices, Mat2D<T>& mat);
        IndicesRefBase(Indices&& indices, Mat2D<T>& mat);
    };

    template <typename T>
    class IndicesRef : public IndicesRefBase<T>
    {
    public:
        IndicesRef(const Indices& indices, Mat2D<T>& mat);
        IndicesRef(Indices&& indices, Mat2D<T>& mat);

        void operator=  (T val);
        void operator+= (T val);
        void operator-= (T val);
        void operator*= (T val);
        void operator/= (T val);
        
        void operator=  (const Mat2D<T>& mat);
        void operator+= (const Mat2D<T>& mat);
        void operator-= (const Mat2D<T>& mat);
        void operator*= (const Mat2D<T>& mat);
        void operator/= (const Mat2D<T>& mat);

    };

    template <typename T>
    class IndicesConstRef : public IndicesRefBase<T>
    {
    public:
        IndicesConstRef(const Indices& indices, Mat2D<T>& mat);
        IndicesConstRef(Indices&& indices, Mat2D<T>& mat);
    };

    //       //
    // Mat2D //
    //       //

    template <typename T>
    inline Mat2D<T>::Mat2D()
        : bounds(0, 0)
        , capacity(0)
        , data(nullptr)
    {}

    template <typename T>
    inline Mat2D<T>::Mat2D(VecInt bounds, const T& val)
        : bounds(VecInt::Max(bounds, VecInt(0, 0)))
        , capacity(bounds.Area())
        , data(new T[capacity])
    {
        Clear(val);
    }

    template <typename T>
    inline Mat2D<T>::Mat2D(int x, int y, const T& val)
        : Mat2D(VecInt(x, y), val)
    {}

    template <typename T>
    inline Mat2D<T>::Mat2D(const Mat2D& s)
        : bounds(s.bounds)
        , capacity(s.bounds.Area())
        , data(new T[capacity])
    {
        memcpy(data, s.data, capacity * sizeof(T));
    }

    template <typename T>
    inline Mat2D<T>::Mat2D(Mat2D&& s)
        : bounds(s.bounds)
        , capacity(s.capacity)
        , data(s.data)
    {
        s.bounds = VecInt(0, 0);
        s.capacity = 0;
        s.data = nullptr;
    }

    template <typename T>
    template <typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC>, bool>>
    inline Mat2D<T>::Mat2D(int x, int y, FUNC f)
        : Mat2D(x, y)
    {
        Apply(f);
    }

    template <typename T>
    template <typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC>, bool>>
    inline Mat2D<T>::Mat2D(VecInt bounds, FUNC f)
        : Mat2D(bounds)
    {
        Apply(f);
    }

    template <typename T>
    template <typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, int, int>, bool>>
    inline Mat2D<T>::Mat2D(int x, int y, FUNC f)
        : Mat2D(x, y)
    {
        Apply(f);
    }

    template <typename T>
    template <typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, int, int>, bool>>
    inline Mat2D<T>::Mat2D(VecInt bounds, FUNC f)
        : Mat2D(bounds)
    {
        Apply(f);
    }

    template <typename T>
    template <typename W, typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, W>, bool>>
    inline Mat2D<T>::Mat2D(const Mat2D<W>& s, FUNC f)
        : bounds(s.Bounds())
        , capacity(bounds.Area())
        , data(new T[capacity])
    {
        ApplySample(s, f);
    }

    template <typename T>
    inline Mat2D<T>& Mat2D<T>::operator=(const Mat2D& s)
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

    template <typename T>
    inline Mat2D<T>& Mat2D<T>::operator=(Mat2D&& s)
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

    template <typename T>
    inline Mat2D<T>::~Mat2D()
    {
        bounds = VecInt(0, 0);
        capacity = 0;
        delete[] data;
        data = nullptr;
    }

    template <typename T>
    inline Indices Mat2D<T>::operator==(T val) const
    {
        return Indices(*this, Indices::EQ, val);
    }

    template <typename T>
    inline Indices Mat2D<T>::operator!=(T val) const
    {
        return Indices(*this, Indices::NE, val);
    }

    template <typename T>
    inline Indices Mat2D<T>::operator< (T val) const
    {
        return Indices(*this, Indices::LT, val);
    }
    
    template <typename T>
    inline Indices Mat2D<T>::operator> (T val) const
    {
        return Indices(*this, Indices::GT, val);
    }
    
    template <typename T>
    inline Indices Mat2D<T>::operator<=(T val) const
    {
        return Indices(*this, Indices::LE, val);
    }

    template <typename T>
    inline Indices Mat2D<T>::operator>=(T val) const
    {
        return Indices(*this, Indices::GE, val);
    }

    template <typename T>
    inline bool Mat2D<T>::ContainsCoord(Vec pos) const
    {
        return (pos >= Vec(0, 0) && pos < bounds);
    }

    template <typename T>
    inline bool Mat2D<T>::ContainsCoord(VecInt pos) const
    {
        return (pos >= VecInt(0, 0) && pos < bounds);
    }

    template <typename T>
    inline VecInt Mat2D<T>::Bounds() const
    {
        return bounds;
    }

    template <typename T>
    inline size_t Mat2D<T>::Area() const
    {
        return bounds.Area();
    }

    template <typename T>
    inline void Mat2D<T>::Set(int x, int y, T val)
    {
        assertContains(VecInt(x, y));

        data[idx_of(x, y)] = val;
    }

    template <typename T>
    inline void Mat2D<T>::Set(VecInt pos, T val)
    {
        Set(pos.X, pos.Y, val);
    }

    template <typename T>
    inline const T& Mat2D<T>::At(int x, int y) const
    {
        assertContains(VecInt(x, y));
        return data[idx_of(x, y)];
    }

    template <typename T>
    inline T& Mat2D<T>::At(int x, int y)
    {
        assertContains(VecInt(x, y));
        return data[idx_of(x, y)];
    }

    template <typename T>
    inline const T& Mat2D<T>::At(VecInt pos) const
    {
        assertContains(pos);
        return data[idx_of(pos)];
    }

    template <typename T>
    inline T& Mat2D<T>::At(VecInt pos)
    {
        assertContains(pos);
        return data[idx_of(pos)];
    }

    template <typename T>
    inline const T& Mat2D<T>::operator()(int x, int y) const
    {
        return data[idx_of(x, y)];
    }

    template <typename T>
    inline T& Mat2D<T>::operator()(int x, int y)
    {
        return data[idx_of(x, y)];
    }

    template <typename T>
    inline const T& Mat2D<T>::operator()(VecInt pos) const
    {
        return data[idx_of(pos)];
    }

    template <typename T>
    inline T& Mat2D<T>::operator()(VecInt pos)
    {
        return data[idx_of(pos)];
    }

    template <typename T>
    inline IndicesRef<T> Mat2D<T>::operator()(const Indices& indices)
    {
        return IndicesRef<T>(indices, *this);
    }

    template <typename T>
    inline IndicesRef<T> Mat2D<T>::operator()(Indices&& indices)
    {
        return IndicesRef<T>(std::move(indices), *this);
    }

    template <typename T>
    inline IndicesConstRef<T> Mat2D<T>::operator()(const Indices& indices) const
    {
        return IndicesConstRef<T>(indices, *this);
    }

    template <typename T>
    inline IndicesConstRef<T> Mat2D<T>::operator()(Indices&& indices) const
    {
        return IndicesConstRef<T>(std::move(indices), *this);
    }
    
    template <typename T>
    template <typename LESS>
    inline T Mat2D<T>::GetMin(LESS less) const
    {
        T min = At(0, 0);
        size_t area = Area();
        for (size_t i = 0; i < area; i++)
        {
            min = less(data[i], min) ? data[i] : min;
        }
        return min;
    }
    
    template <typename T>
    template <typename LESS>
    inline T Mat2D<T>::GetMax(LESS less) const
    {
        T max = At(0, 0);
        size_t area = Area();
        for (size_t i = 0; i < area; i++)
        {
            max = less(max, data[i]) ? data[i] : max;
        }
        return max; 
    }
    
    template <typename T>
    template <typename LESS>
    inline std::pair<T, T> Mat2D<T>::GetMinMax(LESS less) const
    {
        T min, max;
        min = max = At(0, 0);
        size_t area = Area();
        for (size_t i = 0; i < area; i++)
        {
            min = less(data[i], min) ? data[i] : min;
            max = less(max, data[i]) ? data[i] : max;
        }
        return {min, max};
    }

    template <typename T>
    inline void Mat2D<T>::Resize(int x, int y, T clearVal)
    {
        Resize(VecInt(x, y), clearVal);
    }

    template <typename T>
    inline void Mat2D<T>::Resize(VecInt newBounds, T clearVal)
    {
        size_t req_capacity = newBounds.Area();
        if (req_capacity > capacity)
        {
            delete[] data;
            data = new T[req_capacity];
            capacity = req_capacity;
        }
        
        bounds = newBounds;
        Clear(clearVal);
    }

    template <typename T>
    inline void Mat2D<T>::Clear(T val)
    {
        Apply([=](T){ return val; });
    }

    template <typename T>
    inline void Mat2D<T>::Replace(T val, T with)
    {
        Apply([=](T v){
            return (v == val) ? with : v;
        });
    }

    template <typename T>
    inline void Mat2D<T>::FillBorder(int thickness, T val)
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
    inline void Mat2D<T>::Fill(VecInt min, VecInt max, T val)
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
    inline void Mat2D<T>::Paste(const Mat2D<T>& mat, VecInt at)
    {
        RectInt r = RectInt(bounds).Intersection(at, at + mat.bounds);
        for (int x = r.Min().X; x < r.Max().X; x++)
        {
            for (int y = r.Min().Y; y < r.Max().Y; y++)
            {
                at_itl(x, y) = mat.at_itl(VecInt(x, y) - at);
            }
        }
    }

    template <typename T>
    template <typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC>, bool>>
    inline void Mat2D<T>::Apply(FUNC f)
    {
        size_t len = bounds.Area();
        for (size_t i = 0; i < len; i++)
        {
            data[i] = f();
        }
    }

    template <typename T>
    template <typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, T>, bool>>
    inline void Mat2D<T>::Apply(FUNC f)
    {
        size_t len = bounds.Area();
        for (size_t i = 0; i < len; i++)
        {
            data[i] = f(data[i]);
        }
    }

    template <typename T>
    template <typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, int, int>, bool>>
    inline void Mat2D<T>::Apply(FUNC f)
    {
        for (int x = 0; x < bounds.X; x++)
        {
            for (int y = 0; y < bounds.Y; y++)
            {
                at_itl(x, y) = f(x, y);
            }
        }
    }

    template <typename T>
    template <typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, T, int, int>, bool>>
    inline void Mat2D<T>::Apply(FUNC f)
    {
        for (int x = 0; x < bounds.X; x++)
        {
            for (int y = 0; y < bounds.Y; y++)
            {
                at_itl(x, y) = f(at_itl(x, y), x, y);
            }
        }
    }

    template <typename T>
    template <typename W, typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, W>, bool>>
    inline void Mat2D<T>::ApplySample(const Mat2D<W>& mat, FUNC f)
    {
        auto this_it = begin(), this_end = end();
        auto mat_it = mat.begin(), mat_end = mat.end();
        while (this_it != this_end && mat_it != mat_end)
        {
            *this_it++ = f(*mat_it++);
        }
    }

    template <typename T>
    template <typename W, typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, T, W, int, int>, bool>>
    inline void Mat2D<T>::ApplySample(const Mat2D<W>& mat, FUNC f)
    {
        assertSameSize(mat);
        for (int x = 0; x < bounds.X; x++)
        {
            for (int y = 0; y < bounds.Y; y++)
            {
                at_itl(x, y) = f(at_itl(x, y), mat(x, y), x, y);
            }
        }
    }

    template <typename T>
    template <typename W, typename FUNC, std::enable_if_t<std::is_invocable_v<FUNC, T, W>, bool>>
    inline void Mat2D<T>::ApplySample(const Mat2D<W>& mat, FUNC f)
    {
        auto this_it = begin(), this_end = end();
        auto mat_it = mat.begin(), mat_end = mat.end();
        while (this_it != this_end && mat_it != mat_end)
        {
            const T& datum = *this_it;
            *this_it++ = f(datum, *mat_it++);
        }
    }

    template <typename T>
    inline void Mat2D<T>::CopyInRange(const Mat2D<T>& mat, VecInt min, VecInt max, VecInt to)
    {
        VecInt setCoord = Vec::Max(Vec(0, 0), to);
        for (int x = min.X; x < max.X && setCoord.X < bounds.X; x++, setCoord.X++)
        {
            setCoord.Y = to.Y;
            for (int y = min.Y; y < max.Y && setCoord.Y < bounds.Y; y++, setCoord.Y++)
            {
                VecInt coord(x, y);
                Set(setCoord, mat.At(coord));
            }
        }
    }

    template <typename T>
    inline void Mat2D<T>::CopyNotInRange(const Mat2D<T>& mat, VecInt min, VecInt max, VecInt to)
    {
        const VecInt otherBounds(mat.bounds);
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

                Set(x, y, mat.At(otherCoord));
            }
        }
    }

    template <typename T>
    inline T Mat2D<T>::Sample(VecInt pos) const
    {
        return At(pos.X, pos.Y);
    }

    template <typename T>
    inline T Mat2D<T>::Sample(Vec pos) const
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

    template <typename T>
    inline void  Mat2D<T>::FlipVertical()
    {
        for (int x = 0; x < bounds.X; x++)
        {
            for (int y = 0; y < bounds.Y / 2; y++)
            {
                std::swap(data[idx_of(x, y)], data[idx_of(x, bounds.Y - 1 - y)]);
            }
        }
    }

    template <typename T>
    inline void  Mat2D<T>::FlipHorizontal()
    {
        for (int x = 0; x < bounds.X / 2; x++)
        {
            for (int y = 0; y < bounds.Y; y++)
            {
                std::swap(data[idx_of(x, y)], data[idx_of(bounds.X - 1 - x, y)]);
            }
        }
    }

    template <typename T>
    inline typename Mat2D<T>::Iterator Mat2D<T>::GetIterator(VecInt pos)
    {
        return Iterator(this, &At(pos));
    }

    template <typename T>
    inline typename Mat2D<T>::ConstIterator Mat2D<T>::GetIterator(VecInt pos) const
    {
        return ConstIterator(this, &At(pos));
    }

    template <typename T>
    inline typename Mat2D<T>::Iterator Mat2D<T>::begin()
    {
        return GetIterator(VecInt(0, 0));
    }
    
    template <typename T>
    inline typename Mat2D<T>::ConstIterator Mat2D<T>::begin() const
    {
        return GetIterator(VecInt(0, 0));
    }

    template <typename T>
    inline typename Mat2D<T>::Iterator Mat2D<T>::end()
    {
        return Iterator(this, nullptr);
    }

    template <typename T>
    inline typename Mat2D<T>::ConstIterator Mat2D<T>::end() const
    {
        return ConstIterator(this, nullptr);
    }

    template <typename T>
    inline void Mat2D<T>::assertContains(Vec check) const
    {
        if (!ContainsCoord(check))
        {
            std::ostringstream os;
            os << "Mat2D out of bounds error: bounds = "
               << bounds << ", but point = " << check; 
            throw std::runtime_error(os.str());
        }
    }

    template <typename T>
    inline void Mat2D<T>::assertContains(VecInt check) const
    {
        if (!ContainsCoord(check))
        {
            std::ostringstream os;
            os << "Mat2D out of bounds error: bounds = "
               << bounds << ", but point = " << check; 
            throw std::runtime_error(os.str());
        }
    }

    template <typename T>
    template <typename W>
    inline void Mat2D<T>::assertSameSize(const Mat2D<W>& mat) const
    {
        if (bounds != mat.Bounds())
        {
            std::ostringstream os;
            os << "Mat2D bounds mismatch: " << bounds << " vs. " << mat.Bounds();
            throw std::runtime_error(os.str());
        }
    }

    template <typename T>
    inline VecInt Mat2D<T>::pos_of(int idx) const
    {
        return VecInt(
            idx / bounds.Y,
            idx % bounds.Y
        );
    }

    template <typename T>
    inline size_t Mat2D<T>::idx_of(int x, int y) const
    {
        return x * bounds.Y + y;
    }

    template <typename T>
    inline size_t Mat2D<T>::idx_of(VecInt vec) const
    {
        return vec.X * bounds.Y + vec.Y;
    }

    template <typename T>
    inline T& Mat2D<T>::at_itl(int x, int y)
    {
        return data[idx_of(x, y)];
    }

    template <typename T>
    inline const T& Mat2D<T>::at_itl(int x, int y) const
    {
        return data[idx_of(x, y)];
    }

    template <typename T>
    inline T& Mat2D<T>::at_itl(VecInt vec)
    {
        return data[idx_of(vec)];
    }

    template <typename T>
    inline const T& Mat2D<T>::at_itl(VecInt vec) const
    {
        return data[idx_of(vec)];
    }

    //              //
    // IteratorBase //
    //              //

    template <typename T>
    inline Mat2D<T>::IteratorBase::IteratorBase(Mat2D<T>* source, T* datum)
        : source(source)
        , datum(datum)
    {}

    template <typename T>
    inline VecInt Mat2D<T>::IteratorBase::Pos() const
    {
        return source->pos_of(datum - source->data);
    }

    template <typename T>
    inline void Mat2D<T>::IteratorBase::Shift(VecInt by)
    {
        datum = &source->At(Pos() + by);
    }

    template <typename T>
    inline T& Mat2D<T>::IteratorBase::At(VecInt relativePos) const
    {
        return source->At(Pos() + relativePos);
    }

    template <typename T>
    inline typename Mat2D<T>::IteratorBase& Mat2D<T>::IteratorBase::operator++()
    {
        if (datum)
        {
            datum++;
            if (!source->ContainsCoord(Pos()))
            {
                datum = nullptr;
            }
        }
        return *this;
    }

    template <typename T>
    inline typename Mat2D<T>::IteratorBase Mat2D<T>::IteratorBase::operator++(int)
    {
        IteratorBase iter = *this;
        ++(*this);
        return iter;
    }

    template <typename T>
    inline bool Mat2D<T>::IteratorBase::operator==(const IteratorBase& iter)
    {
        return datum == iter.datum;
    }

    template <typename T>
    inline bool Mat2D<T>::IteratorBase::operator!=(const IteratorBase& iter)
    {
        return !(*this == iter); 
    }

    //          //
    // Iterator //
    //          //

    template <typename T>
    inline Mat2D<T>::Iterator::Iterator(Mat2D<T>* source, T* datum)
        : IteratorBase(source, datum)
    {}

    template <typename T>
    inline T& Mat2D<T>::Iterator::operator*() const
    {
        return *this->datum;
    }

    template <typename T>
    inline typename Mat2D<T>::Iterator& Mat2D<T>::Iterator::operator++()
    {
        ++(*static_cast<IteratorBase*>(this));
        return *this;
    }

    template <typename T>
    inline typename Mat2D<T>::Iterator Mat2D<T>::Iterator::operator++(int)
    {
        Iterator iter = *this;
        ++(*static_cast<IteratorBase*>(this));
        return iter;
    }

    //               //
    // ConstIterator //
    //               //

    template <typename T>
    inline Mat2D<T>::ConstIterator::ConstIterator(const Mat2D<T>* source, const T* datum)
        : IteratorBase(const_cast<Mat2D<T>*>(source), const_cast<T*>(datum))
    {}

    template <typename T>
    inline const T& Mat2D<T>::ConstIterator::operator*() const
    {
        return *this->datum;
    }

    template <typename T>
    inline typename Mat2D<T>::ConstIterator& Mat2D<T>::ConstIterator::operator++()
    {
        ++(*static_cast<IteratorBase*>(this));
        return *this;
    }

    template <typename T>
    inline typename Mat2D<T>::ConstIterator Mat2D<T>::ConstIterator::operator++(int)
    {
        ConstIterator iter = *this;
        ++(*static_cast<IteratorBase*>(this));
        return iter;
    }

    //         //
    // Indices //
    //         //

    template <typename T>
    inline Indices::Indices(const Mat2D<T>& source, bool (*func)(T, T), T val)
        : Mat2D(source, [=](T sval) { return func(sval, val); })
    {}

    inline Indices Indices::operator&& (const Indices& rhs) const
    {
        Indices ret(rhs);
        ret.ApplySample(*this, [](bool b1, bool b2){
            return b1 && b2;
        });
        return ret;
    }

    inline Indices Indices::operator|| (const Indices& rhs) const
    {
        Indices ret(rhs);
        ret.ApplySample(*this, [](bool b1, bool b2){
            return b1 || b2;
        });
        return ret;
    }

    inline Indices Indices::operator^ (const Indices& rhs) const
    {
        Indices ret(rhs);
        ret.ApplySample(*this, [](bool b1, bool b2){
            return b1 != b2;
        });
        return ret;
    }

    inline Indices Indices::operator&& (Indices&& rhs) const
    {
        Indices ret(std::move(rhs));
        ret.ApplySample(*this, [](bool b1, bool b2){
            return b1 && b2;
        });
        return ret;
    }

    inline Indices Indices::operator|| (Indices&& rhs) const
    {
        Indices ret(std::move(rhs));
        ret.ApplySample(*this, [](bool b1, bool b2){
            return b1 || b2;
        });
        return ret;
    }

    inline Indices Indices::operator^ (Indices&& rhs) const
    {
        Indices ret(std::move(rhs));
        ret.ApplySample(*this, [](bool b1, bool b2){
            return b1 != b2;
        });
        return ret;
    }

    template <typename T>
    inline bool Indices::EQ(T val1, T val2)
    {
        return val1 == val2;
    }

    template <typename T>
    inline bool Indices::NE(T val1, T val2)
    {
        return val1 != val2;
    }

    template <typename T>
    inline bool Indices::LT(T val1, T val2)
    {
        return val1 < val2;
    }

    template <typename T>
    inline bool Indices::GT(T val1, T val2)
    {
        return val1 > val2;
    }

    template <typename T>
    inline bool Indices::LE(T val1, T val2)
    {
        return val1 <= val2;
    }

    template <typename T>
    inline bool Indices::GE(T val1, T val2)
    {
        return val1 >= val2;
    }

    //                               //
    // IndicesRefBase Implementation //
    //                               //

    template <typename T>
    IndicesRefBase<T>::IndicesRefBase(const Indices& indices, Mat2D<T>& mat)
        : indices(indices)
        , ref(mat)
    {}

    template <typename T>
    IndicesRefBase<T>::IndicesRefBase(Indices&& indices, Mat2D<T>& mat)
        : indices(std::move(indices))
        , ref(mat)
    {}

    //                           //
    // IndicesRef Implementation //
    //                           //

    template <typename T>
    IndicesRef<T>::IndicesRef(const Indices& indices, Mat2D<T>& mat)
        : IndicesRefBase<T>(indices, mat)
    {}

    template <typename T>
    IndicesRef<T>::IndicesRef(Indices&& indices, Mat2D<T>& mat)
        : IndicesRefBase<T>(std::move(indices), mat)
    {}

    template <typename T>
    inline void IndicesRef<T>::operator= (T val)
    {
        IndicesRefBase<T>::ref.ApplySample(
            IndicesRefBase<T>::indices,
            [=](T cur, bool cond){
                return (cond) ? val : cur;
            }
        );
    }

    template <typename T>
    inline void IndicesRef<T>::operator+= (T val)
    {
        IndicesRefBase<T>::ref.ApplySample(
            IndicesRefBase<T>::indices,
            [=](T cur, bool cond){
                return (cond) ? cur + val : cur;
            }
        );
    }

    template <typename T>
    inline void IndicesRef<T>::operator-= (T val)
    {
        IndicesRefBase<T>::ref.ApplySample(
            IndicesRefBase<T>::indices,
            [=](T cur, bool cond){
                return (cond) ? cur - val : cur;
            }
        );
    }

    template <typename T>
    inline void IndicesRef<T>::operator*= (T val)
    {
        IndicesRefBase<T>::ref.ApplySample(
            IndicesRefBase<T>::indices,
            [=](T cur, bool cond){
                return (cond) ? cur * val : cur;
            }
        );
    }

    template <typename T>
    inline void IndicesRef<T>::operator/= (T val)
    {
        IndicesRefBase<T>::ref.ApplySample(
            IndicesRefBase<T>::indices,
            [=](T cur, bool cond){
                return (cond) ? cur / val : cur;
            }
        );
    }

    template <typename T>
    inline void IndicesRef<T>::operator= (const Mat2D<T>& mat)
    {
        IndicesRefBase<T>::ref.ApplySample(
            IndicesRefBase<T>::indices,
            [=](T cur, bool cond, int x, int y){
                return (cond) ? mat(x, y) : cur;
            }
        );
    }

    template <typename T>
    inline void IndicesRef<T>::operator+= (const Mat2D<T>& mat)
    {
        IndicesRefBase<T>::ref.ApplySample(
            IndicesRefBase<T>::indices,
            [=](T cur, bool cond, int x, int y){
                return (cond) ? cur + mat(x, y) : cur;
            }
        );
    }

    template <typename T>
    inline void IndicesRef<T>::operator-= (const Mat2D<T>& mat)
    {
        IndicesRefBase<T>::ref.ApplySample(
            IndicesRefBase<T>::indices,
            [=](T cur, bool cond, int x, int y){
                return (cond) ? cur - mat(x, y) : cur;
            }
        );
    }

    template <typename T>
    inline void IndicesRef<T>::operator*= (const Mat2D<T>& mat)
    {
        IndicesRefBase<T>::ref.ApplySample(
            IndicesRefBase<T>::indices,
            [=](T cur, bool cond, int x, int y){
                return (cond) ? cur * mat(x, y) : cur;
            }
        );
    }

    template <typename T>
    inline void IndicesRef<T>::operator/= (const Mat2D<T>& mat)
    {
        IndicesRefBase<T>::ref.ApplySample(
            IndicesRefBase<T>::indices,
            [=](T cur, bool cond, int x, int y){
                return (cond) ? cur / mat(x, y) : cur;
            }
        );
    }

    //                                //
    // IndicesConstRef Implementation //
    //                                //

    template <typename T>
    IndicesConstRef<T>::IndicesConstRef(const Indices& indices, Mat2D<T>& mat)
        : IndicesRefBase<T>(indices, mat)
    {}

    template <typename T>
    IndicesConstRef<T>::IndicesConstRef(Indices&& indices, Mat2D<T>& mat)
        : IndicesRefBase<T>(std::move(indices), mat)
    {}

} // namespace zmath
