#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <string>
#include <sstream>
#include <tuple>
#include <memory>
#include <optional>
#include <functional>

template<typename ... StringElements>
class StringBuilder {
    std::tuple<StringElements...> elements;
    std::wstringstream wss;
public:
    StringBuilder(StringElements&&... elements) : elements(elements...) {

    }
    template<std::size_t i=0>
    void iterate() {
        wss << std::get<i>(elements);
        if constexpr (i < sizeof...(StringElements) - 1) {
            iterate<i+1>();
        }
    }
    std::wstring operator()() && {
        iterate();
        return wss.str();
    }
};

template<typename Value>
class lazyValue {
    std::optional<Value> value;
    std::function<Value()> ctor;
public:
    lazyValue(const lazyValue& other);
    lazyValue(const Value& v);
    lazyValue(Value&& v);
    lazyValue(const std::function<Value()>& ctor);

    Value get();
    Value get() const;
};

template<typename Value> lazyValue<Value>::lazyValue(const lazyValue& other) : value(other.value), ctor(other.ctor) {}
template<typename Value> lazyValue<Value>::lazyValue(const Value& v) : value(v) {}
template<typename Value> lazyValue<Value>::lazyValue(Value&& v) : value(v) {}
template<typename Value> lazyValue<Value>::lazyValue(const std::function<Value()>& ctor) : value(), ctor(ctor) {}

template<typename Value> Value lazyValue<Value>::get() const{
    if (value) {
        return *value;
    } else {
        return ctor();
    }
}

template<typename Value> Value lazyValue<Value>::get() {
    if (value) {
        return *value;
    } else {
        value = ctor();
        return *value;
    }
}

template<typename ... StringElements>
std::wstring concat(StringElements&&... elements) {
    return StringBuilder<StringElements...>{std::forward<StringElements>(elements)...}();
}

inline std::string to_ascii_string(const std::wstring& wstr) {
    return std::string(wstr.begin(), wstr.end());
}

template<typename Class, typename ... Elements>
std::unique_ptr<Class> make(Elements&&... elements) {
    return std::make_unique<Class>( std::move(elements)... );
}

class Position;
std::wstring error_marker(const Position& pos);

#endif
