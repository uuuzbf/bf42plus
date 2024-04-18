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
struct PlayerInput;
class IObject;

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
    virtual void* getVehicle() const;
    virtual bool setVehicle(IObject* vehicle, int inputid);
    virtual int getInputId() const;
    virtual int getId() const;
    virtual void setId(int id);
    virtual void setIsRemote();
    virtual bool getIsRemote() const;
    virtual void setIsAIPlayer();
    virtual bool getIsAIPlayer() const;
    virtual void setCamera(IObject* camera);
    static BFPlayer* __stdcall getFromID(int id);
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
