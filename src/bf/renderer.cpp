#include "../pch.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <fpng.cpp>
#include <fpng.h>

// disable warnings about unreferenced parameters, uninitialized object variables, __asm blocks, ...
#pragma warning(push)
#pragma warning(disable: 26495 4100 4410 4409 4740)

uintptr_t drawDebugText_addr = 0x004611D0;
void Renderer::drawDebugText_orig(int x, int y, const bfs::string& str) noexcept
{
    _asm mov eax,drawDebugText_addr
    _asm jmp eax
}

__declspec(naked) bool convertWorldPosToScreenPos(Vec3& output, const Vec3& input)
{
    _asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        mov eax, 0x009AB868
        mov esi, [eax] // pRenderView
        mov ebx, output
        mov edi, input
        mov eax, 0x00440790 // convertWorldPosToScreenPos
        call eax
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret
    }
}

#pragma warning(pop)

void Renderer::drawDebugText(int x, int y, const bfs::string& str)
{
    if (font) {
        uint32_t origColor = font->getColor();
        font->setColor(g_settings.debugTextColor | 0xFF000000);

        // adjust debug text offsets if the font isn't the default size
        if (font->getHeight() != 11) {
            y = y * font->getHeight() / 11;
        }
        if (font->getCharWidth('W') != 8) {
            x = x * font->getCharWidth('W') / 8;
        }

        font->drawText(x, y, str);

        font->setColor(origColor);
    }
    else {
        drawDebugText_orig(x, y, str);
    }
}

static bool writeTextureToFile(IBFStream* output, ITexture* texture, const std::string& format)
{
    const auto pixelFormat = texture->getPixelFormat();
    const auto width = texture->getWidth(), height = texture->getHeight();

    void* out[2];
    // Get the pixel data from the texture, out[0] will point
    // to the pixel data, in the format specified by pixelFormat.
    if (!texture->getFrameBuffer(0, pixelFormat, 0, out, 1)) {
        debuglogt("PNGTextureHandler::saveTexture: texture->getFrameBuffer failed (pf: %d)\n", texture->getPixelFormat());
        return false;
    }

    // The resulting pixel data most likely needs conversion for the
    // stbi_write_* functions. Those accept pixels in either Y, RGB or RGBA
    // format.
    int stride;
    uint8_t* data;
    int channels;
    bool freeData = false;
    if (pixelFormat == PF_Pal_8 || pixelFormat == PF_Lum_8) {
        // Untested!
        stride = width;
        data = (uint8_t*)out[0];
        channels = 1;
    }
    else if (pixelFormat == PF_RGB_565 || pixelFormat == PF_XRGB_8888) {
        stride = width * 3;
        data = new uint8_t[stride * height];
        freeData = true;
        channels = 3;
        auto q = data;
        if (pixelFormat == PF_RGB_565) {
            // Untested!
            auto p = (uint16_t*)out[0];
            for (int i = 0; i < (width * height); i++) {
                q[0] = (*p >> 8) & 0b11111000; // red
                q[1] = (*p >> 5) & 0b11111100; // green
                q[2] = (*p << 3); // blue
                p++;
                q += 3;
            }
        }
        else if (pixelFormat == PF_XRGB_8888) {
            // Alpha channel is undefined, ignore it
            auto p = (uint8_t*)out[0];
            for (int i = 0; i < (width * height); i++) {
                q[0] = p[2]; // red
                q[1] = p[1]; // green
                q[2] = p[0]; // blue
                p += 4;
                q += 3;
            }
        }
    }
    else if (pixelFormat == PF_ARGB_8888) {
        // Untested!
        stride = width * 4;
        data = new uint8_t[stride * height];
        freeData = true;
        channels = 4;
        auto q = data;
        auto p = (uint8_t*)out[0];
        for (int i = 0; i < (width * height); i++) {
            q[0] = p[2]; // red
            q[1] = p[1]; // green
            q[2] = p[0]; // blue
            q[3] = p[3]; // alpha
            p += 4;
            q += 4;
        }
    }
    else {
        debuglogt("PNGTextureHandler::saveTexture: invalid pixel format %d\n", pixelFormat);
        return false;
    }

    stbi_write_func* writefunc = [](void* context, void* data, int size) {
        reinterpret_cast<IBFStream*>(context)->write(data, size);
        };

    bool result = false;
    if (format == "png") {
        static bool init_fpng = false;
        if (!init_fpng) {
            fpng::fpng_init();
            init_fpng = true;
        }
        std::vector<uint8_t> outBuffer;
        result = fpng::fpng_encode_image_to_memory(data, width, height, channels, outBuffer);
        if (result) {
            output->write(outBuffer.data(), outBuffer.size());
        }
    }
    else if (format == "jpg") result = stbi_write_jpg_to_func(writefunc, output, width, height, channels, data, 90) > 0;

    if (!result) {
        debuglogt("PNGTextureHandler::saveTexture: stbi_write_%s_to_func failed (w:%d, h:%d, ch:%d, stride:%d)\n", format.c_str(), width, height, channels, stride);
    }

    if (freeData) delete[] data;

    return result;
}

