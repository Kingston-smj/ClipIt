#include "global_hotkey_x11.h"
#include "app/logging.h"

#include <QGuiApplication>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

// XCB KeyPress event opcode
#ifndef XCB_KEY_PRESS
#  define XCB_KEY_PRESS 2
#endif

namespace platform {

// Super (Mod4) + V
static constexpr unsigned int kTargetMod = Mod4Mask;  // Super key
static constexpr KeySym       kTargetSym = XK_v;

GlobalHotkeyX11::GlobalHotkeyX11(QObject* parent)
    : GlobalHotkey(parent)
{}

GlobalHotkeyX11::~GlobalHotkeyX11()
{
    if (grabbed_)
        ungrabKey();

    qApp->removeNativeEventFilter(this);
}

void GlobalHotkeyX11::start()
{
    grabKey();

    if (grabbed_)
    {
        qApp->installNativeEventFilter(this);
        app::log_info("GlobalHotkeyX11: Super+V grabbed (keycode=%u mod=0x%x)",
                      keycode_, modifiers_);
    }
}

// ── XGrabKey ─────────────────────────────────────────────────────────────────

void GlobalHotkeyX11::grabKey()
{
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy)
    {
        app::log_warn("GlobalHotkeyX11: cannot open X display");
        return;
    }

    Window root   = DefaultRootWindow(dpy);
    keycode_      = XKeysymToKeycode(dpy, kTargetSym);
    modifiers_    = kTargetMod;

    // Grab with all combinations of NumLock / CapsLock / ScrollLock masks
    // so the hotkey fires regardless of those lock states.
    static const unsigned int lock_masks[] = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};

    XGrabKey(dpy, keycode_, modifiers_,          root, True, GrabModeAsync, GrabModeAsync);
    for (unsigned int extra : lock_masks)
    {
        XGrabKey(dpy, keycode_, modifiers_ | extra, root, True, GrabModeAsync, GrabModeAsync);
    }

    XFlush(dpy);
    XCloseDisplay(dpy);
    grabbed_ = true;
}

void GlobalHotkeyX11::ungrabKey()
{
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return;

    Window root = DefaultRootWindow(dpy);
    static const unsigned int lock_masks[] = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};

    XUngrabKey(dpy, keycode_, modifiers_, root);
    for (unsigned int extra : lock_masks)
        XUngrabKey(dpy, keycode_, modifiers_ | extra, root);

    XFlush(dpy);
    XCloseDisplay(dpy);
    grabbed_ = false;
}

// ── Native event filter ───────────────────────────────────────────────────────

bool GlobalHotkeyX11::nativeEventFilter(const QByteArray& eventType,
                                         void*             message,
                                         qintptr*          /*result*/)
{
    if (eventType != "xcb_generic_event_t")
        return false;

    auto* ev = static_cast<xcb_generic_event_t*>(message);
    if ((ev->response_type & ~0x80) != XCB_KEY_PRESS)
        return false;

    auto* kp = static_cast<xcb_key_press_event_t*>(message);

    // Mask off lock bits before comparing modifiers.
    static constexpr unsigned int lock_mask = LockMask | Mod2Mask;
    const unsigned int pressed_mod = kp->state & ~lock_mask;

    if (kp->detail == keycode_ && pressed_mod == modifiers_)
    {
        emit activated();
        return true;  // consume the event
    }

    return false;
}

} // namespace platform
