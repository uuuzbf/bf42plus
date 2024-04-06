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
        uint32_t allocator;
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

    class wstring {
    private:
        static const uint32_t DUMB_OBJECT = 0xff748751;
        // dumb constructor, objects created with this can only be passed to functions expecting const ptrs/refs
        wstring(const wchar_t* data, const size_t length)
        {
            allocator = DUMB_OBJECT;
            *(reinterpret_cast<const wchar_t**>(&internal_buffer[0])) = data;
            this->length = length;
            this->buffersize = length < 8 ? 8 : length;
        };
    public:
        uint32_t allocator;
        wchar_t internal_buffer[8];
        size_t length;
        size_t buffersize;

        wstring(wstring const&); // reference initializer
        wstring(wchar_t const*); // c string initializer
        wstring();

        wstring(std::wstring const& str) : wstring(wstring(str.data(), str.length())) {};

        ~wstring();

        bool empty() const { return length == 0; };
        size_t size() const { return length; };
        const wchar_t* c_str() const;
        wchar_t* data() { return buffersize < 8 ? internal_buffer : *(reinterpret_cast<wchar_t**>(&internal_buffer[0])); };

        void resize(size_t length);
        wstring& replace(size_t pos, size_t length, wstring const& str);
        wstring& replace(size_t pos, size_t length, wchar_t const* str);
        wstring& append(wchar_t const* str);

        wstring& operator=(wstring const& str);
        wstring& operator+=(wchar_t const* str) { return append(str); };
    };

    //using string = basic_string<char>;
    //using wstring = basic_string<wchar_t>;
}