void PNGTextureHandler::addRef()
{
    refcount++;
}

void PNGTextureHandler::release()
{
    refcount--;
    if (refcount == 0) delete this;
}

IBase* PNGTextureHandler::queryInterface(uint32_t iid) const
{
    if (iid == IID_IBase || iid == IID_ITextureHandler || iid == 500012)
        return (IBase*)this;
    return 0;
}

const bfs::string& PNGTextureHandler::getFileExtension()
{
    static bfs::string extension("PNG");
    return extension;
}

bool PNGTextureHandler::canLoad() { return false; }
bool PNGTextureHandler::canSave() { return true; }
bool PNGTextureHandler::loadTexture(IBFStream*, ITexture*) { return false; }

bool PNGTextureHandler::saveTexture(IBFStream* output, ITexture* texture)
{
    return writeTextureToFile(output, texture, "png");
}


void JPEGTextureHandler::addRef()
{
    refcount++;
}

void JPEGTextureHandler::release()
{
    refcount--;
    if (refcount == 0) delete this;
}

IBase* JPEGTextureHandler::queryInterface(uint32_t iid) const
{
    if (iid == IID_IBase || iid == IID_ITextureHandler || iid == 500012)
        return (IBase*)this;
    return 0;
}

const bfs::string& JPEGTextureHandler::getFileExtension()
{
    static bfs::string extension("JPG");
    return extension;
}

bool JPEGTextureHandler::canLoad() { return false; }
bool JPEGTextureHandler::canSave() { return true; }
bool JPEGTextureHandler::loadTexture(IBFStream*, ITexture*) { return false; }

bool JPEGTextureHandler::saveTexture(IBFStream* output, ITexture* texture)
{
    return writeTextureToFile(output, texture, "jpg");
}

bool isPositionOnScreen(float x, float y) {
    // Get viewport size from RendPCDX8's D3DVIEWPORT
    void* rend = *(void**)0x009A99D4;
    if (!rend) return false;
    auto vpSize = (DWORD*)((uintptr_t)rend + 0x7C);
    return x >= 0 && y >= 0 && x <= vpSize[0] && y <= vpSize[1];
}

inline static void draw3DMapItem(const Pos3& position, const Pos3& playerPosition, const bfs::string& text, int maxDistance, uint32_t color, NewRendFont* font, bool showDistance)
{
    float distance = (position - playerPosition).lengthSquare();
    if (distance < maxDistance * maxDistance) {
        Vec3 screenPos;
        if (convertWorldPosToScreenPos(screenPos, position) && isPositionOnScreen(screenPos.x, screenPos.y)) {
            distance = sqrtf(distance);
            // alpha range: 60-255
            uint32_t alpha = (maxDistance - distance) * ((255.0 - 60.0) / maxDistance) + 60.0;

            font->setColor(color | (alpha << 24));

            font->drawText(screenPos.x - font->getStringWidth(text) / 2, screenPos.y, text);
            if (showDistance) {
                bfs::string diststr = std::format("{}m", (int)distance);
                font->drawText(screenPos.x - font->getStringWidth(diststr) / 2, screenPos.y - font->getHeight(), diststr);
            }
        }
    }
}

