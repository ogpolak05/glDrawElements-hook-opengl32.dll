#include <Windows.h>
#include <iostream>
#include "../ext/minhook/minhook.h"

#include <gl/GL.h>

#pragma comment(lib, "opengl32.lib")

typedef void(APIENTRY* fnDrawElements)(unsigned int mode, int count, unsigned int type, const void* indices);
static fnDrawElements oDrawElements = nullptr;

static void APIENTRY hkDrawElements(unsigned int mode, int count, unsigned int type, const void* indices)
{
    oDrawElements(mode, count, type, indices);

    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

    oDrawElements(mode, count, type, indices);
}

unsigned long MainStart(LPVOID hInstance)
{
    FILE* pFile;
    AllocConsole();
    freopen_s(&pFile, "CONOUT$", "w", stdout);
    SetConsoleTitle("wallhack");

    static FARPROC DrawElements = nullptr;

    uintptr_t opengl32 = (uintptr_t)GetModuleHandle("opengl32.dll");
    if (opengl32)
    {
		DrawElements = (FARPROC)GetProcAddress((HMODULE)opengl32, "glDrawElements");
    }

    std::cout << "opengl32.dll: " << std::hex << opengl32 << std::endl;
    std::cout << "glDrawElements: " << DrawElements << std::endl;

    MH_Initialize();

    if (DrawElements)
    {
        MH_CreateHook(DrawElements, hkDrawElements, (LPVOID*)&oDrawElements);
        MH_EnableHook(DrawElements);
    }
    else
    {
        std::cout << "Failed to hook glDrawElements\n";
    }

    while (!(GetAsyncKeyState(VK_DELETE) & 1))
        Sleep(500);

    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    if (pFile)
        fclose(pFile);
    FreeConsole();

    FreeLibraryAndExitThread((HMODULE)hInstance, 0);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainStart, hModule, 0, NULL);
        break;
    }
    return TRUE;
}
