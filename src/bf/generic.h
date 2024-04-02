#pragma once
#include <cstdint>
#include <string>

class IBase {
public:
    virtual void addRef() = 0;
    virtual void release() = 0;
    virtual IBase* queryInterface(uint32_t) = 0;
};

class BitStream;

template<int B>
struct Placeholder {
    uint8_t b[B];
};

void chatMessage(std::string message, bool status = false);
