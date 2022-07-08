#include <type_traits>
#include <utility>
#include <cstddef>
#include <tuple>
#include <functional>

namespace zmath
{
    //                      //
    // Get zipped iterators //
    //                      //

    template <typename... Args>
    auto get_zipped_begins(Args&&... args)
    {
        return std::make_tuple(args.begin()...);
    }

    template <typename... Args>
    auto get_zipped_ends(Args&&... args)
    {
        return std::make_tuple(args.end()...);
    }

    template <typename... Args>
    using zip_iter_t = std::invoke_result_t<decltype(get_zipped_begins<Args...>), Args...>;

    template <typename... Args>
    constexpr bool are_iterable_v = std::is_invocable_v<decltype(get_zipped_begins<Args...>), Args...>;

    template <typename T, typename... Args>
    constexpr bool first_is_integral_v = std::is_integral_v<std::remove_reference_t<T>>;

    template <typename... Args>
    auto _get_zipped_dereferenced_begins(Args&&... args)
    {
        return std::make_tuple(std::ref(*args.begin())...);
    }

    template <typename... Args>
    using zip_deref_t = std::invoke_result_t<decltype(_get_zipped_dereferenced_begins<Args...>), Args...>;

    template <typename... Args, size_t... Idx>
    zip_deref_t<Args...> dereference_zipped_iterators(const zip_iter_t<Args...>& iters, std::index_sequence<Idx...>)
    {
        return zip_deref_t<Args...>(std::ref(*std::get<Idx>(iters))...);
    }

    //         //
    // Classes //
    //         //

    template <typename... Args>
    class Zip
    {
    private:
        class Iterator;
        typedef zip_iter_t<Args...> iter_t;

    public:
        Zip(Args&&... args);
        Iterator begin() const;
        Iterator end() const;

    private:
        iter_t iters_begin;
        iter_t iters_end;

        class Iterator
        {
        public:
            typedef std::index_sequence_for<Args...> idx_t;
            typedef zip_deref_t<Args...> ret_t;

            Iterator(iter_t iters);
            Iterator& operator++();
            Iterator operator++(int);
            ret_t operator*() const;
            
            // @return whether *all* iterators are equal between the left- and
            //         right-hand sides, according to operator==
            bool operator==(const Iterator& rhs) const;
            // @return whether *all* iterators are unequal between the left- and
            //         right-hand sides, according to operator!=
            bool operator!=(const Iterator& rhs) const;

        private:
            iter_t iters;

            template <size_t N=0>
            void increment();

            template <size_t... Idx>
            bool all_unequal(const Iterator& rhs, std::index_sequence<Idx...>) const;

        }; // class Zip::Iterator
    }; // class Zip

    template <typename... Args>
    class ZipFor
    {
    private:
        class Iterator;
        typedef zip_iter_t<Args...> iter_t;

    public:
        ZipFor(size_t size, Args&&... args);
        Iterator begin() const;
        Iterator end()   const;

    private:
        iter_t iters_begin;
        iter_t iters_end;
        size_t size;

        class Iterator
        {
        public:
            typedef std::index_sequence_for<Args...> idx_t;
            typedef zip_deref_t<Args...> ret_t;

            Iterator(iter_t iters, size_t idx);
            Iterator& operator++();
            Iterator operator++(int);
            ret_t operator*() const;

            bool operator==(const Iterator& rhs) const;
            bool operator!=(const Iterator& rhs) const;

        private:
            iter_t iters;
            size_t idx;

            template <size_t N=0>
            void increment();

        }; // class ZipFor::Iterator
    }; // class ZipFor

    //                    //
    // Zip Implementation //
    //                    //

    template <typename... Args>
    inline Zip<Args...>::Zip(Args&&... args)
        : iters_begin(get_zipped_begins(std::forward<Args>(args)...))
        , iters_end(get_zipped_ends(std::forward<Args>(args)...))
    {}

    template <typename... Args>
    inline typename Zip<Args...>::Iterator Zip<Args...>::begin() const
    {
        return Iterator(iters_begin);
    }
    
    template <typename... Args>
    inline typename Zip<Args...>::Iterator Zip<Args...>::end() const
    {
        return Iterator(iters_end);
    }