// These functions can be used to render only the closest visible item of several

struct PreparedMapItem {
    float distanceSquared;
    Vec3 screenPos;
};
inline static void prepareClosest3DMapItem(PreparedMapItem& out, const Pos3& position, const Pos3& playerPosition, int maxDistance)
{
    float distance = (position - playerPosition).lengthSquare();
    if (distance < maxDistance * maxDistance) {
        Vec3 screenPos;
        if (convertWorldPosToScreenPos(screenPos, position) && isPositionOnScreen(screenPos.x, screenPos.y)) {
            if (out.distanceSquared == 0 || out.distanceSquared > distance) {
                out.distanceSquared = distance;
                out.screenPos = screenPos;
            }
        }
    }
}
inline static void drawClosest3DMapItem(const PreparedMapItem& item, const bfs::string& text, int maxDistance, uint32_t color, NewRendFont* font)
{
    float distance = sqrtf(item.distanceSquared);
    // alpha range: 60-255
    uint32_t alpha = (maxDistance - distance) * ((255.0 - 60.0) / maxDistance) + 60.0;

    font->setColor(color | (alpha << 24));

    font->drawText(item.screenPos.x - font->getStringWidth(text) / 2, item.screenPos.y, text);
    bfs::string diststr = std::format("{}m", (int)distance);
    font->drawText(item.screenPos.x - font->getStringWidth(diststr) / 2, item.screenPos.y - font->getHeight(), diststr);
}

