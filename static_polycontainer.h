template<typename T, typename... Ts>
concept same_as_any = ( ... || std::same_as<std::remove_cvref_t<T>, Ts> );

template<typename F, typename... Ts>
concept invocable_for_all = (std::invocable<F, Ts&> && ...);

template<typename F, typename... Ts>
concept invocable_for_all_const = (std::invocable<F, const Ts&> && ...);

template<typename... Ts>
struct static_polycontainer 
{
private:
    std::tuple<std::vector<Ts>...> m_containers;

public:
    template<typename T>
    requires same_as_any<T,Ts...>
    void add(T&& obj) noexcept(noexcept(std::get<std::vector<std::remove_cvref_t<T>>>(m_containers).push_back(std::forward<T>(obj))))
    {
        using DecayedT = std::remove_cvref_t<T>;

        std::get<std::vector<DecayedT>>( m_containers ).push_back(std::forward<T>(obj));
    }

    template<typename T, typename... Args>
    requires same_as_any<T,Ts...> &&
             std::constructible_from<std::remove_cvref_t<T>, Args...>
    std::remove_cvref_t<T>& emplace(Args&&... args) noexcept(noexcept(std::get<std::vector<std::remove_cvref_t<T>>>(m_containers).emplace_back(std::forward<Args>(args)...)))
    {
        using DecayedT = std::remove_cvref_t<T>;

        auto& container = std::get<std::vector<DecayedT>>(m_containers);
        return container.emplace_back(std::forward<Args>(args)...);
    }

    template<typename T>
    requires same_as_any<T, Ts...>
    const std::vector<T>& get() const noexcept(noexcept(std::get<std::vector<T>>(m_containers))) 
    {
        return std::get<std::vector<T>>(m_containers);
    }

    template<typename T>
    requires same_as_any<T, Ts...>
    size_t size() const noexcept(noexcept(std::get<std::vector<T>>(m_containers).size()))
    {
        return std::get<std::vector<T>>(m_containers).size();
    }

    size_t size() const noexcept((noexcept(std::get<std::vector<Ts>>(m_containers).size()) && ...))
    {
        return (std::get<std::vector<Ts>>(m_containers).size() + ...);
    }

    template<typename T>
    requires same_as_any<T, Ts...>
    void reserve_for(const std::size_t s) noexcept(noexcept(std::get<std::vector<std::remove_cvref_t<T>>>(m_containers).reserve(s))) 
    {
        using DecayedT = std::remove_cvref_t<T>;

        std::get<std::vector<DecayedT>>(m_containers).reserve(s);
    }

    void reserve_all(const std::size_t s) 
    {
        ( reserve_for<Ts>(s), ... );
    }

    template<typename T>
    requires same_as_any<T, Ts...>
    bool empty() const noexcept(noexcept(std::get<std::vector<T>>(m_containers).empty()))
    {
        return std::get<std::vector<T>>(m_containers).empty();
    }

    bool empty() const noexcept((noexcept(std::get<std::vector<Ts>>(m_containers).empty()) && ...))
    {
        return (std::get<std::vector<Ts>>(m_containers).empty() && ...);
    }

    template<typename T>
    requires same_as_any<T, Ts...>
    void clear() noexcept(noexcept(std::get<std::vector<T>>(m_containers).clear()))
    {
        std::get<std::vector<T>>(m_containers).clear();
    }

    void clear_all() noexcept((noexcept(std::get<std::vector<Ts>>(m_containers).clear()) && ...))
    {
        ( std::get<std::vector<Ts>>(m_containers).clear(), ... );
    }

    // pre C++23
    // template<typename T, typename F>
    // requires same_as_any<T, Ts...> && std::invocable<F, T&> && (!std::invocable<F, T>)
    // void for_object(F&& func)
    // {
    //     for( T& i : std::get<std::vector<T>>(m_containers) )
    //     {
    //         std::invoke(func, i);
    //     }
    // }

    // template<typename T, typename F>
    // requires same_as_any<T, Ts...> && std::invocable<F, T>
    // void for_object(F&& func) const
    // {
    //     for( T i : std::get<std::vector<T>>(m_containers) )
    //     {
    //         std::invoke(func, i);
    //     }
    // }

    // template<typename T, typename F>
    // requires same_as_any<T, Ts...> && std::invocable<F, T>
    // void for_object(F&& func)
    // {
    //     for( T i : std::get<std::vector<T>>(m_containers) )
    //     {
    //         std::invoke(func, i);
    //     }
    // }

    // template<typename T, typename F>
    // requires same_as_any<T, Ts...> && std::invocable<F, const T&>
    // void for_object(F&& func) const
    // {
    //     for( const T& i : std::get<std::vector<T>>(m_containers) ) 
    //     {
    //         std::invoke(func, i);
    //     }
    // }

    template<typename T, typename F>
    requires same_as_any<T, Ts...> && (!std::invocable<F, T>)
    void for_object(this auto&& self, F&& func)
    {
        auto& container = std::get<std::vector<T>>(std::forward_like<decltype(self)>(self.m_containers));
        
        for (auto& i : container)
        {
            std::invoke(func, i);
        }
    }

    template<typename T, typename F>
    requires same_as_any<T, Ts...> && std::invocable<F, T>
    void for_object(this auto&& self, F&& func)
    {
        auto& container = std::get<std::vector<T>>(self.m_containers);
        
        for (T i : container)
        {
            std::invoke(func, i);
        }
    }

    // pre C++23
    // template<typename F>
    // requires invocable_for_all<F, Ts...>
    // void for_all(F&& func)
    // {
    //     ( for_object<Ts>(func), ... );
    // }

    // template<typename F>
    // requires invocable_for_all_const<F, Ts...>
    // void for_all(F&& func) const
    // {
    //     ( for_object<Ts>(func), ... );
    // }

    template<typename F>
    void for_all(this auto&& self, F&& func)
    {
        ( std::forward_like<decltype(self)>(self).template for_object<Ts>(func), ... );
    }
};