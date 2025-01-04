#include "system/sys.h"

static bool open_clipboard()
{
	return false;
}

static void close_clipboard()
{
}

bool sys_write_data_to_native_clipboard(const void *data, int size)
{
	return false;
}

int sys_get_native_clipboard_data_size()
{
	return false;
}

bool sys_read_data_from_native_clipboard(void *data, int max_size)
{
	return false;
}
