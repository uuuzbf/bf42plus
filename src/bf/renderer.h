#pragma once
#include <cstdint>
#include "stl.h"
#include "generic.h"

enum PixelFormat {
    PF_RGB_888 = 0x1,
    PF_ARGB_8888 = 0x2,
    PF_XRGB_8888 = 0x3,
    PF_ARGB_4444 = 0x4,
    PF_ARGB_1555 = 0x5,
    PF_RGB_565 = 0x6,
    PF_RGB_555 = 0x7,
    PF_Alpha_8 = 0x8,
    PF_Lum_8 = 0x9,
    PF_Pal_8 = 0xA,
    PF_DXT1 = 0xC,
    PF_DXT2 = 0xD,
    PF_DXT3 = 0xE,
    PF_DXT4 = 0xF,
    PF_DXT5 = 0x10,
    PF_DEPTHBUFFER_D24S8 = 0x11,
    PF_DEPTHBUFFER_D16 = 0x12,
    PF_DEPTHBUFFER_D32 = 0x13,
};

struct D3DCOLOR;
struct DisplaySettings;
class WindowWin32;

// not sure yet where ITexture stops and ITexture2D starts
class ITexture : public IBase {
public:
    virtual int getInterfaceID() = 0;
    virtual PixelFormat getPixelFormat() = 0;
    virtual int getField40h() = 0;
    virtual int getField44h() = 0;
    virtual void setField44h(int) = 0;
    virtual int getMipmaps() = 0;
    virtual int DXTexBuf1__678300() = 0;
    virtual bool getFrameBuffer(void*, PixelFormat pixelFormat, int mipmapLevel, void** out, int) = 0;
    virtual bool DXTexBuf1__6791E0(int mipmapLevel) = 0;
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
    virtual int getBitdepth() = 0;
    virtual bool DXTexBuf1__6790C0(int, int, void*, int) = 0; // parameters unknown
    virtual bool DXTexBuf1__6783E0(int mipmapLevel) = 0;
};

class RendPCDX8 : public IBase {
public:
    static RendPCDX8* GetSingleton() {
        return *(RendPCDX8**)0x009A99D4;
    };
    virtual bool init(DisplaySettings*, WindowWin32*) = 0;
    virtual int close() = 0;
    virtual int sub_667B20() = 0;
    virtual int sub_667B30() = 0;
    virtual int f28() = 0;
    virtual int sub_667650() = 0;
    virtual int loadVertexShader() = 0;
    virtual int loadPixelShader() = 0;
    virtual int setCurrentDisplaySettings() = 0;
    virtual DisplaySettings* getCurrentDisplaySettings() = 0;
    virtual int sub_742710() = 0;
    virtual int sub_651490() = 0;
    virtual void setViewport(uint32_t, uint32_t, uint32_t, uint32_t) = 0;
    virtual void getViewport(uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height) = 0;
    virtual int sub_6799D0() = 0;
    virtual int sub_679A00() = 0;
    virtual void beginScene() = 0;
    virtual void endScene() = 0;
    virtual void clearDevice(int flags, D3DCOLOR color, float Z, int stencil) = 0;
    virtual int sub_6676C0() = 0;
    virtual int sub_667750() = 0;
    virtual void present() = 0;
    virtual int sub_667B00() = 0;
    virtual int nullsub_h2() = 0;
    virtual int fn_ret_0c() = 0;
    virtual int nullsub_2() = 0;
    virtual int fn_ret_0b() = 0;
    virtual int setStreamSource() = 0;
    virtual int sub_6677F0() = 0;
    virtual int sub_63EAB0_errory() = 0;
    virtual int sub_667E10_renderBuffer() = 0;
    virtual int sub_667F20() = 0;
    virtual int drawPrimitiveUP() = 0;
    virtual int sub_667830() = 0;
    virtual int sub_6680C0() = 0;
    virtual int sub_668120() = 0;
    virtual int sub_667880() = 0;
    virtual int sub_667890() = 0;
    virtual int sub_6679A0() = 0;
    virtual int sub_667B40() = 0;
    virtual int sub_7F8FD0() = 0;
    virtual int sub_667B50() = 0;
    virtual int sub_667B70() = 0;
    virtual int sub_667B80() = 0;
    virtual int sub_667B90() = 0;
    virtual int sub_667B60() = 0;
    virtual int sub_667BA0() = 0;
    virtual int sub_679A20() = 0;
    virtual int get_byte_38() = 0;
    virtual int nullsub____2() = 0;
    virtual int sub_7A87C0() = 0;
    virtual int nullsub_2z() = 0;
    virtual int sub_67A250() = 0;
    virtual char sub_673610() = 0;
    virtual int sub_673620() = 0;
    virtual int sub_673630() = 0;
    virtual int sub_673640() = 0;
    virtual int sub_673650() = 0;
    virtual char sub_673660() = 0;
    virtual char sub_673670() = 0;
    virtual int sub_673680() = 0;
    virtual int sub_673690() = 0;
    virtual int sub_6736A0() = 0;
    virtual int sub_6736B0() = 0;
    virtual int sub_6736C0() = 0;
    virtual int sub_6736E0() = 0;
    virtual int sub_673700() = 0;
    virtual int sub_6736D0() = 0;
    virtual int sub_673710() = 0;
    virtual int sub_673730() = 0;
    virtual int sub_673750() = 0;
    virtual int sub_673770() = 0;
    virtual int sub_673790() = 0;
    virtual int sub_6737A0() = 0;
    virtual int sub_6737C0() = 0;
    virtual int sub_6737D0() = 0;
    virtual int sub_6737F0() = 0;
    virtual int sub_673800() = 0;
    virtual int sub_673810() = 0;
    virtual int accessor1() = 0;
    virtual int fn_ret_byte_0_1arg() = 0;
    virtual int accessor2() = 0;
    virtual int sub_679A50() = 0;
    virtual int sub_679A60() = 0;
    virtual int sub_679A70() = 0;
    virtual int sub_679A80() = 0;
    virtual int sub_679A90() = 0;
    virtual int sub_63EF10() = 0;
    virtual int sub_63EF20() = 0;
    virtual int sub_63EE40() = 0;
    virtual int sub_63EEC0() = 0;
    virtual int sub_63EED0() = 0;
    virtual int sub_796580() = 0;
    virtual int sub_5AD110() = 0;
    virtual int sub_63EF00() = 0;
    virtual int sub_679AA0() = 0;
    virtual int sub_679B60() = 0;
    virtual int sub_4BB9E0() = 0;
    virtual int sub_4BB9F0() = 0;
    virtual int sub_5A1EC0() = 0;
    virtual int sub_679B70() = 0;
    virtual int sub_679B80() = 0;
    virtual int sub_4BB880() = 0;
    virtual int nullsub__2() = 0;
    virtual int nullsub_1() = 0;
    virtual int sub_679B90() = 0;
    virtual int sub_740310() = 0;
    virtual int sub_6403D0_set_byte164() = 0;
    virtual char sub_680630_get_byte164() = 0;
    virtual void sub_63FD30(int) = 0;
    virtual int getD3DDevice8() = 0;
    virtual int sub_796570() = 0;
    virtual int sub_63ECF0() = 0;
    virtual int sub_679EF0() = 0;
    virtual int sub_672D80() = 0;
    virtual int sub_667BE0() = 0;
    virtual int f476() = 0;
    virtual int sub_63ED50() = 0;
    virtual int createVertexBuffer() = 0;
    virtual int sub_63EDE0() = 0;
    virtual int fn_ret_0a() = 0;
    virtual int sub_6677B0() = 0;
    virtual int getClearFlags(char a1) = 0;
};

