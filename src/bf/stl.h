#pragma once
#include <cstdint>
#include <string>

// bf std::*
namespace bfs
{
    //template<class Elem_>
    //class basic_string {
    class string {
    public:
        uint32_t unknown;
        char internal_buffer[16];
        size_t length;
        size_t buffersize;

        string(char const* s); // c string initializer
        string(char const* s, size_t count); // c string initializer with length
        string(string const&); // reference intializer
        string(string const&, size_t pos, size_t count); // reference initializer with pos and count
        string(size_t count, char c);
        string();

        string(std::string const& str) : string(str.data(), str.size()) {  };

        ~string();

        bool empty() const { return length == 0; };
        size_t size() const { return length; };
        const char* c_str() const;

        void clear();

        string& replace(size_t pos, size_t len, string const& str);

        string& operator=(std::string const& str) { return replace(0, size(), string(str)); };

    };

    //using string = basic_string<char>;
    //using wstring = basic_string<wchar_t>;
}
