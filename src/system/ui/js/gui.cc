#include <emscripten.h>
#include "system/display.h"
#include "system/keyboard.h"
#include "system/mouse.h"
#include "system/sysclk.h"
#include "system/ui/gui.h"

#include "sysjs.h"

static void (*gProcessCudaEvents)();

void sys_gui_init() {}

bool sys_gui_open_file_dialog(String &ret, const String &title, const String &filespec, const String &filespecname, const String &home, bool existing) {
    printf("sys_gui_open_file_dialog called with title: %s, filespec: %s, filespecname: %s, home: %s, existing: %d\n", title.contentChar(), filespec.contentChar(), filespecname.contentChar(), home.contentChar(), existing);
    return false; // Placeholder return value
}

int sys_gui_messagebox(const String &title, const String &text, int buttons) {
    printf("sys_gui_messagebox called with title: %s, text: %s, buttons: %d\n", title.contentChar(), text.contentChar(), buttons);
    return 0; // Placeholder return value
}

// doProcessCudaEvent actually expects each mouse event to have the full state
// (position and buttons), so keep track of what we sent to when we get updates.
static SystemEvent gLastMouseEvent;

static void sys_gui_poll_events() {
    int lock = EM_ASM_INT_V({ return workerApi.acquireInputLock(); });
    if (!lock) {
        return;
    }

    int mouseButtonState = EM_ASM_INT_V({
        return workerApi.getInputValue(workerApi.InputBufferAddresses.mouseButtonStateAddr);
    });
    int mouseButton2State = EM_ASM_INT_V({
        return workerApi.getInputValue(workerApi.InputBufferAddresses.mouseButton2StateAddr);
    });
    if (mouseButtonState > -1 || mouseButton2State > -1) {
        SystemEvent ev;
        ev.type = sysevMouse;
        ev.mouse.type = mouseButtonState == 1 || mouseButton2State == 1 ? sme_buttonPressed : sme_buttonReleased;

        if (mouseButtonState != -1) {
            ev.mouse.button1 = mouseButtonState == 1;
            gLastMouseEvent.mouse.button1 = ev.mouse.button1;
        } else {
            ev.mouse.button1 = gLastMouseEvent.mouse.button1;
        }
        if (mouseButton2State != -1) {
            ev.mouse.button2 = mouseButton2State == 1;
            gLastMouseEvent.mouse.button2 = ev.mouse.button2;
        } else {
            ev.mouse.button2 = gLastMouseEvent.mouse.button2;
        }
        ev.mouse.button3 = false;

        // Make sure we don't generate position updates
        ev.mouse.x = gLastMouseEvent.mouse.x;
        ev.mouse.y = gLastMouseEvent.mouse.y;
        ev.mouse.relx = 0;
        ev.mouse.rely = 0;
        gMouse->handleEvent(ev);
    }

    int hasMousePosition = EM_ASM_INT_V({
        return workerApi.getInputValue(workerApi.InputBufferAddresses.mousePositionFlagAddr);
    });
    if (hasMousePosition) {
        SystemEvent ev;
        ev.type = sysevMouse;
        ev.mouse.type = sme_motionNotify;
        ev.mouse.relx  = EM_ASM_INT_V({
            return workerApi.getInputValue(workerApi.InputBufferAddresses.mouseDeltaXAddr);
        });
        ev.mouse.rely = EM_ASM_INT_V({
            return workerApi.getInputValue(workerApi.InputBufferAddresses.mouseDeltaYAddr);
        });
        ev.mouse.x = EM_ASM_INT_V({
            return workerApi.getInputValue(workerApi.InputBufferAddresses.mousePositionXAddr);
        });
        ev.mouse.y = EM_ASM_INT_V({
            return workerApi.getInputValue(workerApi.InputBufferAddresses.mousePositionYAddr);
        });

        // Make sure we don't generate mouse button updates
        ev.mouse.button1 = gLastMouseEvent.mouse.button1;
        ev.mouse.button2 = gLastMouseEvent.mouse.button2;
        ev.mouse.button3 = false;

        gMouse->handleEvent(ev);

        gLastMouseEvent.mouse.x = ev.mouse.x;
        gLastMouseEvent.mouse.y = ev.mouse.y;
    }

    int hasKeyEvent = EM_ASM_INT_V({
        return workerApi.getInputValue(workerApi.InputBufferAddresses.keyEventFlagAddr);
    });
    if (hasKeyEvent) {
        int keyCode = EM_ASM_INT_V({
            return workerApi.getInputValue(workerApi.InputBufferAddresses.keyCodeAddr);
        });

        int keyState = EM_ASM_INT_V({
            return workerApi.getInputValue(workerApi.InputBufferAddresses.keyStateAddr);
        });

        SystemEvent ev;
        ev.type = sysevKey;
        ev.key.keycode = keyCode;
        ev.key.pressed = keyState != 0;
        gKeyboard->handleEvent(ev);
    }

    EM_ASM({ workerApi.releaseInputLock(); });

    // Ensure that period tasks are run (until we have idlewait support).
    EM_ASM({ workerApi.sleep(0); });

    if (gProcessCudaEvents != nullptr) {
        gProcessCudaEvents();
    }
}

void sys_gui_event() {
    sys_gui_poll_events();
    static_cast<JSSystemDisplay*>(gDisplay)->blit();
}

void sys_gui_cpu_ops_hook(uint ops) {
    sys_gui_poll_events();

    // Try to hit 60fps, with a bit (10%) of slack.
    static uint64 lastBlitTicks = sys_get_hiresclk_ticks();
    static uint ticksPerBlit = sys_get_hiresclk_ticks_per_second()/66;
    uint64 currentTicks = sys_get_hiresclk_ticks();
    if (currentTicks - lastBlitTicks > ticksPerBlit) {
        lastBlitTicks = currentTicks;
        static_cast<JSSystemDisplay*>(gDisplay)->blit();
    }
}

void sys_gui_cuda_hook(SystemEventHandler cudaEventHandler, void (*processCudaEvents)()) {
    gKeyboard->attachEventHandler(cudaEventHandler);
    gMouse->attachEventHandler(cudaEventHandler);
    gProcessCudaEvents = processCudaEvents;
}

void initUI(const char *title, const DisplayCharacteristics &aCharacteristics, int redraw_ms, const KeyboardCharacteristics &keyCharacteristics, bool fullscreen) {
    gDisplay = allocSystemDisplay(title, aCharacteristics, redraw_ms);
    gMouse = allocSystemMouse();
    gLastMouseEvent.mouse.button1 = false;
    gLastMouseEvent.mouse.button2 = false;
    gLastMouseEvent.mouse.button3 = false;
    gLastMouseEvent.mouse.x = 0;
    gLastMouseEvent.mouse.y = 0;

    gKeyboard = allocSystemKeyboard();
    if (!gKeyboard->setKeyConfig(keyCharacteristics)) {
        printf("no keyConfig, or is empty\n");
        exit(1);
    }
}

void doneUI() {
    printf("doneUI\n");
}

