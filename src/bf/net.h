#pragma once
#include <stdint.h>
#include "generic.h"

enum BSMode {

};
struct BSPosition {
    uint32_t unknown1;
    uint32_t offset;
    uint32_t unknown2;
    uint32_t mode;
};

class BitStream {
public:
    BitStream(uint32_t);
    BitStream(uint8_t*, uint32_t);
    ~BitStream();
    void init(uint8_t*, uint32_t);
    bool set(uint8_t* data, uint32_t bytes);
    void clear();

    bool isEOF() const { return *(bool*)((uintptr_t)this + 0x20); };
    void resetError() {
        *(bool*)((uintptr_t)this + 0x20) = false; // eof
        *(int*)((uintptr_t)this + 0x24) = 0; // overrun
    };
    bool getAndResetError() {
        bool result = isEOF();
        if (result) resetError();
        return result;
    };

    void resetWritePosition();
    void setWritePosition(BSPosition& position);
    void getPosition(BSPosition& position);
    void setPosition(BSPosition& position);
    void setMode(BSMode);
    BSMode getMode();
    bool seekBits(SeekOrigin);
    void setByteSizeLimit(uint32_t bytes);
    uint32_t getByteSizeLimit();
    void setByteSize(uint32_t bytes);
    uint32_t getByteSize();
    uint32_t getBitSize();
    uint32_t getBitPosition();
    uint32_t getAvailableBits();

    bool beginWrite();
    bool endWrite();
    bool cancelWrite();

    bool writeBits(void const*, uint32_t bits);
    bool readBits(void* ptr, uint32_t bits);
    bool skipBits(uint32_t bits);

    void writeBool(bool);
    bool readBool();

    void writeUnsigned(uint32_t value, int bits);
    uint32_t readUnsigned(int bits);
    void writeUnsigned(uint32_t value, uint32_t min, uint32_t max);
    uint32_t readUnsigned(uint32_t min, uint32_t max);
    void writeSigned(int value, int bits);
    int readSigned(int bits);

    float readSignedFloat(int bits, float scale); // -scale .. scale
    void writeSignedFloat(float value, int bits, float scale); // -scale .. scale
    float readSignedFloat(int bits); // -1.0 .. 1.0
    float writeSignedFloat(float value, int bits); // -1.0 .. 1.0
    float readUnsignedFloat(int bits, float scale); // 0.0 .. scale
    void writeUnsignedFloat(float value, int bits, float scale); // 0.0 .. scale
    float readUnsignedFloat(int bits); // 0.0 .. 1.0
    void writeUnsignedFloat(float value, int bits); // 0.0 .. 1.0

    void shrinkSignedFloat(float value, int bits);
    float shrinkUnsignedFloat(float value, int bits);

    void writeFullVector(Vec3 const& vector);
    Vec3 readFullVector();

    void setCompressionVector(Vec3 const& vector);
    void resetCompressionVector();
    bool writeCompressedVector(Vec3 const& vector, float scale);
    Vec3 readCompressedVector(float scale);
    void writeCompressedVector(Vec3 const& vector, Vec3 const& base, float scale);
    Vec3 readCompressedVector(Vec3 const& base, float scale);
    void writeHighCompressedVector(Vec3 const& vector, Vec3 const& base, float scale);
    Vec3 readHighCompressedVector(Vec3 const& base, float scale);

    void writeNormalVector(Vec3 const& normal, int bits);
    Vec3 readNormalVector(int bits);

    Vec3 shrinkNormalVector(Vec3 const& normal, int bits);

    void writeUnitQuaternion(Quat const&, int bits);
    Quat readUnitQuaternion(int bits);
};
