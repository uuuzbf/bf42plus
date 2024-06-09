#pragma once
#include <cstdint>
#include <string>

// This file contains simplified implementations of the C++ stl library used by BF1942 (std::*)
// These classes can be used when interfacing with native game functions.

namespace bfs
{
    void* operator_new(size_t);
    void operator_delete(void*);

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

        string(std::string const& str) : string(str.data(), str.size()) {};

        string(const std::string_view& view) : string(view.data(), view.size()) {};

        ~string();

        bool empty() const { return length == 0; };
        size_t size() const { return length; };
        const char* c_str() const;

        void clear();

        string& replace(size_t pos, size_t len, string const& str);

        string& append(const char* str);

        int compare(const char* str) const;

        string& operator=(std::string const& str) { return replace(0, size(), string(str)); };
        string& operator=(const char* str) { return replace(0, size(), string(str)); };
        bool operator==(const char* str) const { return compare(str) == 0; };
        operator std::string() const { return std::string(c_str(), size()); };
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

    template <class Tv>
    class list {
    public:
        struct Node;
        friend struct Node;
        struct Node {
            Node* next; // head: first item
            Node* prev; // head: last item
            Tv value;
        };
        uint32_t allocator = 0;
        Node* head;
        size_t _size = 0;
    public:
        class iterator;
        friend class iterator;
        class iterator {
            Node* node;
        public:
            iterator() {};
            iterator(Node* n) : node(n) {}
            Tv& operator*() { return node->value; };
            iterator& operator++() { node = node->next; return *this; };
            iterator& operator++(int) { iterator tmp = *this; ++*this; return tmp; };
            iterator& operator--() { node = node->prev; return *this; };
            iterator& operator--(int) { iterator tmp = *this; --*this; return tmp; };
            bool operator==(const iterator& right) { return this->node == right.node; };
            bool operator!=(const iterator& right) { return this->node != right.node; };
            Node* getnode() const { return node; };
        };
        iterator erase(iterator first, iterator last) {
            iterator it = first;
            while (it != last) {
                iterator it_copy = it;
                it++;
                if (it.getnode() != head) {
                    --this->_size;
                    it_copy.getnode()->prev->next = it_copy.getnode()->next;
                    it_copy.getnode()->next->prev = it_copy.getnode()->prev;
                    operator_delete(it_copy.getnode());
                }
            }
            return it;
        }
        list() {
            head = reinterpret_cast<Node*>(operator_new(sizeof(Node)));
            head->next = head;
            head->prev = head;
        };
        ~list() {
            clear();
            operator_delete(head);
            head = 0;
            _size = 0;
        };

        void push_back(const Tv& value) {
            Node* newnode = reinterpret_cast<Node*>(operator_new(sizeof(Node)));
            newnode->value = value;
            newnode->next = head;
            newnode->prev = head->prev;
            newnode->prev->next = newnode;
            head->prev = newnode;
        };

        void clear() { erase(begin(), end()); };

        iterator begin() const { return iterator(head ? head->next : 0); };
        iterator end() const { return iterator(head); };

        bool empty() const { return _size == 0; };
        size_t size() const { return _size; };

    };


    template<class Kt, class Vt>
    class map {
    public:
        enum Color : uint8_t { Red, Black };
        struct Node {
            Node* left;
            Node* parent;
            Node* right;
            std::pair<Kt, Vt> pair;
            uint8_t _pad;
            Color color;
            // this is iterator::increment(), but there are no iterators yet
            Node* next() {
                if (color == Red) {
                    auto node = right;
                    if (node->color == Black) {
                        auto node2 = this;
                        for (node = parent; node->color == Red; node = node->parent) {
                            if (node2 != node->right) {
                                break;
                            }
                            node2 = node;
                        }
                    }
                    else {
                        for (auto node2 = node->left; node2->color == Red; node2 = node2->left) {
                            node = node2;
                        }
                    }
                    return node;
                }
                return this;
            }
        };
        uint32_t allocator;
        Node* head;
        size_t size;

        bool empty() const { return head->left == head; };
    };

    template<class Tv>
    class vector {
    public:
        class iterator {
            Tv* ptr;
        public:
            Tv& operator*() { return *ptr; };
            iterator& operator++() { ++ptr; return *this; };
            iterator& operator++(int) { iterator tmp = *this; ++*this; return tmp; };
            iterator& operator--() { --ptr; return *this; };
            iterator& operator--(int) { iterator tmp = *this; --*this; return tmp; };
            size_t operator-(const iterator& right) const { return ptr - right.ptr; };
            bool operator==(const iterator& right) const { return ptr == right.ptr; };

        };
        iterator begin() { return _first; };
        iterator end() { return _last; };
        size_t size() { _first == 0 ? 0 : _last - _first; };
        Tv& at(size_t index) { assert(index < size()); return *(_first + index); };
        const Tv& operator[](size_t index) const { return *(_first + index); };
        Tv& operator[](size_t index) { return *(_first + index); };
    protected:
        uint32_t allocator;
        iterator _first;
        iterator _last;
        iterator _end;
    };
}
