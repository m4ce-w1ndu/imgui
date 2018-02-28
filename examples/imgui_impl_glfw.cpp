// ImGui Platform Binding for: GLFW
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan..)
// (Info: GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

// Implemented features:
//  [X] Gamepad navigation mapping. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2018-XX-XX: Platform: Added support for multiple windows via the ImGuiPlatformInterface
//  2018-XX-XX: Inputs: Added support for mouse cursors, honoring ImGui::GetMouseCursor() value.
//  2018-02-06: Misc: Removed call to ImGui::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2018-02-06: Inputs: Added mapping for ImGuiKey_Space.
//  2018-01-25: Inputs: Added gamepad support if ImGuiConfigFlags_NavEnableGamepad is set.
//  2018-01-25: Inputs: Honoring the io.WantMoveMouse by repositioning the mouse (when using navigation and ImGuiConfigFlags_NavMoveMouse is set).
//  2018-01-20: Inputs: Added Horizontal Mouse Wheel support.
//  2018-01-18: Inputs: Added mapping for ImGuiKey_Insert.
//  2017-08-25: Inputs: MousePos set to -FLT_MAX,-FLT_MAX when mouse is unavailable/missing (instead of -1,-1).
//  2016-10-15: Misc: Added a void* user_data parameter to Clipboard function handlers.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h" // FIXME-PLATFORM

// GLFW
#include <GLFW/glfw3.h>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // for glfwGetWin32Window
#endif

// Data
static GLFWwindow*  g_Window = NULL;
static double       g_Time = 0.0f;
static bool         g_MouseJustPressed[5] = { false, false, false, false, false };
static GLFWcursor*  g_MouseCursors[ImGuiMouseCursor_Count_] = { 0 };

// Forward Declarations
static void ImGui_ImplGlfw_InitPlatformInterface();
static void ImGui_ImplGlfw_ShutdownPlatformInterface();

static const char* ImGui_ImplGlfw_GetClipboardText(void* user_data)
{
    return glfwGetClipboardString((GLFWwindow*)user_data);
}

static void ImGui_ImplGlfw_SetClipboardText(void* user_data, const char* text)
{
    glfwSetClipboardString((GLFWwindow*)user_data, text);
}

void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow*, int button, int action, int /*mods*/)
{
    if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(g_MouseJustPressed))
        g_MouseJustPressed[button] = true;
}

void ImGui_ImplGlfw_ScrollCallback(GLFWwindow*, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += (float)xoffset;
    io.MouseWheel += (float)yoffset;
}

void ImGui_ImplGlfw_KeyCallback(GLFWwindow*, int key, int, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)
        io.KeysDown[key] = true;
    if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;

    (void)mods; // Modifiers are not reliable across systems
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void ImGui_ImplGlfw_CharCallback(GLFWwindow*, unsigned int c)
{
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

void ImGui_ImplGlfw_InstallCallbacks(GLFWwindow* window)
{
    glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
    glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
    glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
    glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
}

bool    ImGui_ImplGlfw_Init(GLFWwindow* window, bool install_callbacks)
{
    g_Window = window;

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                         // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
    io.ClipboardUserData = g_Window;
#ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(g_Window);
#endif

    g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
    g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.

    if (install_callbacks)
        ImGui_ImplGlfw_InstallCallbacks(window);

    // Our mouse update function expect PlatformHandle to be filled for the main viewport
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = (void*)g_Window;
    if (io.ConfigFlags & ImGuiConfigFlags_MultiViewports)
        ImGui_ImplGlfw_InitPlatformInterface();

    return true;
}

void ImGui_ImplGlfw_Shutdown()
{
    ImGui_ImplGlfw_ShutdownPlatformInterface();

    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_Count_; cursor_n++)
    {
        glfwDestroyCursor(g_MouseCursors[cursor_n]);
        g_MouseCursors[cursor_n] = NULL;
    }
}