class NewRendFont : public IBase {
public:
    virtual bool loadFontFile(const bfs::string& filename) = 0;
    virtual void drawText(float x, float y, const bfs::string& text) = 0;
    virtual int getHeight() const = 0;
    virtual int getCharWidth(unsigned char) const = 0;
    virtual int getStringWidth(const bfs::string& text) const = 0;
    virtual void setScale(float x, float y) = 0;
    virtual Vec2& getScale(Vec2& vec) const = 0;
    virtual void setColor(uint32_t color) = 0;
    virtual uint32_t getColor() const = 0;
};

class Renderer {
    uint32_t unknown_0;
    NewRendFont* fontForAIDebug;
    NewRendFont* font; // used for nametags, console, debug drawing
public:
    static Renderer* GetSingleton() {
        intptr_t setup = *(intptr_t*)0x00971EAC;
        if (!setup) return 0;
        return *(Renderer**)(setup + 0x2D8);
    };

    void drawDebugText_orig(int x, int y, const bfs::string& str) noexcept;
    void drawDebugText(int x, int y, const bfs::string& str);

    NewRendFont* getFont() const { return font; };
};

class ITextureHandler : public IBase {
public:
    virtual const bfs::string& getFileExtension() = 0;
    virtual bool canLoad() = 0;
    virtual bool canSave() = 0;
    virtual bool loadTexture(IBFStream*, ITexture*) = 0;
    virtual bool saveTexture(IBFStream*, ITexture*) = 0;
};

class PNGTextureHandler : public ITextureHandler {
    int refcount = 1;
public:
    void addRef() override;
    void release() override;
    IBase* queryInterface(uint32_t) const override;
    const bfs::string& getFileExtension() override;
    bool canLoad() override;
    bool canSave() override;
    bool loadTexture(IBFStream*, ITexture*) override;
    bool saveTexture(IBFStream*, ITexture*) override;
};

class JPEGTextureHandler : public ITextureHandler {
    int refcount = 1;
public:
    void addRef() override;
    void release() override;
    IBase* queryInterface(uint32_t) const override;
    const bfs::string& getFileExtension() override;
    bool canLoad() override;
    bool canSave() override;
    bool loadTexture(IBFStream*, ITexture*) override;
    bool saveTexture(IBFStream*, ITexture*) override;
};

// This function is called repeatedly by a hook at CreateDevice if the resolution in
// the settings cannot be used
bool __stdcall tryRecoverFromInvalidScreenResolution(void* RendPCDX8, void* videoMode_);

void renderer_hook_init();