void hook_Renderer_draw_1()
{
    auto rend = Renderer::GetSingleton();
    if (!rend) return;
    auto font = rend->getFont();
    if (!font) return;

    auto oldcolor = font->getColor();

    auto localPlayer = BFPlayer::getLocal();
    Pos3 playerPos = localPlayer->getVehicle()->getAbsolutePosition();

    if (g_settings.enable3DMineMap && g_serverSettings.mine3DMap.allow) {
        auto& projectiles = ObjectManager_getProjectileMap();
        for (auto node = projectiles.head->left; node != projectiles.head; node = node->next()) {
            auto proj = node->pair.second;
            // this compares teams... 
            if (*(int*)((intptr_t)proj + 0x144) == localPlayer->getTeam()) {
                draw3DMapItem(proj->getAbsolutePosition(), playerPos, g_serverSettings.mine3DMap.text,
                    g_serverSettings.mine3DMap.distance, g_serverSettings.mine3DMap.color, font, true);
            }
        }
    }

    if (g_settings.enable3DSupplyMap && g_serverSettings.supplyDepot3DMap.allow) {
        // very hacky way to call some SupplyDepot members
        // move these into a SupplyDepot class when there is one
        typedef bool(__fastcall* SupplyDepot_m_t)(IObject*);
        static SupplyDepot_m_t SupplyDepot_isHealing = (SupplyDepot_m_t)0x00543110;
        static SupplyDepot_m_t SupplyDepot_isRepairing = (SupplyDepot_m_t)0x005431B0;
        static SupplyDepot_m_t SupplyDepot_isReloading = (SupplyDepot_m_t)0x005431E0;

        PreparedMapItem repair{}, ammo{}, heal{};

        const auto maxDistance = g_serverSettings.supplyDepot3DMap.distance;

        auto& supplyDepots = ObjectManager_getSupplyDepotMap();
        for (auto node = supplyDepots.head->left; node != supplyDepots.head; node = node->next()) {
            auto supplyDepot = node->pair.second;

            // Skip SupplyDepots attached to moving stuff. This is needed because it would show APCs,
            // and in some mods (EOD) BFSoldiers have SupplyDepots attached to them.
            if (auto parent = supplyDepot->getParent(); parent && parent->hasMobilePhysics()) continue;

            uint32_t color;
            bfs::string description;

            // The order of these checks matters, repair pads are visible as reload and heal points too for some reason
            if (SupplyDepot_isRepairing(supplyDepot)) {
                prepareClosest3DMapItem(repair, supplyDepot->getAbsolutePosition(), playerPos, maxDistance);
            }
            else if (SupplyDepot_isReloading(supplyDepot)) {
                prepareClosest3DMapItem(ammo, supplyDepot->getAbsolutePosition(), playerPos, maxDistance);
            }
            else if (SupplyDepot_isHealing(supplyDepot)) {
                prepareClosest3DMapItem(heal, supplyDepot->getAbsolutePosition(), playerPos, maxDistance);
            }
            else continue;
        }

        if (repair.distanceSquared != 0) drawClosest3DMapItem(repair, "*REPAIR*", maxDistance, 0x000080, font);
        if (ammo.distanceSquared != 0) drawClosest3DMapItem(ammo, "*AMMO*", maxDistance, 0x006400, font);
        if (heal.distanceSquared != 0) drawClosest3DMapItem(heal, "*HEAL*", maxDistance, 0xDC143C, font);
    }

    if (g_settings.enable3DControlPointMap && g_serverSettings.controlPoint3DMap.allow) {
        const float scale = 0.75;
        font->setScale(scale, scale);
        auto& controlPoints = ObjectManager_getControlPointVector();
        for (auto it = controlPoints.begin(); it != controlPoints.end(); it++) {
            auto controlPoint = *it;
            auto tmpl = controlPoint->getTemplate();

            int team = *(int*)((uintptr_t)controlPoint + 0x190);
            uint32_t color = team == 0 ? 0x777777 : team == 1 ? 0xFF0000 : 0x0000FF;

            Vec3 screenPos;
            auto position = controlPoint->getAbsolutePosition() + Vec3(0, 10, 0);
            float radius = *(float*)((uintptr_t)tmpl + 0x2c8);
            const int minDistance = radius * 4;

            float distance = (position - playerPos).lengthSquare();
            if (distance > minDistance * minDistance) {
                if (convertWorldPosToScreenPos(screenPos, position) && isPositionOnScreen(screenPos.x, screenPos.y)) {
                    distance = sqrtf(distance);
                    // alpha range: 60-180
                    uint32_t alpha = (distance - minDistance) * ((130.0 - 60.0) / minDistance) + 60.0;
                    if (alpha > 130) alpha = 130;

                    auto& controlPointName = *(bfs::string*)((uintptr_t)tmpl + 0x2D0);
                    bfs::string localizedName;
                    getAnsiLocale(localizedName, controlPointName);

                    font->setColor(color | (alpha << 24));
                    float sx = floor(screenPos.x), sy = floor(screenPos.y);
                    font->drawText(sx - font->getStringWidth(localizedName) * scale / 2, sy, localizedName);
                    bfs::string diststr = std::format("{}m", (int)distance);
                    font->drawText(sx - font->getStringWidth(diststr) * scale / 2, sy - font->getHeight() * scale, diststr);
                }
            }
        }
        font->setScale(1.0, 1.0);
    }
    
    if (!g_serverSettings.custom3DMaps.empty()) {
        auto& allObjs = ObjectManager_getAllRegisteredObjects();
        for (auto node = allObjs.head->left; node != allObjs.head; node = node->next()) {
            auto obj = node->pair.second;
            // Is root object and not disabled?
            if (obj->getFlags() & 0x02000000 && !(obj->getFlags() & 1)) {
                auto tmpl = obj->getTemplate();
                auto it = g_serverSettings.custom3DMaps.find(tmpl);
                if (it != g_serverSettings.custom3DMaps.end()) {
                    auto& map = it->second;
                    if (!map.onlySameTeam || obj->getTeam() == -1 || obj->getTeam() == localPlayer->getTeam()) {
                        draw3DMapItem(obj->getAbsolutePosition(), playerPos, map.text,
                            map.distance, map.color, font, map.showDistance);
                    }
                }
            }
        }

    }
    
    font->setColor(oldcolor);
}

