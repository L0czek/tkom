#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <string>
#include <sstream>
#include <tuple>

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

#endif
