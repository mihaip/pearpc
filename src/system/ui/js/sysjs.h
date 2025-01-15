#ifndef __SYSJS_H__
#define __SYSJS_H__

#include <memory>

#include "system/display.h"

class JSSystemDisplay: public SystemDisplay {
public:
	JSSystemDisplay(const DisplayCharacteristics &chr, int redraw_ms);

    void blit();

	virtual void finishMenu();
	virtual	void updateTitle();
	virtual	void displayShow();
	virtual	void convertCharacteristicsToHost(DisplayCharacteristics &aHostChar, const DisplayCharacteristics &aClientChar);
	virtual	bool changeResolution(const DisplayCharacteristics &aCharacteristics);
	virtual	void getHostCharacteristics(Container &modes);
	virtual void setMouseGrab(bool enable);

	// Override VT100 functions to allow disabling them when not doing a verbose
	// boot.
	virtual void drawChar(int x, int y, vcp color, byte chr);
	virtual void fillVT(int x, int y, int w, int h, vcp color, byte chr);
	virtual void fillAllVT(vcp color, byte chr);

private:
    std::unique_ptr<byte[]> jsFrameBuffer;
    int jsFrameBufferSize;
	bool vt100DisplayEnabled;
};

class SystemKeyboard;
class SystemMouse;

SystemDisplay *allocSystemDisplay(const char *title, const DisplayCharacteristics &chr, int redraw_ms);
SystemKeyboard *allocSystemKeyboard();
SystemMouse *allocSystemMouse();

#endif
