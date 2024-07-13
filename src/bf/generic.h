#pragma once
#include <cstdint>
#include <string>

enum ClassID {
    CID_BFSoldierTemplate = 0x9493,
    CID_ProjectileTemplate = 0x9495,
};

enum InterfaceID {
    IID_IBase = 1,
    IID_IPlayerControlObjectTemplate = 0xc4c4,
    IID_IPlayerControlObject = 0xc4c5,
    IID_ITextureHandler = 500009,
};

class IBase {
public:
    virtual void addRef() = 0;
    virtual void release() = 0;
    virtual IBase* queryInterface(uint32_t) const = 0;
};

class IConsoleSaveable : public IBase {
public:
    virtual bool makeScript(void* stream) = 0; // IStream*
};

class IObject;
class BitStream;

const float BF_FLT_EPSILON = 1.1920929e-7;

struct Vec2 {
    float x, y;
};

struct Quat {
    float x, y, z, w;
};

template <class T>
struct BaseVector3 {
    T x, y, z;
    BaseVector3() : x(0), y(0), z(0) {};
    BaseVector3(T x, T y, T z) : x(x), y(y), z(z) {};
    BaseVector3 operator+(const BaseVector3& r) const { return BaseVector3(x + r.x, y + r.y, z + r.z); };
    BaseVector3 operator-(const BaseVector3& r) const { return BaseVector3(x - r.x, y - r.y, z - r.z); };
    T length() const { return sqrt(lengthSquare()); };
    T lengthSquare() const { return x * x + y * y + z * z; };
};
typedef BaseVector3<float> Vec3;
typedef BaseVector3<float> Pos3;

template <class T>
struct BaseVector4 : public BaseVector3<T> {
    float w;
    BaseVector4& operator= (const BaseVector3<T>& right) { this->x = right.x; this->y = right.y; this->z = right.z; w = 0; return *this; };
};

template <class T>
struct BaseMatrix4 {
    BaseVector4<T> a;
    BaseVector4<T> b;
    BaseVector4<T> c;
    BaseVector4<T> position;
};

typedef BaseMatrix4<float> Mat4;

struct PlayerInput {
    float controls[55];
    uint64_t mask;
    int unk1;
    int unk2;
};

static_assert(sizeof(PlayerInput) == 0xF0);

class Game : public IBase {
public:
    // missing virtual methods here

    void addPlayerInput_orig(int playerid, PlayerInput* input) noexcept;
    void addPlayerInput_hook(int playerid, PlayerInput* input);
};


class BFPlayer : public IBase {

public:
    virtual int GetClassID();
    virtual ~BFPlayer();
    virtual void setName(bfs::string const& name);
    virtual bfs::string const& getName() const;
    virtual void setFlags(void*);
    virtual bool testFlags(void*) const;
    virtual void handleUpdate(float tDelta, unsigned int);
    virtual int getUpdateFrequencyType() const;
    virtual void setUpdateFrequencyType(int);
    virtual void handleInput(PlayerInput* input, float tDelta);
    virtual void* getCamera() const;
    virtual IObject* getVehicle() const;
    virtual bool setVehicle(IObject* vehicle, int inputid);
    virtual int getInputId() const;
    virtual int getId() const;
    virtual void setId(int id);
    virtual void setIsRemote();
    virtual bool getIsRemote() const;
    virtual void setIsAIPlayer();
    virtual bool getIsAIPlayer() const;
    virtual void setCamera(IObject* camera);
    // these belong in PlayerManager
    static BFPlayer* __stdcall getFromID(int id);
    static BFPlayer* getLocal();
    static bfs::list<BFPlayer*>* getPlayers();

    int getTeam() const { return *(int*)((intptr_t)this + 0xAC); };
};

template<int B>
struct Placeholder {
    uint8_t b[B];
};

// For casting non-virtual method pointers to void*, for hook_function
// example: hook_function(..., method_to_voidptr(&class::method);
// This is needed because C++ doesn't let you cast method pointers, but probably
// there is a better solution
// See also:
//  https://stackoverflow.com/questions/1307278/casting-between-void-and-a-pointer-to-member-function
//  https://stackoverflow.com/questions/1207106/calling-base-class-definition-of-virtual-member-function-with-function-pointer
__declspec(naked) inline void* method_to_voidptr(...) {
    _asm mov eax, [esp + 4]
    _asm ret
}


template<class Tk, class Tv/*, class Tcompare*/>
class Hash {
public:
    class Node {
        Node* next;
        Tk key;
        Tv value;
    };
    Node* buckets;
    size_t numBuckets;
    uint32_t unknown;
};

enum SeekOrigin {
    SO_BEGIN = 0,
    SO_CURRENT = 1,
    SO_END = 2,
};

// This is IStream in the game, but there are no namespaces here and it
// colldides some windows stuff.
class IBFStream : public IBase {
public:
    virtual size_t read(void* data, size_t bytes) = 0;
    virtual size_t write(const void* data, size_t bytes) = 0;
    virtual bool skip(size_t bytes) = 0;
    virtual size_t getAvailableBytes() = 0;
    virtual IBFStream* clone() = 0;
    virtual bool canRead() = 0;
    virtual bool canWrite() = 0;
    virtual bool seek(SeekOrigin origin, int) = 0;
    virtual size_t getPosition() = 0;
    virtual size_t getSize() = 0;
};

uint32_t __fastcall calcStringHashValueNoCase(const bfs::string& str);

// MD5 hashes data, then fills outputHexString with 32 hexadecimal characters. A null
// terminator is also appended, outputHexString must be atleast 33 bytes.
void __fastcall MD5Digest(const void* data, unsigned int length, char* outputHexString);

// Calls Locale::getAnsiLocale
void __stdcall getAnsiLocale(bfs::string& out, const bfs::string& key);

// Calls Locale::getWideLocale
void __stdcall getWideLocale(bfs::wstring& out, const bfs::string& key);

Mat4& __fastcall setRotation(Mat4& m, const Vec3& rotation);



void generic_hook_init();
