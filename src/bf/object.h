#pragma once
#include <cstdint>



class ObjectTemplate;
class NetworkableBase;

class IObject : public IBase {
protected:
    uint32_t flags;
    uint8_t unknown8[52];
    int unknown3C;
    uint8_t unknown40[8];
    int gridid;
    ObjectTemplate* tmpl;
    IObject* parent;
    IObject* nextSibling;
    void* AIObject;
    void* geometry;
    void* physicsNode;
    void* responsePhysics;
    NetworkableBase* networkable;
public:
    virtual ~IObject() = 0;
    virtual void init() = 0;
    virtual void destroy() = 0;
    virtual void immediateDestroy() = 0;
    virtual bool isObjectDestroyed() = 0;
    virtual bool setComponent(int, IBase* component) = 0;
    virtual void* queryComponent(unsigned int, unsigned int) = 0;
    virtual void updateFlags(unsigned int setflags, unsigned int clearflags) = 0;
    virtual const bfs::string& getName() = 0;
    virtual void setName(const bfs::string& name) = 0;
    virtual const Pos3& getAbsolutePosition() = 0;
    virtual void setAbsolutePosition(const Pos3& position) = 0;
    virtual const Mat4& getAbsoluteTransformation() = 0;
    virtual void setAbsoluteTransformation(const Mat4& matrix) = 0;
    virtual float getBoundingRadius() = 0;
    virtual void handleVisualUpdate(float, float) = 0;
    virtual void handleFrameUpdate(float) = 0;
    virtual void handleUpdate(float, unsigned int) = 0;
    virtual void handleCollision(IObject*, const Vec3&, const Vec3&, const Vec3&, int, int) = 0;
    virtual void updateComponents() = 0;
    virtual int getUpdateFrequency() = 0;
    virtual int getUpdateFrequencyType() = 0;
    virtual int setUpdateFrequencyType(int) = 0; // UpdateFreqencyType
    virtual const Vec3& getRelativePosition() = 0;
    virtual void setRelativePosition(const Vec3& position) = 0;
    virtual const Mat4& getRelativeTransformation() = 0;
    virtual void setRelativeTransformation(const Mat4& matrix) = 0;
    virtual IObject* getChild() = 0;
    virtual void addChild(IObject*) = 0; // ICompositeObject*
    virtual void removeChild(IObject*) = 0; // ICompositeObject*
    virtual void addSibling(IObject*) = 0; // ICompositeObject*
    virtual void removeSibling(IObject*) = 0; // ICompositeObject*
    virtual void setParent(IObject*) = 0; // ICompositeObject*
    virtual void resetCachedRootParents() = 0;
    virtual void handlePlayerInput(BFPlayer*, const void* playerInput, float) = 0;
    virtual void handleMessage(int templateMessage, BFPlayer*) = 0; // TemplateMessage
    virtual char getInputId() = 0;
    virtual void setInputId(char) = 0;
    virtual char getChildId() = 0;
    virtual void setChildId(char) = 0;
    virtual void internalRelease() = 0;
    ObjectTemplate* getTemplate() const { return tmpl; };
    int getID() const { return gridid; };
    NetworkableBase* getNetworkable() const { return networkable; };
    IObject* getParent() const { return parent; };
    bool hasMobilePhysics() const;
};

static_assert(sizeof(IObject) == 0x6C);

class ObjectTemplate : public IBase, IConsoleSaveable {
protected:
    bfs::string name;
    bfs::string networkableInfo;
    int id;
    unsigned int flags;
    bfs::map<unsigned int, void*> components; // map<unsigned int, SmartPtr<IBase>>
    float cullRadiusScale;
public:
    virtual int getClassID() const = 0;
    virtual void setName(const bfs::string) = 0;
    virtual const bfs::string& getName() const = 0;
    virtual void setId(unsigned int) = 0;
    virtual unsigned int getId() const = 0;
    virtual unsigned int getFlags() const = 0;
    virtual void updateFlags(unsigned int set, unsigned int clear) = 0;
    virtual ObjectTemplate* queryImplementation(unsigned int) const = 0;
    virtual IObject* createObject() = 0;
    virtual void setComponent(unsigned int, IBase*) = 0;
    virtual IBase* queryComponent(unsigned int, unsigned int) = 0;
    virtual void preCache() = 0;
    virtual void setCullRadiusScale(float) = 0;
    virtual float getCullRadiusScale() const = 0;
    virtual ~ObjectTemplate() = 0;
    virtual void setNetworkableInfo(const bfs::string& networkableInfo) = 0;
    virtual const bfs::string& getNetworkableInfo() const = 0; 
};

static_assert(sizeof(ObjectTemplate) == 0x58);

struct BaseLineData;
struct NetworkInfo;

class NetworkableBase : public IBase {
    uint16_t networkID;
    NetworkInfo* networkInfo;
    uint8_t unkC;
    float basePriority;
    uint8_t unk14;
    void* pINetworkableObject;
    bool allocatedFromMemoryPool;
    uint8_t unk1D;
    uint32_t unk20;
    uint32_t unk24;
    int updateIndex;
    uint32_t unk2C;
    int bsStartPosition;
public:
    virtual bool init(void* object) = 0;
    virtual uint32_t getGhostStateMask() const = 0;
    virtual void updateStateMask(uint32_t mask) = 0;
    virtual bool getNetUpdate(BitStream& bs, BaseLineData* b, uint32_t, int32_t, bool) = 0;
    virtual void setNetUpdate(BitStream& bs, float time, bool, BaseLineData*, bool) = 0;
    virtual bool getBaseLine(BaseLineData& out) = 0;
    virtual int getMinimumBitSize() const = 0;
    virtual ~NetworkableBase() = 0;
    uint16_t getID() const { return networkID; };
};

static_assert(sizeof(NetworkableBase) == 0x34);

// This returns a map containing all projectiles which need a mine warning icon
// Move this into ObjectManager when there will be one.
bfs::map<unsigned int, IObject*> ObjectManager_getProjectileMap();

// This returns a map containing all SupplyDepots
// Move this into ObjectManager when there will be one.
bfs::map<unsigned int, IObject*> ObjectManager_getSupplyDepotMap();
