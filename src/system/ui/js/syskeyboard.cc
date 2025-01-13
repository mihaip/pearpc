#include "system/keyboard.h"

class JSSystemKeyboard: public SystemKeyboard {
	virtual int getKeybLEDs() {
        return 0;
    }

	void setKeybLEDs(int leds) {
    }

    virtual bool handleEvent(const SystemEvent &ev) {
		return SystemKeyboard::handleEvent(ev);
	}
};

SystemKeyboard *allocSystemKeyboard() {
	if (gKeyboard) return NULL;
	return new JSSystemKeyboard();
}