static void ImGui_ImplGlfw_UpdateMouse()
{
#if 0
    if (io.WantMoveMouse)
        glfwSetCursorPos(g_Window, (double)io.MousePos.x, (double)io.MousePos.y);   // Set mouse position if requested by io.WantMoveMouse flag (used when io.NavMovesTrue is enabled by user and using directional navigation)
#endif

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    io.MousePosViewport = 0;
    io.MouseHoveredViewport = 0;

    // Update buttons
    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
    {
        // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        io.MouseDown[i] = g_MouseJustPressed[i] || glfwGetMouseButton(g_Window, i) != 0;
        g_MouseJustPressed[i] = false;
    }

    const ImVector<ImGuiViewport*>& viewports = ImGui::GetViewports();
    for (int n = 0; n < viewports.Size; n++)
    {
        ImGuiViewport* viewport = viewports[n];
        GLFWwindow* window = (GLFWwindow*)viewport->PlatformHandle;
        IM_ASSERT(window != NULL);
        if (glfwGetWindowAttrib(window, GLFW_FOCUSED))
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            io.MousePos = ImVec2((float)mouse_x + viewport->Pos.x, (float)mouse_y + viewport->Pos.y);
            io.MousePosViewport = viewport->ID;
            for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
                io.MouseDown[i] |= glfwGetMouseButton(window, i) != 0;
        }

#if GLFW_HAS_GLFW_HOVERED
        io.ConfigFlags |= ImGuiConfigFlags_PlatformHasMouseHoveredViewport;
        if (glfwGetWindowAttrib(data->Window, GLFW_HOVERED) && !(viewport->Flags & ImGuiViewportFlags_NoInputs))
            io.MouseHoveredViewport = viewport->ID;
#endif
    }

    // Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
    ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None)
    {
        glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    else
    {
        glfwSetCursor(g_Window, g_MouseCursors[cursor] ? g_MouseCursors[cursor] : g_MouseCursors[ImGuiMouseCursor_Arrow]);
        glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void ImGui_ImplGlfw_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt());     // Font atlas needs to be built, call renderer _NewFrame() function e.g. ImGui_ImplOpenGL3_NewFrame() 

    // Setup display size
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(g_Window, &w, &h);
    glfwGetFramebufferSize(g_Window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

    // Setup time step
    double current_time = glfwGetTime();
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f/60.0f);
    g_Time = current_time;

    ImGui_ImplGlfw_UpdateMouse();

    // Gamepad navigation mapping [BETA]
    memset(io.NavInputs, 0, sizeof(io.NavInputs));
    if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad)
    {
        // Update gamepad inputs
        #define MAP_BUTTON(NAV_NO, BUTTON_NO)       { if (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS) io.NavInputs[NAV_NO] = 1.0f; }
        #define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1) { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); if (v > 1.0f) v = 1.0f; if (io.NavInputs[NAV_NO] < v) io.NavInputs[NAV_NO] = v; }
        int axes_count = 0, buttons_count = 0;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
        const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
        MAP_BUTTON(ImGuiNavInput_Activate,   0);     // Cross / A
        MAP_BUTTON(ImGuiNavInput_Cancel,     1);     // Circle / B
        MAP_BUTTON(ImGuiNavInput_Menu,       2);     // Square / X
        MAP_BUTTON(ImGuiNavInput_Input,      3);     // Triangle / Y
        MAP_BUTTON(ImGuiNavInput_DpadLeft,   13);    // D-Pad Left
        MAP_BUTTON(ImGuiNavInput_DpadRight,  11);    // D-Pad Right
        MAP_BUTTON(ImGuiNavInput_DpadUp,     10);    // D-Pad Up
        MAP_BUTTON(ImGuiNavInput_DpadDown,   12);    // D-Pad Down
        MAP_BUTTON(ImGuiNavInput_FocusPrev,  4);     // L1 / LB
        MAP_BUTTON(ImGuiNavInput_FocusNext,  5);     // R1 / RB
        MAP_BUTTON(ImGuiNavInput_TweakSlow,  4);     // L1 / LB
        MAP_BUTTON(ImGuiNavInput_TweakFast,  5);     // R1 / RB
        MAP_ANALOG(ImGuiNavInput_LStickLeft, 0,  -0.3f,  -0.9f);
        MAP_ANALOG(ImGuiNavInput_LStickRight,0,  +0.3f,  +0.9f);
        MAP_ANALOG(ImGuiNavInput_LStickUp,   1,  +0.3f,  +0.9f);
        MAP_ANALOG(ImGuiNavInput_LStickDown, 1,  -0.3f,  -0.9f);
        #undef MAP_BUTTON
        #undef MAP_ANALOG
    }

    // Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
    ImGui::NewFrame();
}

