#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef struct {
    char *data;
    i64 count;
} String;
String MAKE_STRING(char *data, i64 count) {
    String string;
    string.data = data;
    string.count = count;
    return string;
};
typedef struct {
    void *data;
    i64 count;
} Slice;
Slice MAKE_SLICE(void *data, i64 count) {
    Slice slice;
    slice.data = data;
    slice.count = count;
    return slice;
};
typedef struct {
    void *data;
    i64 type;
} Any;
Any MAKE_ANY(void *data, i64 type) {
    Any any;
    any.data = data;
    any.type = type;
    return any;
};
void print_char(u8 c) {
    printf("%c", c);
}
void print_int(i64 i) {
    printf("%lld", i);
}
void print_float(float f) {
    printf("%f", f);
}
void print_bool(bool b) {
    printf((b ? "true" : "false"));
}
void print_string(String string) {
    for (i64 i = 0; i < string.count; i++) {
        char c = string.data[i];
        putchar(c);
    }
}
void *alloc(i64 size) {
    char *memory = (char *)malloc(size);
    return memory;
}
void assert(bool condition) {
    if (!condition) {
        printf("Assertion failed.");
        *((char *)0) = 0;
    }
}

// Forward declarations
void print_char(u8 c);
void print_int(i64 i);
void print_float(f32 f);
void print_bool(bool b);
void print_string(String str);
void *alloc(i64 size);
void free(void *ptr);
void assert(bool condition);
void *memcpy(void *dst, void *src, u64 size_in_bytes);
void *memmove(void *dst, void *src, u64 size_in_bytes);
u64 strlen(u8 *cstr);
void print(String fmt, Slice args);
void printa(Slice args);
bool string_eq(String a, String b);
String string_ptr(u8 *ptr, i64 count);
void SDL_AddEventWatch(i32 (*filter)(void *, struct Event *), void *userdata);
void SDL_AddHintCallback(u8 *name, u32 (*callback)(u32 , void *), void *userdata);
i32 SDL_AddTimer(u32 interval, u32 (*callback)(u32 , void *), void *param);
struct Pixel_Format *SDL_AllocFormat(u32 pixel_format);
struct Palette *SDL_AllocPalette(i32 ncolors);
struct Rw_Ops *SDL_AllocRW();
i32 SDL_AtomicAdd(struct Atomic *a, i32 v);
i64 SDL_AtomicCAS(struct Atomic *a, i32 oldval, i32 newval);
i64 SDL_AtomicCASPtr(void **a, void *oldval, void *newval);
i32 SDL_AtomicGet(struct Atomic *a);
void *SDL_AtomicGetPtr(void **a);
void SDL_AtomicLock(i32 *lock);
i32 SDL_AtomicSet(struct Atomic *a, i32 v);
void *SDL_AtomicSetPtr(void **a, void *v);
i64 SDL_AtomicTryLock(i32 *lock);
void SDL_AtomicUnlock(i32 *lock);
i32 SDL_AudioInit(u8 *driver_name);
void SDL_AudioQuit();
i32 SDL_BuildAudioCVT(struct Audio_Cvt *cvt, u16 src_format, u8 src_channels, i32 src_rate, u16 dst_format, u8 dst_channels, i32 dst_rate);
void SDL_CalculateGammaRamp(f32 gamma, u16 *ramp);
i32 SDL_CaptureMouse(i64 enabled);
void SDL_ClearError();
void SDL_ClearHints();
void SDL_ClearQueuedAudio(u32 dev);
void SDL_CloseAudio();
void SDL_CloseAudioDevice(u32 dev);
i32 SDL_CondBroadcast(struct Cond *cond);
i32 SDL_CondSignal(struct Cond *cond);
i32 SDL_CondWait(struct Cond *cond, struct Mutex *mutex);
i32 SDL_CondWaitTimeout(struct Cond *cond, struct Mutex *mutex, u32 ms);
i32 SDL_ConvertAudio(struct Audio_Cvt *cvt);
i32 SDL_ConvertPixels(i32 width, i32 height, u32 src_format, void *src, i32 src_pitch, u32 dst_format, void *dst, i32 dst_pitch);
struct Surface *SDL_ConvertSurface(struct Surface *src, struct Pixel_Format *fmt, u32 flags);
struct Surface *SDL_ConvertSurfaceFormat(struct Surface *src, u32 pixel_format, u32 flags);
struct Cursor *SDL_CreateColorCursor(struct Surface *surface, i32 hot_x, i32 hot_y);
struct Cond *SDL_CreateCond();
struct Cursor *SDL_CreateCursor(u8 *data, u8 *mask, i32 w, i32 h, i32 hot_x, i32 hot_y);
struct Mutex *SDL_CreateMutex();
struct Surface *SDL_CreateRGBSurface(u32 flags, i32 width, i32 height, i32 depth, u32 Rmask, u32 Gmask, u32 Bmask, u32 Amask);
struct Surface *SDL_CreateRGBSurfaceFrom(void *pixels, i32 width, i32 height, i32 depth, i32 pitch, u32 Rmask, u32 Gmask, u32 Bmask, u32 Amask);
struct Surface *SDL_CreateRGBSurfaceWithFormat(u32 flags, i32 width, i32 height, i32 depth, u32 format);
struct Surface *SDL_CreateRGBSurfaceWithFormatFrom(void *pixels, i32 width, i32 height, i32 depth, i32 pitch, u32 format);
struct Renderer *SDL_CreateRenderer(struct Window *window, i32 index, i64 flags);
struct Sem *SDL_CreateSemaphore(u32 initial_value);
struct Window *SDL_CreateShapedWindow(u8 *title, u32 x, u32 y, u32 w, u32 h, i64 flags);
struct Renderer *SDL_CreateSoftwareRenderer(struct Surface *surface);
struct Cursor *SDL_CreateSystemCursor(i64 id);
struct Texture *SDL_CreateTexture(struct Renderer *renderer, u32 format, i32 access, i32 w, i32 h);
struct Texture *SDL_CreateTextureFromSurface(struct Renderer *renderer, struct Surface *surface);
struct Thread *SDL_CreateThread(i32 (*fn)(void *), u8 *name, void *data);
struct Window *SDL_CreateWindow(u8 *title, i32 x, i32 y, i32 w, i32 h, i64 flags);
i32 SDL_CreateWindowAndRenderer(i32 width, i32 height, i64 window_flags, struct Window **window, struct Renderer **renderer);
struct Window *SDL_CreateWindowFrom(void *data);
i64 SDL_DXGIGetOutputInfo(i32 display_index, i32 *adapter_index, i32 *output_index);
void SDL_DelEventWatch(i32 (*filter)(void *, struct Event *), void *userdata);
void SDL_DelHintCallback(u8 *name, u32 (*callback)(u32 , void *), void *userdata);
void SDL_Delay(u32 ms);
u32 SDL_DequeueAudio(u32 dev, void *data, u32 len);
void SDL_DestroyCond(struct Cond *cond);
void SDL_DestroyMutex(struct Mutex *mutex);
void SDL_DestroyRenderer(struct Renderer *renderer);
void SDL_DestroySemaphore(struct Sem *sem);
void SDL_DestroyTexture(struct Texture *texture);
void SDL_DestroyWindow(struct Window *window);
void SDL_DetachThread(struct Thread *thread);
i32 SDL_Direct3D9GetAdapterIndex(i32 display_index);
void SDL_DisableScreenSaver();
void SDL_EnableScreenSaver();
i64 SDL_EnclosePoints(struct Point *points, i32 count, struct Rect *clip, struct Rect *result);
i32 SDL_Error(i64 code);
u8 SDL_EventState(u32 event_type, i32 state);
i32 SDL_FillRect(struct Surface *dst, struct Rect *rect, u32 color);
i32 SDL_FillRects(struct Surface *dst, struct Rect *rect, i32 count, u32 color);
void SDL_FilterEvents(i32 (*filter)(void *, struct Event *), void *userdata);
void SDL_FlushEvent(u32 event_type);
void SDL_FlushEvents(u32 min_type, u32 max_type);
void SDL_FreeCursor(struct Cursor *cursor);
void SDL_FreeFormat(struct Pixel_Format *format);
void SDL_FreePalette(struct Palette *palette);
void SDL_FreeRW(struct Rw_Ops *area);
void SDL_FreeSurface(struct Surface *surface);
void SDL_FreeWAV(u8 *audio_buf);
i32 SDL_GL_BindTexture(struct Texture *texture, f32 *texw, f32 *texh);
void *SDL_GL_CreateContext(struct Window *window);
void SDL_GL_DeleteContext(void *gl_context);
i64 SDL_GL_ExtensionSupported(u8 *extension);
i32 SDL_GL_GetAttribute(i64 attr, i32 *value);
void *SDL_GL_GetCurrentContext();
struct Window *SDL_GL_GetCurrentWindow();
void SDL_GL_GetDrawableSize(struct Window *window, i32 *w, i32 *h);
void *SDL_GL_GetProcAddress(u8 *name);
i32 SDL_GL_GetSwapInterval();
i32 SDL_GL_LoadLibrary(u8 *path);
i32 SDL_GL_MakeCurrent(struct Window *window, void *gl_context);
void SDL_GL_ResetAttributes();
i32 SDL_GL_SetAttribute(i64 attr, i32 value);
i32 SDL_GL_SetSwapInterval(i32 interval);
void SDL_GL_SwapWindow(struct Window *window);
i32 SDL_GL_UnbindTexture(struct Texture *texture);
void SDL_GL_UnloadLibrary();
i32 SDL_GameControllerAddMapping(u8 *mapping_string);
i32 SDL_GameControllerAddMappingsFromRW(struct Rw_Ops *area, i32 freerw);
void SDL_GameControllerClose(struct Game_Controller *game_controller);
i32 SDL_GameControllerEventState(i32 state);
struct Game_Controller *SDL_GameControllerFromInstanceID(i32 joy_id);
i64 SDL_GameControllerGetAttached(struct Game_Controller *game_controller);
i16 SDL_GameControllerGetAxis(struct Game_Controller *game_controller, i64 axis);
u8 *SDL_GameControllerGetAxisFromString(u8 *pch_string);
struct Game_Controller_Button_Bind SDL_GameControllerGetBindForAxis(struct Game_Controller *game_controller, i64 axis);
struct Game_Controller_Button_Bind SDL_GameControllerGetBindForButton(struct Game_Controller *game_controller, i64 button);
u8 SDL_GameControllerGetButton(struct Game_Controller *game_controller, i64 button);
i64 SDL_GameControllerGetButtonFromString(u8 *pch_string);
struct Joystick *SDL_GameControllerGetJoystick(struct Game_Controller *game_controller);
u8 *SDL_GameControllerGetStringForAxis(i64 axis);
u8 *SDL_GameControllerGetStringForButton(i64 button);
u8 *SDL_GameControllerMapping(struct Game_Controller *game_controller);
struct Static_Array_16_u8 ;
struct Joystick_Guid;
u8 *SDL_GameControllerMappingForGUID(struct Joystick_Guid guid);
u8 *SDL_GameControllerName(struct Game_Controller *game_controller);
u8 *SDL_GameControllerNameForIndex(i32 joystick_index);
struct Game_Controller *SDL_GameControllerOpen(i32 joystick_index);
void SDL_GameControllerUpdate();
i64 (*SDL_GetAssertionHandler(void **userdata))(struct Assert_Data *, void *);
struct Assert_Data *SDL_GetAssertionReport();
u8 *SDL_GetAudioDeviceName(i32 index, i32 iscapture);
i64 SDL_GetAudioDeviceStatus(u32 dev);
u8 *SDL_GetAudioDriver(i32 index);
i64 SDL_GetAudioStatus();
u8 *SDL_GetBasePath();
i32 SDL_GetCPUCacheLineSize();
i32 SDL_GetCPUCount();
void SDL_GetClipRect(struct Surface *surface, struct Rect *rect);
u8 *SDL_GetClipboardText();
struct Display_Mode *SDL_GetClosestDisplayMode(i32 display_index, struct Display_Mode *mode, struct Display_Mode *closest);
i32 SDL_GetColorKey(struct Surface *surface, u32 *key);
u8 *SDL_GetCurrentAudioDriver();
i32 SDL_GetCurrentDisplayMode(i32 display_index, struct Display_Mode *mode);
u8 *SDL_GetCurrentVideoDriver();
struct Cursor *SDL_GetCursor();
i64 (*SDL_GetDefaultAssertionHandler())(struct Assert_Data *, void *);
struct Cursor *SDL_GetDefaultCursor();
i32 SDL_GetDesktopDisplayMode(i32 display_index, struct Display_Mode *mode);
i32 SDL_GetDisplayBounds(i32 display_index, struct Rect *rect);
i32 SDL_GetDisplayDPI(i32 display_index, f32 *ddpi, f32 *hdpi, f32 *vdpi);
i32 SDL_GetDisplayMode(i32 display_index, i32 mode_index, struct Display_Mode *mode);
u8 *SDL_GetDisplayName(i32 display_index);
i32 SDL_GetDisplayUsableBounds(i32 display_index, struct Rect *rect);
u8 *SDL_GetError();
i64 SDL_GetEventFilter(i32 (**filter)(void *, struct Event *), void **userdata);
u32 SDL_GetGlobalMouseState(i32 *x, i32 *y);
struct Window *SDL_GetGrabbedWindow();
u8 *SDL_GetHint(u8 *name);
i64 SDL_GetHintBoolean(u8 *name, i64 default_value);
i32 SDL_GetKeyFromName(u8 *name);
i32 SDL_GetKeyFromScancode(i64 scancode);
u8 *SDL_GetKeyName(i32 key);
struct Window *SDL_GetKeyboardFocus();
u8 *SDL_GetKeyboardState(i32 *numkeys);
i64 SDL_GetModState();
struct Window *SDL_GetMouseFocus();
u32 SDL_GetMouseState(i32 *x, i32 *y);
i32 SDL_GetNumAudioDevices(i32 iscapture);
i32 SDL_GetNumAudioDrivers();
i32 SDL_GetNumDisplayModes(i32 display_index);
i32 SDL_GetNumRenderDrivers();
i32 SDL_GetNumTouchDevices();
i32 SDL_GetNumTouchFingers(i64 touch_id);
i32 SDL_GetNumVideoDisplays();
i32 SDL_GetNumVideoDrivers();
u64 SDL_GetPerformanceCounter();
u64 SDL_GetPerformanceFrequency();
u8 *SDL_GetPixelFormatName(u32 format);
u8 *SDL_GetPlatform();
i64 SDL_GetPowerInfo(i32 secs, i32 *pct);
u8 *SDL_GetPrefPath(u8 *org, u8 *app);
u32 SDL_GetQueuedAudioSize(u32 dev);
void SDL_GetRGB(u32 pixel, struct Pixel_Format *format, u8 *r, u8 *g, u8 *b);
void SDL_GetRGBA(u32 pixel, struct Pixel_Format *format, u8 *r, u8 *g, u8 *b, u8 *a);
i64 SDL_GetRelativeMouseMode();
u32 SDL_GetRelativeMouseState(i32 *x, i32 *y);
i32 SDL_GetRenderDrawBlendMode(struct Renderer *renderer, i64 *blend_mode);
i32 SDL_GetRenderDrawColor(struct Renderer *renderer, u8 *r, u8 *g, u8 *b, u8 *a);
i32 SDL_GetRenderDriverInfo(i32 index, struct Renderer_Info *info);
struct Texture *SDL_GetRenderTarget(struct Renderer *renderer);
struct Renderer *SDL_GetRenderer(struct Window *window);
i32 SDL_GetRendererInfo(struct Renderer *renderer, struct Renderer_Info *info);
i32 SDL_GetRendererOutputSize(struct Renderer *renderer, i32 *w, i32 *h);
u8 *SDL_GetRevision();
i32 SDL_GetRevisionNumber();
i64 SDL_GetScancodeFromKey(i32 key);
i64 SDL_GetScancodeFromName(u8 *name);
u8 *SDL_GetScancodeName(i64 scancode);
i32 SDL_GetShapedWindowMode(struct Window *window, struct Window_Shape_Mode *shape_mode);
i32 SDL_GetSurfaceAlphaMod(struct Surface *surface, u8 *alpha);
i32 SDL_GetSurfaceBlendMode(struct Surface *surface, i64 *blend_mode);
i32 SDL_GetSurfaceColorMod(struct Surface *surface, u8 *r, u8 *g, u8 *b);
i32 SDL_GetSystemRAM();
i32 SDL_GetTextureAlphaMod(struct Texture *texture, u8 *alpha);
i32 SDL_GetTextureBlendMode(struct Texture *texture, i64 *blend_mode);
i32 SDL_GetTextureColorMod(struct Texture *texture, u8 *r, u8 *g, u8 *b);
u64 SDL_GetThreadID(struct Thread *thread);
u8 *SDL_GetThreadName(struct Thread *thread);
u32 SDL_GetTicks();
i64 SDL_GetTouchDevice(i32 index);
struct Finger *SDL_GetTouchFinger(i64 touch_id, i32 index);
void SDL_GetVersion(struct Version *ver);
u8 *SDL_GetVideoDriver(i32 index);
i32 SDL_GetWindowBordersSize(struct Window *window, i32 *top, i32 *left, i32 *bottom, i32 *right);
f32 SDL_GetWindowBrightness(struct Window *window);
void *SDL_GetWindowData(struct Window *window, u8 *name);
i32 SDL_GetWindowDisplayIndex(struct Window *window);
i32 SDL_GetWindowDisplayMode(struct Window *window, struct Display_Mode *mode);
u32 SDL_GetWindowFlags(struct Window *window);
struct Window *SDL_GetWindowFromID(u32 id);
i32 SDL_GetWindowGammaRamp(struct Window *window, u16 r, u16 g, u16 b);
i64 SDL_GetWindowGrab(struct Window *window);
u32 SDL_GetWindowID(struct Window *window);
void SDL_GetWindowMaximumSize(struct Window *window, i32 *w, i32 *h);
void SDL_GetWindowMinimumSize(struct Window *window, i32 *w, i32 *h);
i32 SDL_GetWindowOpacity(struct Window *window, f32 *opacity);
u32 SDL_GetWindowPixelFormat(struct Window *window);
void SDL_GetWindowPosition(struct Window *window, i32 *x, i32 *y);
void SDL_GetWindowSize(struct Window *window, i32 *w, i32 *h);
struct Surface *SDL_GetWindowSurface(struct Window *window);
u8 *SDL_GetWindowTitle(struct Window *window);
i64 SDL_GetWindowWMInfo(struct Window *window, struct Sys_Wm_Info *info);
void SDL_HapticClose(struct Haptic *haptic);
void SDL_HapticDestroyEffect(struct Haptic *haptic, i32 effect);
i32 SDL_HapticEffectSupported(struct Haptic *haptic, struct Haptic_Effect *effect);
i32 SDL_HapticGetEffectStatus(struct Haptic *haptic, i32 effect);
i32 SDL_HapticIndex(struct Haptic *haptic);
u8 *SDL_HapticName(i32 device_index);
i32 SDL_HapticNewEffect(struct Haptic *haptic, struct Haptic_Effect *effect);
i32 SDL_HapticNumAxes(struct Haptic *haptic);
i32 SDL_HapticNumEffects(struct Haptic *haptic);
i32 SDL_HapticNumEffectsPlaying(struct Haptic *haptic);
struct Haptic *SDL_HapticOpen(i32 device_index);
struct Haptic *SDL_HapticOpenFromJoystick(struct Joystick *joystick);
struct Haptic *SDL_HapticOpenFromMouse();
i32 SDL_HapticOpened(i32 device_index);
i32 SDL_HapticPause(struct Haptic *haptic);
u32 SDL_HapticQuery(struct Haptic *haptic);
i32 SDL_HapticRumbleInit(struct Haptic *haptic);
i32 SDL_HapticRumblePlay(struct Haptic *haptic, f32 strength, u32 length);
i32 SDL_HapticRumbleStop(struct Haptic *haptic);
i32 SDL_HapticRumbleSupported(struct Haptic *haptic);
i32 SDL_HapticRunEffect(struct Haptic *haptic, i32 effect, u32 iterations);
i32 SDL_HapticSetAutocenter(struct Haptic *haptic, i32 autocenter);
i32 SDL_HapticSetGain(struct Haptic *haptic, i32 gain);
i32 SDL_HapticStopAll(struct Haptic *haptic);
i32 SDL_HapticStopEffect(struct Haptic *haptic, i32 effect);
i32 SDL_HapticUnpause(struct Haptic *haptic);
i32 SDL_HapticUpdateEffect(struct Haptic *haptic, i32 effect, struct Haptic_Effect *data);
i64 SDL_Has3DNow();
i64 SDL_HasAVX();
i64 SDL_HasAVX2();
i64 SDL_HasAltiVec();
i64 SDL_HasClipboardText();
i64 SDL_HasEvent(u32 event_type);
i64 SDL_HasEvents(u32 min_type, u32 max_type);
i64 SDL_HasIntersection(struct Rect *a, struct Rect *b);
i64 SDL_HasMMX();
i64 SDL_HasRDTSC();
i64 SDL_HasSSE();
i64 SDL_HasSSE2();
i64 SDL_HasSSE3();
i64 SDL_HasSSE41();
i64 SDL_HasSSE42();
i64 SDL_HasScreenKeyboardSupport();
void SDL_HideWindow(struct Window *window);
i32 SDL_Init(i64 flags);
i32 SDL_InitSubSystem(u32 flags);
i64 SDL_IntersectRect(struct Rect *a, struct Rect *b, struct Rect *result);
i64 SDL_IntersectRectAndLine(struct Rect *rect, i32 *x1, i32 *y1, i32 *x2, i32 *y2);
i64 SDL_IsGameController(i32 joystick_index);
i64 SDL_IsScreenKeyboardShown(struct Window *window);
i64 SDL_IsScreenSaverEnabled();
struct Window;
i64 SDL_IsShapedWindow(struct Window window);
i64 SDL_IsTextInputActive();
void SDL_JoystickClose(struct Joystick *joystick);
i64 SDL_JoystickCurrentPowerLevel(struct Joystick *joystick);
i32 SDL_JoystickEventState(i32 state);
struct Joystick *SDL_JoystickFromInstanceID(i32 *joystick_id);
i64 SDL_JoystickGetAttached(struct Joystick *joystick);
i16 SDL_JoystickGetAxis(struct Joystick *joystick, i32 axis);
i32 SDL_JoystickGetBall(struct Joystick *joystick, i32 ball, i32 *dx, i32 *dy);
u8 SDL_JoystickGetButton(struct Joystick *joystick, i32 button);
struct Joystick_Guid SDL_JoystickGetDeviceGUID(i32 device_index);
struct Joystick_Guid SDL_JoystickGetGUID(struct Joystick *joystick);
struct Joystick_Guid SDL_JoystickGetGUIDFromString(u8 *pch_guid);
void SDL_JoystickGetGUIDString(struct Joystick_Guid guid, u8 *psz_guid, i32 cb_guid);
u8 SDL_JoystickGetHat(struct Joystick *joystick, i32 hat);
i32 SDL_JoystickInstanceID(struct Joystick *joystick);
i32 SDL_JoystickIsHaptic(struct Joystick *joystick);
u8 *SDL_JoystickName(struct Joystick *joystick);
u8 *SDL_JoystickNameForIndex(i32 device_index);
i32 SDL_JoystickNumAxes(struct Joystick *joystick);
i32 SDL_JoystickNumBalls(struct Joystick *joystick);
i32 SDL_JoystickNumButtons(struct Joystick *joystick);
i32 SDL_JoystickNumHats(struct Joystick *joystick);
struct Joystick *SDL_JoystickOpen(i32 device_index);
void SDL_JoystickUpdate();
struct Surface *SDL_LoadBMP_RW(struct Rw_Ops *src, i32 freerw);
i32 SDL_LoadDollarTemplates(i64 touch_id, struct Rw_Ops *src);
void *SDL_LoadFunction(void *handle, u8 *name);
u8 *SDL_LoadObject(u8 *sofile);
struct Audio_Spec *SDL_LoadWAV_RW(struct Rw_Ops *src, i32 freesrc, struct Audio_Spec *spec, u8 **audio_buf, u32 *audio_len);
void SDL_LockAudio();
void SDL_LockAudioDevice(u32 dev);
i32 SDL_LockMutex(struct Mutex *mutex);
i32 SDL_LockSurface(struct Surface *surface);
i32 SDL_LockTexture(struct Texture *texture, struct Rect *rect, void **pixels, i32 *pitch);
void SDL_Log(Slice fmt);
void SDL_LogCritical(i64 category, Slice fmt);
void SDL_LogDebug(i64 category, Slice fmt);
void SDL_LogError(i64 category, Slice fmt);
void SDL_LogGetOutputFunction(void (**callback)(void *, i64 , i64 , u8 *), void **userdata);
i64 SDL_LogGetPriority(i64 category);
void SDL_LogInfo(i64 category, Slice fmt);
void SDL_LogMessage(i64 category, i64 priority, Slice fmt);
void SDL_LogMessageV(i64 category, i64 priority, u8 *fmt, u8 *va_list);
void SDL_LogResetPriorities();
void SDL_LogSetAllPriority(i64 priority);
void SDL_LogSetOutputFunction(void (*callback)(void *, i64 , i64 , u8 *), void *userdata);
void SDL_LogSetPriority(i64 category, i64 priority);
void SDL_LogVerbose(i64 category, Slice fmt);
void SDL_LogWarn(i64 category, Slice fmt);
i32 SDL_LowerBlit(struct Surface *src, struct Rect *srcrect, struct Surface *dst, struct Rect *dstrect);
i32 SDL_LowerBlitScaled(struct Surface *src, struct Rect *srcrect, struct Surface *dst, struct Rect *dstrect);
u32 SDL_MapRGB(struct Pixel_Format *format, u8 r, u8 g, u8 b);
u32 SDL_MapRGBA(struct Pixel_Format *format, u8 r, u8 g, u8 b, u8 a);
u32 SDL_MasksToPixelFormatEnum(i32 bpp, u32 r_mask, u32 g_mask, u32 b_mask, u32 a_mask);
void SDL_MaximizeWindow(struct Window *window);
void SDL_MinimizeWindow(struct Window *window);
void SDL_MixAudio(u8 *dst, u8 *src, u32 len, i32 volume);
void SDL_MixAudioFormat(u8 *dst, u8 *src, u16 format, u32 len, i32 volume);
i32 SDL_MouseIsHaptic();
i32 SDL_NumHaptics();
i32 SDL_NumJoysticks();
i32 SDL_OpenAudio(struct Audio_Spec *desired, struct Audio_Spec *obtained);
u32 SDL_OpenAudioDevice(u8 *device, i32 iscapture, struct Audio_Spec *desired, struct Audio_Spec *obtained, i32 allowed_changed);
void SDL_PauseAudio(i32 pause_on);
void SDL_PauseAudioDevice(u32 dev, i32 pause_on);
i32 SDL_PeepEvents(struct Event *events, i32 num_events, i64 action, u32 min_type, u32 max_type);
i64 SDL_PixelFormatEnumToMasks(u32 format, i32 *bpp, u32 *r_mask, u32 *g_mask, u32 *b_mask, u32 *a_mask);
i32 SDL_PollEvent(struct Event *event);
void SDL_PumpEvents();
i32 SDL_PushEvent(struct Event *event);
i32 SDL_QueryTexture(struct Texture *texture, u32 *format, i32 *access, i32 *w, i32 *h);
i32 SDL_QueueAudio(u32 dev, void *data, u32 len);
void SDL_Quit();
void SDL_QuitSubSystem(u32 flags);
void SDL_RaiseWindow(struct Window *window);
u16 SDL_ReadBE16(struct Rw_Ops *src);
u32 SDL_ReadBE32(struct Rw_Ops *src);
u64 SDL_ReadBE64(struct Rw_Ops *src);
u16 SDL_ReadLE16(struct Rw_Ops *src);
u32 SDL_ReadLE32(struct Rw_Ops *src);
u64 SDL_ReadLE64(struct Rw_Ops *src);
u8 SDL_ReadU8(struct Rw_Ops *src);
i32 SDL_RecordGesture(i64 touch_id);
i32 SDL_RegisterApp(u8 *name, u32 style, void *h_inst);
u32 SDL_RegisterEvents(i32 num_events);
i64 SDL_RemoveTimer(i32 id);
i32 SDL_RenderClear(struct Renderer *renderer);
i32 SDL_RenderCopy(struct Renderer *renderer, struct Texture *texture, struct Rect *srcrect, struct Rect *dstrect);
i32 SDL_RenderCopyEx(struct Renderer *renderer, struct Texture *texture, struct Rect *srcrect, struct Rect *dstrect, f64 angle, struct Point *center, i64 flip);
i32 SDL_RenderDrawLine(struct Renderer *renderer, i32 x1, i32 y1, i32 x2, i32 y2);
i32 SDL_RenderDrawLines(struct Renderer *renderer, struct Point *points, i32 count);
i32 SDL_RenderDrawPoint(struct Renderer *renderer, i32 x, i32 y);
i32 SDL_RenderDrawPoints(struct Renderer *renderer, struct Point *points, i32 count);
i32 SDL_RenderDrawRect(struct Renderer *renderer, struct Rect *rect);
i32 SDL_RenderDrawRects(struct Renderer *renderer, struct Rect *rects, i32 count);
i32 SDL_RenderFillRect(struct Renderer *dst, struct Rect *rect);
i32 SDL_RenderFillRects(struct Renderer *dst, struct Rect *rect, i32 count);
void SDL_RenderGetClipRect(struct Surface *surface, struct Rect *rect);
struct IDirect3D_Device9 *SDL_RenderGetD3D9Device(struct Renderer *renderer);
i64 SDL_RenderGetIntegerScale(struct Renderer *renderer);
void SDL_RenderGetLogicalSize(struct Renderer *renderer, i32 *w, i32 *h);
void SDL_RenderGetScale(struct Renderer *renderer, f32 *scale_x, f32 *scale_y);
void SDL_RenderGetViewport(struct Renderer *renderer, struct Rect *rect);
i64 SDL_RenderIsClipEnabled(struct Renderer *renderer);
void SDL_RenderPresent(struct Renderer *renderer);
i32 SDL_RenderReadPixels(struct Renderer *renderer, struct Rect *rect, u32 format, void *pixels, i32 pitch);
i64 SDL_RenderSetClipRect(struct Surface *surface, struct Rect *rect);
i32 SDL_RenderSetIntegerScale(struct Renderer *renderer, i64 enable);
i32 SDL_RenderSetLogicalSize(struct Renderer *renderer, i32 w, i32 h);
i32 SDL_RenderSetScale(struct Renderer *renderer, f32 scale_x, f32 scale_y);
void SDL_RenderSetViewport(struct Renderer *renderer, struct Rect *rect);
i64 SDL_RenderTargetSupported(struct Renderer *renderer);
void SDL_ResetAssertionReport();
void SDL_RestoreWindow(struct Window *window);
struct Rw_Ops *SDL_RWFromConstMem(void *mem, i32 size);
struct Rw_Ops *SDL_RWFromFP(void *fp, i64 auto_close);
struct Rw_Ops *SDL_RWFromFile(u8 *file, u8 *mode);
struct Rw_Ops *SDL_RWFromMem(void *mem, i32 size);
i32 SDL_SaveAllDollarTemplates(struct Rw_Ops *dst);
i32 SDL_SaveBMP_RW(struct Surface *surface, struct Rw_Ops *dst, i32 free_dst);
i32 SDL_SaveDollarTemplate(i64 gesture_id, struct Rw_Ops *dst);
struct Sem;
i32 SDL_SemPost(struct Sem sem);
i32 SDL_SemTryWait(struct Sem sem);
u32 SDL_SemValue(struct Sem sem);
i32 SDL_SemWait(struct Sem sem);
i32 SDL_SemWaitTimeout(struct Sem sem, u32 ms);
void SDL_SetAssertionHandler(i64 (*handler)(struct Assert_Data *, void *), void *userdata);
i64 SDL_SetClipRect(struct Surface *surface, struct Rect *rect);
i32 SDL_SetClipboardText(u8 *text);
i32 SDL_SetColorKey(struct Surface *surface, i32 flag, u32 key);
void SDL_SetCursor(struct Cursor *cursor);
i32 SDL_SetError(Slice fmt);
void SDL_SetEventFilter(i32 (*filter)(void *, struct Event *), void *userdata);
i64 SDL_SetHint(u8 *name, u8 *value);
i64 SDL_SetHintWithPriority(u8 *name, u8 *value, i64 priority);
void SDL_SetMainReady();
void SDL_SetModState(i64 modstate);
i32 SDL_SetPaletteColors(struct Palette *palette, struct Color *colors, i32 firstcolor, i32 ncolors);
i32 SDL_SetPixelFormatPalette(struct Pixel_Format *format, struct Palette *palette);
i32 SDL_SetRelativeMouseMode(i64 enabled);
i32 SDL_SetRenderDrawBlendMode(struct Renderer *renderer, i64 blend_mode);
i32 SDL_SetRenderDrawColor(struct Renderer *renderer, u8 r, u8 g, u8 b, u8 a);
i32 SDL_SetRenderTarget(struct Renderer *renderer, struct Texture *texture);
i32 SDL_SetSurfaceAlphaMod(struct Surface *surface, u8 alpha);
i32 SDL_SetSurfaceBlendMode(struct Surface *surface, i64 blend_mode);
i32 SDL_SetSurfaceColorMod(struct Surface *surface, u8 r, u8 g, u8 b);
i32 SDL_SetSurfacePalette(struct Surface *surface, struct Palette *palette);
i32 SDL_SetSurfaceRLE(struct Surface *surface, i32 flag);
void SDL_SetTextInputRect(struct Rect *rect);
i32 SDL_SetTextureAlphaMod(struct Texture *texture, u8 alpha);
i32 SDL_SetTextureBlendMode(struct Texture *texture, i64 blend_mode);
i32 SDL_SetTextureColorMod(struct Texture *texture, u8 r, u8 g, u8 b);
i32 SDL_SetThreadPriority(i64 priority);
void SDL_SetWindowBordered(struct Window *window, i64 bordered);
i32 SDL_SetWindowBrightness(struct Window *window, f32 brightness);
void *SDL_SetWindowData(struct Window *window, u8 *name, void *userdata);
i32 SDL_SetWindowDisplayMode(struct Window *window, struct Display_Mode *mode);
i32 SDL_SetWindowFullscreen(struct Window *window, u32 flags);
i32 SDL_SetWindowGammaRamp(struct Window *window, u16 *r, u16 *g, u16 *b);
void SDL_SetWindowGrab(struct Window *window, i64 grabbed);
i32 SDL_SetWindowHitTest(struct Window *window, i64 (*callback)(struct Window *, struct Point *, void *), void *callback_data);
void SDL_SetWindowIcon(struct Window *window, struct Surface *icon);
i32 SDL_SetWindowInputFocus(struct Window *window);
void SDL_SetWindowMaximumSize(struct Window *window, i32 w, i32 h);
void SDL_SetWindowMinimumSize(struct Window *window, i32 w, i32 h);
i32 SDL_SetWindowModalFor(struct Window *window, struct Window *parent_window);
i32 SDL_SetWindowOpacity(struct Window *window, f32 opacity);
void SDL_SetWindowPosition(struct Window *window, i32 x, i32 y);
void SDL_SetWindowResizable(struct Window *window, i64 resizable);
struct Color;
struct Window_Shape_Params;
struct Window_Shape_Mode;
i32 SDL_SetWindowShape(struct Window *window, struct Surface *shape, struct Window_Shape_Mode shape_mode);
void SDL_SetWindowSize(struct Window *window, i32 w, i32 h);
void SDL_SetWindowTitle(struct Window *window, u8 *title);
void SDL_SetWindowsMessageHook(void (*callback)(void *, void *, u32 , u64 , i64 ), void *userdata);
i32 SDL_ShowCursor(i32 toggle);
i32 SDL_ShowMessageBox(struct Message_Box_Data *message_box_data, i32 *button_id);
i32 SDL_ShowSimpleMessageBox(u32 flags, u8 *title, u8 *message, struct Window *window);
void SDL_ShowWindow(struct Window *window);
i32 SDL_SoftStretch(struct Surface *src, struct Rect *srcrect, struct Surface *dst, struct Rect *dstrect);
void SDL_StartTextInput();
void SDL_StopTextInput();
u32 SDL_TLSCreate();
void *SDL_TLSGet(u32 id);
i32 SDL_TLSSet(u32 id, void *value, void (*destructor)(void *));
u64 SDL_ThreadID();
i32 SDL_TryLockMutex(struct Mutex *mutex);
void SDL_UnionRect(struct Rect *a, struct Rect *b, struct Rect *result);
void SDL_UnloadObject(void *handle);
void SDL_UnlockAudio();
void SDL_UnlockAudioDevice(u32 dev);
i32 SDL_UnlockMutex(struct Mutex *mutex);
void SDL_UnlockSurface(struct Surface *surface);
void SDL_UnlockTexture(struct Texture *texture);
void SDL_UnregisterApp();
void SDL_UpdateTexture(struct Texture *texture, struct Rect *rect, void *pixels, i32 pitch);
i32 SDL_UpdateWindowSurface(struct Window *window);
i32 SDL_UpdateWindowSurfaceRects(struct Window *window, struct Rect *rects, i32 num_rects);
i32 SDL_UpdateYUVTexture(struct Texture *texture, struct Rect *rect, u8 *y_plane, i32 y_pitch, u8 *u_plane, i32 u_pitch, u8 *v_plane, i32 v_pitch);
i32 SDL_UpperBlit(struct Surface *src, struct Rect *srcrect, struct Surface *dst, struct Rect *dstrect);
i32 SDL_UpperBlitScaled(struct Surface *src, struct Rect *srcrect, struct Surface *dst, struct Rect *dstrect);
i32 SDL_VideoInit(u8 *driver_name);
void SDL_VideoQuit();
i32 SDL_WaitEvent(struct Event *event);
i32 SDL_WaitEventTimeout(struct Event *event, i32 timeout);
void SDL_WaitThread(struct Thread *thread, i32 *status);
i32 SDL_WarpMouseGlobal(i32 x, i32 y);
void SDL_WarpMouseInWindow(struct Window *window, i32 x, i32 y);
u32 SDL_WasInit(u32 flags);
u64 SDL_WriteBE16(struct Rw_Ops *dst, u16 value);
u64 SDL_WriteBE32(struct Rw_Ops *dst, u32 value);
u64 SDL_WriteBE64(struct Rw_Ops *dst, u64 value);
u64 SDL_WriteLE16(struct Rw_Ops *dst, u16 value);
u64 SDL_WriteLE32(struct Rw_Ops *dst, u32 value);
u64 SDL_WriteLE64(struct Rw_Ops *dst, u64 value);
u64 SDL_WriteU8(struct Rw_Ops *dst, u8 value);
void log_sdl_error();
struct Rect;
struct Surface;
i32 main();
struct Blit_Map;
struct Renderer;
struct Texture;
struct Cond;
struct Mutex;
struct Thread;
struct Haptic;
struct Joystick;
struct Game_Controller;
struct Cursor;
struct IDirect3D_Device9;
struct Rw_Ops;
struct Sys_Wm_Info;
struct Sys_Wm_Msg;
struct Anonymous_Struct_2;
struct Anonymous_Struct_1;
struct Game_Controller_Button_Bind;
struct Message_Box_Data;
struct Message_Box_Button_Data;
struct Message_Box_Color;
struct Static_Array_5_Message_Box_Color_ ;
struct Message_Box_Color_Scheme;
struct Assert_Data;
struct Point;
struct Static_Array_16_u32 ;
struct Renderer_Info;
struct Version;
struct Display_Mode;
struct Finger;
struct Audio_Spec;
struct Static_Array_10_function_pointer_void_parameters_ptr_Audio_Cvt__u16 ;
struct Audio_Cvt;
struct Palette;
struct Static_Array_2_u8 ;
struct Pixel_Format;
struct Atomic;
struct Keysym;
struct Static_Array_3_i32 ;
struct Haptic_Direction;
struct Haptic_Constant;
struct Haptic_Periodic;
struct Static_Array_3_u16 ;
struct Static_Array_3_i16 ;
struct Haptic_Condition;
struct Haptic_Ramp;
struct Haptic_Left_Right;
struct Haptic_Custom;
struct Haptic_Effect;
struct Common_Event;
struct Window_Event;
struct Keyboard_Event;
struct Static_Array_32_u8 ;
struct Text_Editing_Event;
struct Text_Input_Event;
struct Mouse_Motion_Event;
struct Mouse_Button_Event;
struct Mouse_Wheel_Event;
struct Joy_Axis_Event;
struct Joy_Ball_Event;
struct Joy_Hat_Event;
struct Joy_Button_Event;
struct Joy_Device_Event;
struct Controller_Axis_Event;
struct Controller_Button_Event;
struct Controller_Device_Event;
struct Audio_Device_Event;
struct Quit_Event;
struct User_Event;
struct Sys_Wm_Event;
struct Touch_Finger_Event;
struct Multi_Gesture_Event;
struct Dollar_Gesture_Event;
struct Drop_Event;
struct Static_Array_56_u8 ;
struct Event;
struct OS_Event;

