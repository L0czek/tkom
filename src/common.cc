#include "common.hpp"

#include "source.hpp"

std::wstring error_marker(const Position& pos) {
    std::wstring ret = L"\033[1;32m";
    for (std::size_t i=1; i < pos.column_number; ++i) {
        ret += L"-";
    }
    ret += L"\033[1;31m^\033[0m";
    return ret;
}


