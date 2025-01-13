#include "system/mouse.h"

class JSSystemMouse: public SystemMouse {
public:

	virtual bool handleEvent(const SystemEvent &ev)
	{
		return SystemMouse::handleEvent(ev);
	}
};

SystemMouse *allocSystemMouse()
{
	if (gMouse) return NULL;
	return new JSSystemMouse();
}