void install_hook_Renderer_draw_1()
{
    // Add a hook around where the nametags are rendered. This can be used for drawing
    // custom texts on the screen with NewRendFont.
    BEGIN_ASM_CODE(a)
        mov eax, hook_Renderer_draw_1
        call eax
    MOVE_CODE_AND_ADD_CODE(a, 0x004670A5, 5, HOOK_ADD_ORIGINAL_AFTER);
}

bool __stdcall tryRecoverFromInvalidScreenResolution(void* RendPCDX8, void* videoMode_)
{
    struct DisplaySettings {
        int width, height, depth, refreshrate;
        bool windowed;
        bool operator==(const DisplaySettings& r) const {
            return width == r.width && height == r.height && depth == r.depth && refreshrate == r.refreshrate;
        };
    };
    auto videoMode = (DisplaySettings*)videoMode_;

    auto numDisplaySettings = *(size_t*)((uintptr_t)RendPCDX8 + 0xAC);
    auto availableDisplaySettings = *(DisplaySettings**)((uintptr_t)RendPCDX8 + 0xA8);

    static int attempts = 1;

    debuglogt("tryRecoverFromInvalidScreenResolution called, attempt %d, invalid: %dx%dx%d@%d\n", attempts, videoMode->width, videoMode->height, videoMode->depth, videoMode->refreshrate);

    switch (attempts++) {
        case 1: { // List available resolutions and try to use desktop resolution
            DEVMODEA mode = { 0 };
            mode.dmSize = sizeof(DEVMODE);
            bool found = false, error = false;

            // Get desktop resolution and store it as the target resolution
            if (EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &mode)) {
                videoMode->width = mode.dmPelsWidth + 54;
                videoMode->height = mode.dmPelsHeight;
                videoMode->depth = mode.dmBitsPerPel;
                videoMode->refreshrate = mode.dmDisplayFrequency;
                videoMode->windowed = 0;
            }
            else {
                debuglog("  EnumDisplaySettings error: %08X\n", GetLastError());
                error = true;
            }

            debuglog("  have %zu available resolutions:", numDisplaySettings);
            for (size_t i = 0; i < numDisplaySettings; i++) {
                auto& res = availableDisplaySettings[i];
                debuglog(" %dx%dx%d@%d", res.width, res.height, res.depth, res.refreshrate);

                if (res == *videoMode) {
                    found = true;
                }
            }
            debuglog("\n");

            if (found) {
                debuglog("  selected: %dx%dx%d@%d (desktop resolution)\n", videoMode->width, videoMode->height, videoMode->depth, videoMode->refreshrate);
                return true;
            }
            else if (!error) {
                debuglog("  desktop resolution %dx%dx%d@%d was not found in list\n", videoMode->width, videoMode->height, videoMode->depth, videoMode->refreshrate);
            }
            [[fallthrough]];
        }
        case 2: { // Pick a resolution from the list of available resolutions
            int highestAreaW = 0, highestAreaH = 0, highestArea = 0;
            int highestBitdepth = 0;
            // Find highest resolution and bit depth
            for (size_t i = 0; i < numDisplaySettings; i++) {
                auto& res = availableDisplaySettings[i];

                // Don't use too high resolutions
                if (res.width > 1920) continue;

                auto area = res.width * res.height;
                if (area > highestArea) {
                    highestArea = area, highestAreaW = res.width, highestAreaH = res.height;
                }
                if (res.depth > highestBitdepth) {
                    highestBitdepth = res.depth;
                }
            }

            debuglog("  highest refresh rate and bitdepth: %dx%dx%d\n", highestAreaW, highestAreaH, highestBitdepth);

            int idx60Hz = -1, idxHighestRefresh = -1, highestRefresh = -1;
            // Find indices for the above parameters where the refresh rate is 60Hz and where it is the highest
            for (size_t i = 0; i < numDisplaySettings; i++) {
                auto& res = availableDisplaySettings[i];

                if (res.width == highestAreaW && res.height == highestAreaH && res.depth == highestBitdepth) {
                    if (res.refreshrate == 60) idx60Hz = i;
                    if (highestRefresh < res.refreshrate) {
                        highestRefresh = res.refreshrate;
                        idxHighestRefresh = i;
                    }
                }
            }

            // Try using highest resolution with highest bitdepth
            // First with 60Hz refresh rate
            if (idx60Hz != -1) {
                *videoMode = availableDisplaySettings[idx60Hz];
                debuglog("  selected 60Hz refresh rate\n");
                return true;
            }
            // Otherwise, highest available refresh rate
            if (idxHighestRefresh != -1) {
                *videoMode = availableDisplaySettings[idxHighestRefresh];
                debuglog("  selected %dHz refresh rate\n", highestRefresh);
                return true;
            }
            [[fallthrough]];
        }
        default: // Out of options, fail
            return false;
    }

    // unreachable
}

