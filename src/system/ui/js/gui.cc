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

void initUI(const char *title, const DisplayCharacteristics &aCharacteristics, int redraw_ms, const KeyboardCharacteristics &keyCharacteristics, bool fullscreen) {
	gDisplay = allocSystemDisplay(title, aCharacteristics, redraw_ms);
	// gMouse = allocSystemMouse();
	// gKeyboard = allocSystemKeyboard();

}

void doneUI() {
    printf("doneUI\n");
}

