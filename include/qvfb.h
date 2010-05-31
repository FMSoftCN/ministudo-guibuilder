
#ifndef QVFB_H
#define QVFB_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include <minigui/minigui.h>
MG_EXPORT void GUIAPI VFBSetCaption(const char* caption);
MG_EXPORT void GUIAPI VFBShowWindow(BOOL bshow);
MG_EXPORT void GUIAPI VFBAtExit(void (*callback)(void));

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