static __declspec(naked) bool TextureManager_registerTextureHandler(ITextureHandler* handler)
{
    _asm {
        push ebp
        mov ebp,esp

        // Get g_TextureManager into ecx
        mov ecx, 0x009A99D8
        mov ecx, [ecx]
        test ecx,ecx
        jz fail

        // Call g_TextureManager->registerTextureHandler(handler)
        mov eax, [ecx]
        push handler
        call [eax+0x38]

        mov al,1
        jmp done
    fail:
        xor eax,eax
    done:
        pop ebp
        ret
    }
}

static bool registerExtraTextureHandlers()
{
    ITextureHandler* handler = new PNGTextureHandler();
    if (!TextureManager_registerTextureHandler(handler)) {
        debuglogt("failed to register PNG handler\n");
        return false;
    }
    handler->release();

    handler = new JPEGTextureHandler();
    if (!TextureManager_registerTextureHandler(handler)) {
        debuglogt("failed to register JPEG handler\n");
        return false;
    }
    handler->release();

    return true;
}

static void patch_hook_registerTextureHandlers()
{
    // Add a tail call from the end of registerTextureHandlers
    // to our function
    BEGIN_ASM_CODE(a)
        mov ecx, registerExtraTextureHandlers
        jmp ecx
    MOVE_CODE_AND_ADD_CODE(a, 0x00460A92, 5, HOOK_DISCARD_ORIGINAL);
}

static const char* getScreenshotPath()
{
    static char path[MAX_PATH];
    SYSTEMTIME now;
    GetLocalTime(&now);
    snprintf(path, MAX_PATH, "ScreenShots/%04d-%02d-%02d_%02d-%02d-%02d-%03d.%s",
        now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds, g_settings.screenshotFormat.value.c_str());
    return path;
}

static void patch_screenshot_name()
{
    // static const char* newFormat = "ScreenShots/ScreenShot%d.png";
    // patchBytes<const char*>(0x004676C0, newFormat);

    // Replace the code that generates the screenshot name with a custom function
    BEGIN_ASM_CODE(a)
        // eax must contain a const char* with the screenshot path
        mov eax, getScreenshotPath
        call eax
    MOVE_CODE_AND_ADD_CODE(a, 0x004676B4, 30, HOOK_DISCARD_ORIGINAL);

    // Disable code that initializes g_screenshot_counter when the main window is
    // created. This may save some time because the game tries to open each
    // screenshot file until it fails.
    inject_jmp(0x004634B2, 5, (void*)0x463552, 1);
}

void renderer_hook_init()
{
    drawDebugText_addr = (uintptr_t)hook_function(drawDebugText_addr, 5, method_to_voidptr(&Renderer::drawDebugText));

    install_hook_Renderer_draw_1();
    patch_hook_registerTextureHandlers();
    patch_screenshot_name();
}
