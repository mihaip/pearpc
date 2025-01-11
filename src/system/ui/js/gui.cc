#include "system/display.h"
#include "system/ui/gui.h"

#include "sysjs.h"

void sys_gui_init() {}

bool sys_gui_open_file_dialog(String &ret, const String &title, const String &filespec, const String &filespecname, const String &home, bool existing) {
    printf("sys_gui_open_file_dialog called with title: %s, filespec: %s, filespecname: %s, home: %s, existing: %d\n", title.contentChar(), filespec.contentChar(), filespecname.contentChar(), home.contentChar(), existing);
    return false; // Placeholder return value
}

int sys_gui_messagebox(const String &title, const String &text, int buttons) {
    printf("sys_gui_messagebox called with title: %s, text: %s, buttons: %d\n", title.contentChar(), text.contentChar(), buttons);
    return 0; // Placeholder return value
}

void sys_gui_event() {
    static_cast<JSSystemDisplay*>(gDisplay)->blit();
}

void sys_gui_cpu_ops_hook(uint ops) {
    // We get invoked every 0x3ffff (256K ops), but for now blit every 
    // 0x7ffff (512K) ops which is roughly 60 Hz on my machine.
    // TODO: use realtime click to actually try to hit 60fps.
    if ((ops & 0x7ffff) == 0) {
        static_cast<JSSystemDisplay*>(gDisplay)->blit();
    }
}

void initUI(const char *title, const DisplayCharacteristics &aCharacteristics, int redraw_ms, const KeyboardCharacteristics &keyCharacteristics, bool fullscreen) {
	gDisplay = allocSystemDisplay(title, aCharacteristics, redraw_ms);
	// gMouse = allocSystemMouse();
	// gKeyboard = allocSystemKeyboard();

}

void doneUI() {
    printf("doneUI\n");
}

