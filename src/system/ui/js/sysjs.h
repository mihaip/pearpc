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
private:
    std::unique_ptr<byte[]> jsFrameBuffer;
    int jsFrameBufferSize;
};

class SystemKeyboard;
class SystemMouse;

SystemDisplay *allocSystemDisplay(const char *title, const DisplayCharacteristics &chr, int redraw_ms);
SystemKeyboard *allocSystemKeyboard();
SystemMouse *allocSystemMouse();

#endif
