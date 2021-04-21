#include "utils.h"

LPTSTR MPECreateStringFromUnicode(LSA_UNICODE_STRING string) {
	if (string.Buffer == NULL) {
		return NULL;
	}
	USHORT length = string.Length + 1;
	LPTSTR resString = (LPTSTR)GlobalAlloc(GPTR, sizeof(WCHAR) * length);

	if (!wcscpy(resString, string.Buffer)) {

	}

	return resString;
}