    //                              //
    // Zip::Iterator Implementation //
    //                              //

    template <typename... Args>
    Zip<Args...>::Iterator::Iterator(iter_t iters)
        : iters(iters)
    {}

    template <typename... Args>
    typename Zip<Args...>::Iterator& Zip<Args...>::Iterator::operator++()
    {
        increment();
        return *this;
    }

    template <typename... Args>
    typename Zip<Args...>::Iterator Zip<Args...>::Iterator::operator++(int)
    {
        Iterator cpy = *this;
        ++(*this);
        return cpy;
    }

    template <typename... Args>
    typename Zip<Args...>::Iterator::ret_t Zip<Args...>::Iterator::operator*() const
    {
        return dereference_zipped_iterators<Args...>(iters, idx_t{});
    }

    template <typename... Args>
    bool Zip<Args...>::Iterator::operator==(const Iterator& rhs) const
    {
        return all_equal(rhs, idx_t{});
    }

    template <typename... Args>
    bool Zip<Args...>::Iterator::operator!=(const Iterator& rhs) const
    {
        return all_unequal(rhs, idx_t{});
    }

    template <typename... Args>
    template <size_t N>
    void Zip<Args...>::Iterator::increment()
    {
        if constexpr (N < std::tuple_size_v<iter_t>)
        {
            std::get<N>(iters)++;
            increment<N+1>();
        }
    }

    template <typename... Args>
    template <size_t... Idx>
    bool Zip<Args...>::Iterator::all_unequal(const Iterator& rhs, std::index_sequence<Idx...>) const
    {
        return (... && (std::get<Idx>(iters) != std::get<Idx>(rhs.iters)));
    }

    //                       //
    // ZipFor Implementation //
    //                       //

    template <typename... Args>
    ZipFor<Args...>::ZipFor(size_t size, Args&&... args)
        : iters_begin(get_zipped_begins(std::forward<Args>(args)...))
        , iters_end(get_zipped_ends(std::forward<Args>(args)...))
        , size(size)
    {}

    template <typename... Args>
    typename ZipFor<Args...>::Iterator ZipFor<Args...>::begin() const
    {
        return Iterator(iters_begin, 0);
    }

    template <typename... Args>
    typename ZipFor<Args...>::Iterator ZipFor<Args...>::end() const
    {
        return Iterator(iters_end, size);
    }

    template <typename... Args>
    ZipFor<Args...>::Iterator::Iterator(iter_t iters, size_t idx)
        : iters(iters)
        , idx(idx)
    {}

    template <typename... Args>
    typename ZipFor<Args...>::Iterator& ZipFor<Args...>::Iterator::operator++()
    {
        idx++;
        increment();
        return *this;
    }

    template <typename... Args>
    typename ZipFor<Args...>::Iterator ZipFor<Args...>::Iterator::operator++(int)
    {
        Iterator cpy = *this;
        ++(*this);
        return cpy;
    }

    template <typename... Args>
    typename ZipFor<Args...>::Iterator::ret_t ZipFor<Args...>::Iterator::operator*() const
    {
        return dereference_zipped_iterators<Args...>(iters, idx_t{});
    }

    template <typename... Args>
    bool ZipFor<Args...>::Iterator::operator==(const Iterator& rhs) const
    {
        return idx == rhs.idx;
    }

    template <typename... Args>
    bool ZipFor<Args...>::Iterator::operator!=(const Iterator& rhs) const
    {
        return idx != rhs.idx;
    }

    template <typename...Args>
    template <size_t N>
    void ZipFor<Args...>::Iterator::increment()
    {
        if constexpr (N < std::tuple_size_v<iter_t>)
        {
            std::get<N>(iters)++;
            increment<N+1>();
        }
    }

    //                  //
    // Zipper Functions //
    //                  //

    template <typename... Args>
    auto zip_for(size_t size, Args&&... args)
    {
        return ZipFor<Args...>(size, std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto zip(Args&&... args)
    {
        if constexpr (first_is_integral_v<Args...>)
            return zip_for(std::forward<Args>(args)...);
        else
            return Zip<Args...>(std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    void zip_exec(Func f, Args&&... args)
    {
        for (auto z : zip(std::forward<Args>(args)...))
            std::apply(f, z);
    }

} // namespace zmath
