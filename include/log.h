/*
 * log.h
 *
 *  Created on: 2009-3-30
 *      Author: dongjunjie
 */

#ifndef LOG_H_
#define LOG_H_

#define DEBUG 0
#define COMMON_DEBUG 0x01 // 0
#define VALUETYPE_DEBUG 0 //0x02 //0
#define CLASS_INSTANCE_DEBUG 0 // 0x04
#define MAINFRAME_DEBUG 0 //0x08
#define UIEDITOR_DEBUG 0 //0x10
#define EDITPANEL_DEBUG 0 //0x20

#if 0
#define LOG_WARNING(format,...) log_warning(__FILE__ ":%d:" format "\n", __LINE__, ##__VA_ARGS__)

#define LOG_DEAD(format, ...) log_dead(__FILE__ ":%d" format "\n", __LINE__, ##__VA_ARGS__)
#else
#define LOG_WARNING(format,...)
#define LOG_DEAD(format, ...)
#endif

#if DEBUG
#define LOG_DEBUG(format, ...) log_debug(__FILE__ ":%d" format "\n", __LINE__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif
// log info
void log_warning(const char* format, ...);

void log_dead(const char* format, ...);

void log_debug(const char* format, ...);

#if DEBUG

#define EXPEND_RECT(rc)  (rc).left, (rc).top, (rc).right, (rc).bottom

#define RECT_FROMAT  "Left=%d, Top=%d, Right=%d, Bottom=%d"

#ifdef WIN32

#define DPRINT(format, ...)

#define DP(format, ...)

#else
#define DPRINT(format, ...) do{ \
	log_debug(__FILE__ ":%d:" format "\n", __LINE__, ##__VA_ARGS__); \
}while(0)


#define DP(format, ...) do{ \
	log_debug(format "\n", ##__VA_ARGS__); \
}while(0)

#endif

#else

#define DPRINT(format, ...)

#define DP(format, ...)

#endif

#endif /* LOG_H_ */