// --------------------------------------------------------------------------------------------------------
// Platform Windows
// --------------------------------------------------------------------------------------------------------

struct ImGuiPlatformDataGlfw
{
    GLFWwindow* Window;
    bool        WindowOwned;

    ImGuiPlatformDataGlfw() { Window = NULL; WindowOwned = false; }
    ~ImGuiPlatformDataGlfw() { IM_ASSERT(Window == NULL); }
};

static void ImGui_ImplGlfw_CreateViewport(ImGuiViewport* viewport)
{
    ImGuiPlatformDataGlfw* data = IM_NEW(ImGuiPlatformDataGlfw)();
    viewport->PlatformUserData = data;

    // GLFW 3.2 unfortunately always set focus on glfwCreateWindow() if GLFW_VISIBLE is set, regardless of GLFW_FOCUSED
    glfwWindowHint(GLFW_VISIBLE, false);
    glfwWindowHint(GLFW_FOCUSED, false);
    glfwWindowHint(GLFW_DECORATED, (viewport->Flags & ImGuiViewportFlags_NoDecoration) ? false : true);
    data->Window = glfwCreateWindow((int)viewport->Size.x, (int)viewport->Size.y, "No Title Yet", NULL, g_Window);
    data->WindowOwned = true;
    viewport->PlatformHandle = (void*)data->Window;
    viewport->Name = NULL;
    ImGui_ImplGlfw_InstallCallbacks(data->Window);
}

static void ImGui_ImplGlfw_DestroyViewport(ImGuiViewport* viewport)
{
    if (ImGuiPlatformDataGlfw* data = (ImGuiPlatformDataGlfw*)viewport->PlatformUserData)
    {
#if GLFW_HAS_GLFW_HOVERED
        HWND hwnd = glfwGetWin32Window(data->Window);
        ::RemovePropA(hwnd, "IMGUI_VIEWPORT");
    #endif
        if (data->Window && data->WindowOwned)
            glfwDestroyWindow(data->Window);
        data->Window = NULL;
        IM_DELETE(data);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = NULL;
}

#if defined(_WIN32) && GLFW_HAS_GLFW_HOVERED
static WNDPROC g_GlfwWndProc = NULL;
static LRESULT CALLBACK WndProcNoInputs(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCHITTEST)
    {
        ImGuiViewport* viewport = (ImGuiViewport*)::GetPropA(hWnd, "IMGUI_VIEWPORT");
        if (viewport->Flags & ImGuiViewportFlags_NoInputs)
            return HTTRANSPARENT;
    }
    return ::CallWindowProc(g_GlfwWndProc, hWnd, msg, wParam, lParam);
}
#endif

static void ImGui_ImplGlfw_ShowWindow(ImGuiViewport* viewport)
{
    ImGuiPlatformDataGlfw* data = (ImGuiPlatformDataGlfw*)viewport->PlatformUserData;

#if defined(_WIN32)
    // GLFW hack: Hide icon from task bar
    HWND hwnd = glfwGetWin32Window(data->Window);
    if (viewport->Flags & ImGuiViewportFlags_NoDecoration)
    {
        LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ex_style &= ~WS_EX_APPWINDOW;
        ex_style |= WS_EX_TOOLWINDOW;
        ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
    }

    // GLFW hack: install hook for WM_NCHITTEST message handler
#if GLFW_HAS_GLFW_HOVERED
    ::SetPropA(hwnd, "IMGUI_VIEWPORT", viewport);
    if (g_GlfwWndProc == NULL)
        g_GlfwWndProc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
    ::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProcNoInputs);
