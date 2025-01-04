#include "system/display.h"
#include "system/ui/gui.h"

void sys_gui_init()
{
}

bool sys_gui_open_file_dialog(String &ret, const String &title, const String &filespec, const String &filespecname, const String &home, bool existing)
{
}

int sys_gui_messagebox(const String &title, const String &text, int buttons)
{
}

void sys_gui_event()
{
}

void initUI(const char *title, const DisplayCharacteristics &aCharacteristics, int redraw_ms, const KeyboardCharacteristics &keyCharacteristics, bool fullscreen)
{

}

void doneUI()
{
}

