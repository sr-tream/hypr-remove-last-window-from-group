#define WLR_USE_UNSTABLE

#include <cassert>
#include <string_view>
#include <type_traits>

#include <dlfcn.h>

#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>

// Methods
static CFunctionHook* g_pMoveWindowOutOfGroupHook = nullptr;

namespace hook {
    // FIXME: Work only with move window out of group (yes), but also need handle window close event!
    // TODO: 1. Hook window creation
    // TODO: 2. Subscribe to window destroy
    // TODO: 3. Check group size and toggle disable for last window
    EXPORT void moveWindowOutOfGroup(PHLWINDOW pWindow, const std::string& dir) {
        const auto orig       = (*(decltype(&moveWindowOutOfGroup))g_pMoveWindowOutOfGroupHook->m_pOriginal);
        auto       nextWindow = pWindow->m_sGroupData.pNextWindow.lock();

        if (!nextWindow || nextWindow == pWindow)
            return orig(pWindow, dir);

        orig(pWindow, dir);
        if (nextWindow->m_sGroupData.pNextWindow.lock() == nextWindow)
            orig(nextWindow, dir);
    }
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    Dl_info info;
    dladdr((void*)&hook::moveWindowOutOfGroup, &info);
    assert(info.dli_sname != nullptr);

    const auto hookName        = std::string_view{info.dli_sname};
    const auto functionNamePos = hookName.find("moveWindowOutOfGroup");
    assert(functionNamePos != std::string_view::npos);

    const auto functionNmeWithArgs = hookName.substr(functionNamePos);

    auto       FNS = HyprlandAPI::findFunctionsByName(handle, std::string(functionNmeWithArgs));
    for (auto& fn : FNS) {
        if (!fn.demangled.contains("CKeybindManager"))
            continue;

        g_pMoveWindowOutOfGroupHook = HyprlandAPI::createFunctionHook(handle, fn.address, (void*)::hook::moveWindowOutOfGroup);
        break;
    }

    if (g_pMoveWindowOutOfGroupHook == nullptr) {
        HyprlandAPI::addNotification(handle,
                                     "[remove-last-window-from-group] Failure in initialization: Failed to find `CKeybindManager::moveWindowOutOfGroup` with compatible signature",
                                     CColor{1.0, 0.2, 0.2, 1.0}, 7000);
        throw std::runtime_error("[remove-last-window-from-group] Hooks fn init failed");
    }

    if (!g_pMoveWindowOutOfGroupHook->hook()) {
        HyprlandAPI::addNotification(handle, "[remove-last-window-from-group] Failure in initialization (hook failed)!", CColor{1.0, 0.2, 0.2, 1.0}, 7000);
        throw std::runtime_error("[remove-last-window-from-group] Hooks failed");
    }

    return {"remove-last-window-from-group", "A plugin to remove last window from group", "SR_team", "0.1"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    if (g_pMoveWindowOutOfGroupHook != nullptr)
        g_pMoveWindowOutOfGroupHook->unhook();
}

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}