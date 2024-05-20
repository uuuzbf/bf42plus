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

void hook_Renderer_draw_1()
{
    auto rend = Renderer::GetSingleton();
    if (!rend) return;
    auto font = rend->getFont();
    if (!font) return;

    auto oldcolor = font->getColor();

    auto projectiles = ObjectManager_getProjectileMap();

    if (g_settings.enable3DMineMap) {
        auto localPlayer = BFPlayer::getLocal();
        Pos3 playerPos;
        bool playerPosInitialized = false;


        for (auto node = projectiles.head->left; node != projectiles.head; node = node->next()) {
            auto proj = node->pair.second;
            // this compares teams... 
            if (*(int*)((intptr_t)proj + 0x144) == localPlayer->getTeam()) {
                if (!playerPosInitialized) {
                    auto vehicle = localPlayer->getVehicle();
                    playerPos = vehicle->getAbsolutePosition();
                    playerPosInitialized = true;
                }
                Pos3 projPos = proj->getAbsolutePosition();
                float distance = (projPos - playerPos).lengthSquare();
                static const float MAX_MINE_DIST = 65;
                if (distance < MAX_MINE_DIST * MAX_MINE_DIST) {
                    Vec3 screenPos;
                    if (convertWorldPosToScreenPos(screenPos, projPos)) {
                        distance = sqrtf(distance);
                        // alpha range: 60-255
                        uint32_t alpha = (MAX_MINE_DIST - distance) * ((255.0 - 60.0) / MAX_MINE_DIST) + 60.0;

                        font->setColor(0xff1493 | (alpha << 24));

                        font->drawText(screenPos.x - font->getStringWidth("*MINE*") / 2, screenPos.y, "*MINE*");
                        bfs::string diststr = std::format("{}m", (int)distance);
                        font->drawText(screenPos.x - font->getStringWidth(diststr) / 2, screenPos.y - font->getHeight(), diststr);
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

void renderer_hook_init()
{
    drawDebugText_addr = (uintptr_t)hook_function(drawDebugText_addr, 5, method_to_voidptr(&Renderer::drawDebugText));

    install_hook_Renderer_draw_1();
}