#endif

    // GLFW hack: GLFW 3.2 has a bug where glfwShowWindow() also activates/focus the window.
    // The fix was pushed to GLFW repository on 2018/01/09 and should be included in GLFW 3.3. See https://github.com/glfw/glfw/issues/1179
    if (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
    {
        ::ShowWindow(hwnd, SW_SHOWNA);
        return;
    }
#endif

    glfwShowWindow(data->Window);
}

static ImVec2 ImGui_ImplGlfw_GetWindowPos(ImGuiViewport* viewport)
{
    ImGuiPlatformDataGlfw* data = (ImGuiPlatformDataGlfw*)viewport->PlatformUserData;
    int x = 0, y = 0;
    glfwGetWindowPos(data->Window, &x, &y);
    return ImVec2((float)x, (float)y);
}

static void ImGui_ImplGlfw_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
{
    ImGuiPlatformDataGlfw* data = (ImGuiPlatformDataGlfw*)viewport->PlatformUserData;
    glfwSetWindowPos(data->Window, (int)pos.x, (int)pos.y);
}

static ImVec2 ImGui_ImplGlfw_GetWindowSize(ImGuiViewport* viewport)
{
    ImGuiPlatformDataGlfw* data = (ImGuiPlatformDataGlfw*)viewport->PlatformUserData;
    int w = 0, h = 0;
    glfwGetWindowSize(data->Window, &w, &h);
    return ImVec2((float)w, (float)h);
}

static void ImGui_ImplGlfw_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ImGuiPlatformDataGlfw* data = (ImGuiPlatformDataGlfw*)viewport->PlatformUserData;
    glfwSetWindowSize(data->Window, (int)size.x, (int)size.y);
}

static void ImGui_ImplGlfw_SetWindowTitle(ImGuiViewport* viewport, const char* title)
{
    ImGuiPlatformDataGlfw* data = (ImGuiPlatformDataGlfw*)viewport->PlatformUserData;
    glfwSetWindowTitle(data->Window, title);
}

static void ImGui_ImplGlfw_RenderViewport(ImGuiViewport* viewport)
{
    ImGuiPlatformDataGlfw* data = (ImGuiPlatformDataGlfw*)viewport->PlatformUserData;
    glfwMakeContextCurrent(data->Window);

    if (glfwWindowShouldClose(data->Window))
        viewport->PlatformRequestClose = true;
}

static void ImGui_ImplGlfw_SwapBuffers(ImGuiViewport* viewport)
{
    ImGuiPlatformDataGlfw* data = (ImGuiPlatformDataGlfw*)viewport->PlatformUserData;
    glfwSwapBuffers(data->Window);
}

static void ImGui_ImplGlfw_InitPlatformInterface()
{
    // Register platform interface (will be coupled with a renderer interface)
    ImGuiIO& io = ImGui::GetIO();
    io.PlatformInterface.CreateViewport = ImGui_ImplGlfw_CreateViewport;
    io.PlatformInterface.DestroyViewport = ImGui_ImplGlfw_DestroyViewport;
    io.PlatformInterface.ShowWindow = ImGui_ImplGlfw_ShowWindow;
    io.PlatformInterface.SetWindowPos = ImGui_ImplGlfw_SetWindowPos;
    io.PlatformInterface.GetWindowPos = ImGui_ImplGlfw_GetWindowPos;
    io.PlatformInterface.SetWindowSize = ImGui_ImplGlfw_SetWindowSize;
    io.PlatformInterface.GetWindowSize = ImGui_ImplGlfw_GetWindowSize;
    io.PlatformInterface.SetWindowTitle = ImGui_ImplGlfw_SetWindowTitle;
    io.PlatformInterface.RenderViewport = ImGui_ImplGlfw_RenderViewport;
    io.PlatformInterface.SwapBuffers = ImGui_ImplGlfw_SwapBuffers;

    // Register main window handle
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGuiPlatformDataGlfw* data = IM_NEW(ImGuiPlatformDataGlfw)();
    data->Window = g_Window;
    data->WindowOwned = false;
    main_viewport->PlatformUserData = data;
}

static void ImGui_ImplGlfw_ShutdownPlatformInterface()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = NULL;
    memset(&io.PlatformInterface, 0, sizeof(io.PlatformInterface));
}
