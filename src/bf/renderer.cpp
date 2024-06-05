#include "../pch.h"

uintptr_t drawDebugText_addr = 0x004611D0;
void Renderer::drawDebugText_orig(int x, int y, const bfs::string& str) noexcept
{
    _asm mov eax,drawDebugText_addr
    _asm jmp eax
}

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

inline static void draw3DMapItem(const Pos3& position, const Pos3& playerPosition, const bfs::string& description, const float maxDistance, uint32_t color, NewRendFont* font)
{
    float distance = (position - playerPosition).lengthSquare();
    if (distance < maxDistance * maxDistance) {
        Vec3 screenPos;
        if (convertWorldPosToScreenPos(screenPos, position)) {
            distance = sqrtf(distance);
            // alpha range: 60-255
            uint32_t alpha = (maxDistance - distance) * ((255.0 - 60.0) / maxDistance) + 60.0;

            font->setColor(color | (alpha << 24));

            font->drawText(screenPos.x - font->getStringWidth(description) / 2, screenPos.y, description);
            bfs::string diststr = std::format("{}m", (int)distance);
            font->drawText(screenPos.x - font->getStringWidth(diststr) / 2, screenPos.y - font->getHeight(), diststr);
        }
    }
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

    if (g_settings.enable3DMineMap) {


        auto projectiles = ObjectManager_getProjectileMap();
        for (auto node = projectiles.head->left; node != projectiles.head; node = node->next()) {
            auto proj = node->pair.second;
            // this compares teams... 
            if (*(int*)((intptr_t)proj + 0x144) == localPlayer->getTeam()) {
                draw3DMapItem(proj->getAbsolutePosition(), playerPos, "*MINE*", 65.0, 0xff1493, font);
            }
        }
    }

    if (g_settings.enable3DSupplyMap) {
        // very hacky way to call some SupplyDepot members
        // move these into a SupplyDepot class when there is one
        typedef bool(__fastcall* SupplyDepot_m_t)(IObject*);
        static SupplyDepot_m_t SupplyDepot_isHealing = (SupplyDepot_m_t)0x00543110;
        static SupplyDepot_m_t SupplyDepot_isRepairing = (SupplyDepot_m_t)0x005431B0;
        static SupplyDepot_m_t SupplyDepot_isReloading = (SupplyDepot_m_t)0x005431E0;

        auto supplyDepots = ObjectManager_getSupplyDepotMap();
        for (auto node = supplyDepots.head->left; node != supplyDepots.head; node = node->next()) {
            auto supplyDepot = node->pair.second;

            // Skip SupplyDepots attached to moving stuff. This is needed because it would show APCs,
            // and in some mods (EOD) BFSoldiers have SupplyDepots attached to them.
            if (auto parent = supplyDepot->getParent(); parent && parent->hasMobilePhysics()) continue;

            uint32_t color;
            bfs::string description;

            // The order of these checks matters, repair pads are visible as reload and heal points too for some reason
            if (SupplyDepot_isRepairing(supplyDepot)) {
                description = "*REPAIR*";
                color = 0x000080;
            }
            else if (SupplyDepot_isReloading(supplyDepot)) {
                description = "*AMMO*";
                color = 0x006400;
            }
            else if (SupplyDepot_isHealing(supplyDepot)) {
                description = "*HEAL*";
                color = 0xDC143C;
            }
            else continue;

            draw3DMapItem(supplyDepot->getAbsolutePosition(), playerPos, description, 65.0, color, font);
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
                debuglog("  EnumDisplaySettings error: %08X", GetLastError());
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

void renderer_hook_init()
{
    drawDebugText_addr = (uintptr_t)hook_function(drawDebugText_addr, 5, method_to_voidptr(&Renderer::drawDebugText));

    install_hook_Renderer_draw_1();
}