// Actual declarations
void print(String fmt, Slice args) {
    i64 __t1 = 0;
    i64 arg_index = __t1;
    {
        i64 __t2 = 0;
        i64 i = __t2;
        while (true) {
            bool __t3 = i < fmt.count;
            if (!__t3) { break; }
            u8 c = fmt.data[i];
            u8 __t5 = 37;
            bool __t4 = c == __t5;
            if (__t4) {
                i64 __t9 = 1;
                i64 __t8 = i + __t9;
                i64 __t7 = __t8;
                bool __t6 = __t7 < fmt.count;
                if (__t6) {
                    i64 __t12 = 1;
                    i64 __t11 = i + __t12;
                    u8 __t13 = 37;
                    bool __t10 = fmt.data[__t11] == __t13;
                    if (__t10) {
                        u8 __t14 = 37;
                        print_char(__t14);
                        i64 __t15 = 1;
                        i += __t15;
                        continue;
                    }
                }
                Any arg = ((Any *)args.data)[arg_index];
                i64 __t17 = 4;
                bool __t16 = arg.type == __t17;
                if (__t16) {
                    i64 *__t18 = ((i64 *)arg.data);
                    print_int((*__t18));
                }
                else {
                    i64 __t20 = 9;
                    bool __t19 = arg.type == __t20;
                    if (__t19) {
                        f32 *__t21 = ((f32 *)arg.data);
                        print_float((*__t21));
                    }
                    else {
                        i64 __t23 = 13;
                        bool __t22 = arg.type == __t23;
                        if (__t22) {
                            String *__t24 = ((String *)arg.data);
                            print_string((*__t24));
                        }
                        else {
                            i64 __t26 = 11;
                            bool __t25 = arg.type == __t26;
                            if (__t25) {
                                bool *__t27 = ((bool *)arg.data);
                                print_bool((*__t27));
                            }
                        }
                    }
                }
                i64 __t28 = 1;
                arg_index += __t28;
            }
            else {
                print_char(c);
            }
            i64 __t29 = 1;
            i += __t29;
        }
    }
}
void printa(Slice args) {
    {
        i64 __t30 = 0;
        i64 i = __t30;
        while (true) {
            bool __t31 = i < args.count;
            if (!__t31) { break; }
            i64 __t33 = 0;
            bool __t32 = i != __t33;
            if (__t32) {
                u8 __t34 = 32;
                print_char(__t34);
            }
            Any arg = ((Any *)args.data)[i];
            i64 __t36 = 4;
            bool __t35 = arg.type == __t36;
            if (__t35) {
                i64 *__t37 = ((i64 *)arg.data);
                print_int((*__t37));
            }
            else {
                i64 __t39 = 9;
                bool __t38 = arg.type == __t39;
                if (__t38) {
                    f32 *__t40 = ((f32 *)arg.data);
                    print_float((*__t40));
                }
                else {
                    i64 __t42 = 13;
                    bool __t41 = arg.type == __t42;
                    if (__t41) {
                        String *__t43 = ((String *)arg.data);
                        print_string((*__t43));
                    }
                    else {
                        i64 __t45 = 11;
                        bool __t44 = arg.type == __t45;
                        if (__t44) {
                            bool *__t46 = ((bool *)arg.data);
                            print_bool((*__t46));
                        }
                    }
                }
            }
            i64 __t47 = 1;
            i += __t47;
        }
    }
    u8 __t48 = 10;
    print_char(__t48);
}
bool string_eq(String a, String b) {
    bool __t49 = a.count != b.count;
    if (__t49) {
        bool __t50 = false;
        return __t50;
    }
    {
        i64 __t51 = 0;
        i64 i = __t51;
        while (true) {
            bool __t52 = i < a.count;
            if (!__t52) { break; }
            bool __t53 = a.data[i] != b.data[i];
            if (__t53) {
                bool __t54 = false;
                return __t54;
            }
            i64 __t55 = 1;
            i += __t55;
        }
    }
    bool __t56 = true;
    return __t56;
}
String string_ptr(u8 *ptr, i64 count) {
    String str = {0};
    str.data = ptr;
    str.count = count;
    return str;
}
struct Static_Array_16_u8  {
    u8 elements[16];
};
struct Joystick_Guid {
    struct Static_Array_16_u8 data;
};
struct Window {
    bool __dummy;
};
struct Sem {
    bool __dummy;
};
struct Color {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};
struct Window_Shape_Params {
    u8 binarization_cutoff;
    struct Color color_key;
};
struct Window_Shape_Mode {
    i64 mode;
    struct Window_Shape_Params parameters;
};
void log_sdl_error() {
    u8 *__t57 = SDL_GetError();
    u8 *error = __t57;
    u64 __t58 = strlen(error);
    i64 __t59 = ((i64 )__t58);
    String __t60 = string_ptr(error, __t59);
    String str = __t60;
    String __t61 = MAKE_STRING("SDL ERROR: %\n", 13);
    Any __t62[1];
    __t62[0] = MAKE_ANY(&str.count, 4);
    Slice __t63;
    __t63.data = __t62;
    __t63.count = 1;
    print(__t61, __t63);
}
struct Rect {
    i32 x;
    i32 y;
    i32 w;
    i32 h;
};
struct Surface {
    u32 flags;
    struct Pixel_Format *format;
    i32 w;
    i32 h;
    i32 pitch;
    void *pixels;
    void *userdata;
    i32 locked;
    void *lock_data;
    struct Rect clip_rect;
    struct Blit_Map *blip_map;
    i32 refcount;
};
i32 main() {
    i64 __t65 = 0;
    i32 __t66 = SDL_Init(__t65);
    i32 __t67 = 0;
    bool __t64 = __t66 < __t67;
    if (__t64) {
        log_sdl_error();
        i32 __t68 = 1;
        return __t68;
    }
    String __t69 = MAKE_STRING("My Fancy Window", 15);
    String name = __t69;
    i64 __t70 = 0;
    i32 __t71 = 0;
    i32 __t72 = 0;
    i32 __t73 = 1920;
    i32 __t74 = 1080;
    i64 __t75 = 4;
    struct Window *__t76 = SDL_CreateWindow(&name.data[__t70], __t71, __t72, __t73, __t74, __t75);
    struct Window *window = __t76;
    bool __t77 = window == NULL;
    if (__t77) {
        log_sdl_error();
        i32 __t78 = 1;
        return __t78;
    }
    String __t79 = MAKE_STRING("%\n", 2);
    Any __t80[1];
    i64 __t81 = 15;
    __t80[0] = MAKE_ANY(&__t81, 4);
    Slice __t82;
    __t82.data = __t80;
    __t82.count = 1;
    print(__t79, __t82);
    struct Surface *__t83 = SDL_GetWindowSurface(window);
    struct Surface *screen_surface = __t83;
    u8 __t84 = 255;
    u8 __t85 = 0;
    u8 __t86 = 255;
    u32 __t87 = SDL_MapRGB(screen_surface->format, __t84, __t85, __t86);
    i32 __t88 = SDL_FillRect(screen_surface, NULL, __t87);
    i32 __t89 = SDL_UpdateWindowSurface(window);
    u32 __t90 = 2000;
    SDL_Delay(__t90);
}
struct Blit_Map {
    bool __dummy;
};
struct Renderer {
    bool __dummy;
};
struct Texture {
    bool __dummy;
};
struct Cond {
    bool __dummy;
};
struct Mutex {
    bool __dummy;
};
struct Thread {
    bool __dummy;
};
struct Haptic {
    bool __dummy;
};
struct Joystick {
    bool __dummy;
};
struct Game_Controller {
    bool __dummy;
};
struct Cursor {
    bool __dummy;
};
struct IDirect3D_Device9 {
    bool __dummy;
};
struct Rw_Ops {
    bool __dummy;
};
struct Sys_Wm_Info {
    bool __dummy;
};
struct Sys_Wm_Msg {
    bool __dummy;
};
struct Anonymous_Struct_2 {
    i32 hat;
    i32 mask;
};
struct Anonymous_Struct_1 {
    i32 button;
    i32 axis;
    struct Anonymous_Struct_2 hat_mask;
};
struct Game_Controller_Button_Bind {
    i64 bind_type;
    struct Anonymous_Struct_1 value;
};
struct Message_Box_Data {
    u32 flags;
    struct Window *window;
    u8 *title;
    u8 *message;
    i32 num_buttons;
    struct Message_Box_Button_Data *buttons;
    struct Message_Box_Color_Scheme *color_scheme;
};
struct Message_Box_Button_Data {
    u32 flags;
    i32 button_id;
    u8 *text;
};
struct Message_Box_Color {
    u8 r;
    u8 g;
    u8 b;
};
struct Static_Array_5_Message_Box_Color_  {
    struct Message_Box_Color elements[5];
};
struct Message_Box_Color_Scheme {
    struct Static_Array_5_Message_Box_Color_ colors;
};
struct Assert_Data {
    i32 always_ignore;
    u32 trigger_count;
    u8 *condition;
    u8 *filename;
    i32 linenum;
    u8 *function;
    struct Assert_Data *next;
};
struct Point {
    i32 x;
    i32 y;
};
struct Static_Array_16_u32  {
    u32 elements[16];
};
struct Renderer_Info {
    u8 *name;
    u32 flags;
    u32 num_texture_formats;
    struct Static_Array_16_u32 texture_formats;
    i32 max_texture_width;
    i32 max_texture_height;
};
struct Version {
    u8 major;
    u8 minor;
    u8 patch;
};
struct Display_Mode {
    u32 format;
    i32 w;
    i32 h;
    i32 refresh_rate;
    void *driver_data;
};
struct Finger {
    i64 id;
    f32 x;
    f32 y;
    f32 pressure;
};
struct Audio_Spec {
    i32 freq;
    u16 format;
    u8 channels;
    u8 silence;
    u16 samples;
    u16 padding;
    u32 size;
    void (*callback)(void *, u8 *, i32 );
    void *userdata;
};
struct Static_Array_10_function_pointer_void_parameters_ptr_Audio_Cvt__u16  {
    void (*elements[10])(struct Audio_Cvt *, u16 );
};
struct Audio_Cvt {
    i32 needed;
    u16 src_format;
    u16 dst_format;
    i64 rate_incr;
    u8 *buf;
    i32 len;
    i32 len_cvt;
    i32 len_mult;
    i64 len_ratio;
    struct Static_Array_10_function_pointer_void_parameters_ptr_Audio_Cvt__u16 filters;
    i32 filter_index;
};
struct Palette {
    i32 num_colors;
    struct Color *colors;
    u32 version;
    i32 ref_count;
};
struct Static_Array_2_u8  {
    u8 elements[2];
};
struct Pixel_Format {
    u32 format;
    struct Palette *palette;
    u8 bits_per_pixel;
    u8 bytes_per_pixel;
    struct Static_Array_2_u8 padding;
    u32 r_mask;
    u32 g_mask;
    u32 b_mask;
    u32 a_mask;
    u8 r_loss;
    u8 g_loss;
    u8 b_loss;
    u8 a_loss;
    u8 r_shift;
    u8 g_shift;
    u8 b_shift;
    u8 a_shift;
    i32 ref_count;
    struct Pixel_Format *next;
};
struct Atomic {
    i32 value;
};
struct Keysym {
    i64 scancode;
    i32 sym;
    u16 mod;
    u32 unused;
};
struct Static_Array_3_i32  {
    i32 elements[3];
};
struct Haptic_Direction {
    u8 haptic_type;
    struct Static_Array_3_i32 dir;
};
struct Haptic_Constant {
    u16 haptic_type;
    struct Haptic_Direction direction;
    u32 length;
    u16 delay;
    u16 button;
    u16 interval;
    i16 level;
    u16 attack_length;
    u16 attack_level;
    u16 fade_length;
    u16 fade_level;
};
struct Haptic_Periodic {
    u16 haptic_type;
    struct Haptic_Direction direction;
    u32 length;
    u16 delay;
    u16 button;
    u16 interval;
    u16 period;
    i16 magnitude;
    i16 offset;
    u16 phase;
    u16 attack_length;
    u16 attack_level;
    u16 fade_length;
    u16 fade_level;
};
struct Static_Array_3_u16  {
    u16 elements[3];
};
struct Static_Array_3_i16  {
    i16 elements[3];
};
struct Haptic_Condition {
    u16 haptic_type;
    struct Haptic_Direction direction;
    u32 length;
    u16 delay;
    u16 button;
    u16 interval;
    struct Static_Array_3_u16 right_sat;
    struct Static_Array_3_u16 left_sat;
    struct Static_Array_3_i16 right_coeff;
    struct Static_Array_3_i16 left_coeff;
    struct Static_Array_3_u16 dead_band;
    struct Static_Array_3_i16 center;
};
struct Haptic_Ramp {
    u16 haptic_type;
    struct Haptic_Direction direction;
    u32 length;
    u16 delay;
    u16 button;
    u16 interval;
    i16 start;
    i16 end;
    u16 attack_length;
    u16 attack_level;
    u16 fade_length;
    u16 fade_level;
};
struct Haptic_Left_Right {
    u16 haptic_type;
    u32 length;
    u16 large_magnitude;
    u16 small_magnitude;
};
struct Haptic_Custom {
    u16 haptic_type;
    struct Haptic_Direction direction;
    u32 length;
    u16 delay;
    u16 button;
    u16 interval;
    u8 channels;
    u16 period;
    u16 samples;
    u16 *data;
    u16 attack_length;
    u16 attack_level;
    u16 fade_length;
    u16 fade_level;
};
struct Haptic_Effect {
    u16 haptic_type;
    struct Haptic_Constant constant;
    struct Haptic_Periodic periodic;
    struct Haptic_Condition condition;
    struct Haptic_Ramp ramp;
    struct Haptic_Left_Right left_right;
    struct Haptic_Custom custom;
};
struct Common_Event {
    i64 type;
    u32 timestamp;
};
struct Window_Event {
    i64 type;
    u32 timestamp;
    u32 window_id;
    i64 event;
    u8 padding1;
    u8 padding2;
    u8 padding3;
    i32 data1;
    i32 data2;
};
struct Keyboard_Event {
    i64 type;
    u32 timestamp;
    u32 window_id;
    u8 state;
    u8 repeat;
    u8 padding2;
    u8 padding3;
    struct Keysym keysym;
};
struct Static_Array_32_u8  {
    u8 elements[32];
};
struct Text_Editing_Event {
    i64 type;
    u32 timestamp;
    u32 window_id;
    struct Static_Array_32_u8 text;
    i32 start;
    i32 length;
};
struct Text_Input_Event {
    i64 type;
    u32 timestamp;
    u32 window_id;
    struct Static_Array_32_u8 text;
};
struct Mouse_Motion_Event {
    i64 type;
    u32 timestamp;
    u32 window_id;
    u32 which;
    u32 state;
    i32 x;
    i32 y;
    i32 xrel;
    i32 yrel;
};
struct Mouse_Button_Event {
    i64 type;
    u32 timestamp;
    u32 window_id;
    u32 which;
    u8 button;
    u8 state;
    u8 clicks;
    u8 padding1;
    i32 x;
    i32 y;
};
struct Mouse_Wheel_Event {
    i64 type;
    u32 timestamp;
    u32 window_id;
    u32 which;
    i32 x;
    i32 y;
    u32 direction;
};
struct Joy_Axis_Event {
    i64 type;
    u32 timestamp;
    i32 which;
    u8 axis;
    u8 padding1;
    u8 padding2;
    u8 padding3;
    i16 value;
    u16 padding4;
};
struct Joy_Ball_Event {
    i64 type;
    u32 timestamp;
    i32 which;
    u8 ball;
    u8 padding1;
    u8 padding2;
    u8 padding3;
    i16 xrel;
    i16 yrel;
};
struct Joy_Hat_Event {
    i64 type;
    u32 timestamp;
    i32 which;
    u8 hat;
    u8 value;
    u8 padding1;
    u8 padding2;
};
struct Joy_Button_Event {
    i64 type;
    u32 timestamp;
    i32 which;
    u8 button;
    u8 state;
    u8 padding1;
    u8 padding2;
};
struct Joy_Device_Event {
    i64 type;
    u32 timestamp;
    i32 which;
};
struct Controller_Axis_Event {
    i64 type;
    u32 timestamp;
    i32 which;
    u8 axis;
    u8 padding1;
    u8 padding2;
    u8 padding3;
    i16 value;
    u16 padding4;
};
struct Controller_Button_Event {
    i64 type;
    u32 timestamp;
    i32 which;
    u8 button;
    u8 state;
    u8 padding1;
    u8 padding2;
};
struct Controller_Device_Event {
    i64 type;
    u32 timestamp;
    i32 which;
};
struct Audio_Device_Event {
    i64 type;
    u32 timestamp;
    u32 which;
    u8 iscapture;
    u8 padding1;
    u8 padding2;
    u8 padding3;
};
struct Quit_Event {
    i64 type;
    u32 timestamp;
};
struct User_Event {
    i64 type;
    u32 timestamp;
    u32 window_id;
    i32 code;
    void **data1;
    void **data2;
};
struct Sys_Wm_Event {
    i64 type;
    u32 timestamp;
    struct Sys_Wm_Msg *msg;
};
struct Touch_Finger_Event {
    i64 type;
    u32 timestamp;
    i64 touch_id;
    i64 finger_id;
    f32 x;
    f32 y;
    f32 dx;
    f32 dy;
    f32 pressure;
};
struct Multi_Gesture_Event {
    i64 type;
    u32 timestamp;
    i64 touch_id;
    f32 d_theta;
    f32 d_dist;
    f32 x;
    f32 y;
    u16 num_fingers;
    u16 padding;
};
struct Dollar_Gesture_Event {
    i64 type;
    u32 timestamp;
    i64 touch_id;
    i64 gesture_id;
    u32 num_fingers;
    f32 error;
    f32 x;
    f32 y;
};
struct Drop_Event {
    i64 type;
    u32 timestamp;
    u8 *file;
    u32 window_id;
};
struct Static_Array_56_u8  {
    u8 elements[56];
};
struct Event {
    i64 type;
    struct Common_Event common;
    struct Window_Event window;
    struct Keyboard_Event key;
    struct Text_Editing_Event edit;
    struct Text_Input_Event text;
    struct Mouse_Motion_Event motion;
    struct Mouse_Button_Event button;
    struct Mouse_Wheel_Event wheel;
    struct Joy_Axis_Event jaxis;
    struct Joy_Ball_Event jball;
    struct Joy_Hat_Event jhat;
    struct Joy_Button_Event jbutton;
    struct Joy_Device_Event jdevice;
    struct Controller_Axis_Event caxis;
    struct Controller_Button_Event cbutton;
    struct Controller_Device_Event cdevice;
    struct Audio_Device_Event adevice;
    struct Quit_Event quit;
    struct User_Event user;
    struct Sys_Wm_Event syswm;
    struct Touch_Finger_Event tfinger;
    struct Multi_Gesture_Event mgesture;
    struct Dollar_Gesture_Event dgesture;
    struct Drop_Event drop;
    struct Static_Array_56_u8 padding;
};
struct OS_Event {
    i64 type;
    u32 timestamp;
};
