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

void renderer_hook_init()
{
    drawDebugText_addr = (uintptr_t)hook_function(drawDebugText_addr, 5, method_to_voidptr(&Renderer::drawDebugText));

    install_hook_Renderer_draw_1();
}
