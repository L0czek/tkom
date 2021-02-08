#include "source.hpp"

#include <codecvt>
#include "locale.hpp"
#include "common.hpp"

Source::Source() {
    current_position.stream_position = 0;
    current_position.line_number = 1;
    current_position.column_number = 0;
    line_position[1] = current_position;
}

Source::~Source() {

}

std::wstring to_wstring(const Position& position) {
    return concat(
            L"line: ", 
            std::to_wstring(position.line_number), 
            L"; stream: ", 
            std::to_wstring(position.stream_position), 
            L"; column: ",
            std::to_wstring(position.column_number),
            L"; "
        );
}

const Position& Source::get_position() const noexcept {
    return current_position;
}

void Source::update_position(wchar_t ch) {
    if (ch == L'\n') {
        current_position.line_number++;
        current_position.column_number = 0;
        line_position[current_position.line_number] = current_position;
    }
    current_position.column_number++;
    current_position.stream_position++;
}

std::wstring Source::get_lines(std::size_t from, std::size_t to)  {
    Position start = line_position.at(from);
    Position end;
    if (line_position.find(to) != line_position.end()) {
        end = line_position.at(to);
    } else {
        end = current_position;
    }
    return input_between(start, end);
}

FileSource::FileSource(const std::string& path) : file(path, std::ios::in) {
    file.imbue(std::locale(Locale::get().locale(), new std::codecvt_utf8<wchar_t>{}));
    if (!file.good()) {
        Source::report_error("IO error when trying to access file");
    }
}

FileSource::~FileSource() {
    file.close();
}

void Source::report_error(const std::string& msg) {
    throw SourceException {
        msg
    };
}

std::optional<wchar_t> FileSource::next() noexcept {
    wchar_t ch;
    if (file.get(ch)) {
        Source::update_position(ch);
        return ch;
    } else {
        return {};
    }
}

std::wstring FileSource::input_between(const Position& start, const Position& end) {
    const auto st = start.stream_position ;
    const auto en = end.stream_position ;

    if (st >= en) {
        return L"";
    }

    auto position_backup = file.tellg();
    file.seekg(st);
    std::wstring source(en - st, L'\x00');
    file.read(source.data(), en - st);
    file.seekg(position_backup);
    return source;
}

StdInSource::StdInSource() : source_stream(std::wcin.rdbuf()) {
    std::wcin.rdbuf(nullptr);
    source_stream.imbue(std::locale(Locale::get().locale(), new std::codecvt_utf8<wchar_t>{}));
}

StdInSource::~StdInSource() {
    std::wcin.rdbuf(source_stream.rdbuf());
    source_stream.rdbuf(nullptr);
}

std::optional<wchar_t> StdInSource::next() noexcept {
    wchar_t ch;
    if (source_stream.get(ch) && std::char_traits<wchar_t>::not_eof(ch)) {
        Source::update_position(ch);
        source_code.push_back(ch);
        return ch;
    } else {
        return {};
    }
}

std::wstring StdInSource::input_between(const Position& start, const Position& end) {
    const auto st = start.stream_position ;
    const auto en = end.stream_position ;

    if (st >= en) {
        return L"";
    }

    return source_code.substr(st, en - st);
}

StringSource::StringSource(const std::wstring& source) {
    source_stream.imbue(std::locale(Locale::get().locale(), new std::codecvt_utf8<wchar_t>{}));
    source_stream << source;
}

StringSource::~StringSource() {

}

std::optional<wchar_t> StringSource::next() noexcept {
    wchar_t ch;
    if (source_stream.get(ch) && std::char_traits<wchar_t>::not_eof(ch)) {
        Source::update_position(ch);
        return ch;
    } else {
        return {};
    }
}

std::wstring StringSource::input_between(const Position& start, const Position& end) {
    const auto st = start.stream_position ;
    const auto en = end.stream_position ;

    if (st >= en) {
        return L"";
    }

    auto position_backup = source_stream.tellg();
    source_stream.seekg(st);
    std::wstring source(en - st, L'\x00');
    source_stream.read(source.data(), en - st);
    source_stream.seekg(position_backup);
    return source;
}


std::unique_ptr<Source> Source::from_file(const std::string& path) {
    return std::make_unique<FileSource>(path);
}

std::unique_ptr<Source> Source::from_stdin() {
    return std::make_unique<StdInSource>();
}
 
std::unique_ptr<Source> Source::from_wstring(const std::wstring& str) {
    return std::make_unique<StringSource>(str);
}
