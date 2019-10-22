/*
** This file is a part of miniStudio, which provides a WYSIWYG UI designer
** and an IDE for MiniGUI app developers.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minigui/common.h"

#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "qvfb.h"

#define NOBUTTON        0x0000
#define LEFTBUTTON      0x0001
#define RIGHTBUTTON     0x0002
#define MIDBUTTON       0x0004
#define MOUSEBUTTONMASK 0x00FF

#define SHIFTBUTTON     0x0100
#define CONTROLBUTTON   0x0200
#define ALTBUTTON       0x0400
#define METABUTTON      0x0800
#define KEYBUTTONMASK   0x0FFF
#define KEYPAD          0x4000

static QVFBEVENT qvfb_event;
static unsigned char read_mouse_result = 0;
static unsigned char nr_changed_keys = 0;

static POINT mouse_pt;
static int mouse_buttons;
static unsigned char kbd_state [NR_KEYS];

/********************  Low Level Input Operations ******************/

/*
 * Mouse operations -- Event
 */

static int mouse_update (void)
{
	return read_mouse_result;
}

static void mouse_getxy (int *x, int* y)
{
    *x = mouse_pt.x;
    *y = mouse_pt.y;
}

static int mouse_getbutton (void)
{
    int buttons = 0;

    if (mouse_buttons & LEFTBUTTON)
        buttons |= IAL_MOUSE_LEFTBUTTON;
    if (mouse_buttons & RIGHTBUTTON)
        buttons |= IAL_MOUSE_RIGHTBUTTON;
    if (mouse_buttons & MIDBUTTON)
        buttons |= IAL_MOUSE_MIDDLEBUTTON;

    return buttons;
}


static int keyboard_update (void)
{
    return nr_changed_keys;
}

static const char* keyboard_getstate (void)
{
    return (char*)kbd_state;
}

static int read_key (QVFBEVENT *qvfb_event)
{
    static unsigned char last;
    unsigned char scancode;
    unsigned int unicode;
    BYTE press;

	unicode = qvfb_event->data.key.key_code;
    press = (qvfb_event->data.key.key_state) ? 1 : 0;

    if (unicode == 0 && !press) {
        kbd_state [last] = 0;
    }
    else {
        scancode = unicode;
        kbd_state [scancode] = press;
        last = scancode;
    }

    nr_changed_keys = last + 1;
    return 1;
}

void SetQVFB2Caption(const char* caption)
{
	int len = 0;
	char szCaption[1024];
	QVFBCaptionEventData * ced = (QVFBCaptionEventData*)szCaption;

	if(caption == NULL || (len = strlen(caption)) <= 0)
		return;

	ced->event_type = CAPTION_TYPE;
	ced->size = len;
	strcpy(ced->buff,caption);
	//printf("send = %s\n",caption);

	if(__mg_pcxvfb_client_sockfd == -1)
		return ;
	write(__mg_pcxvfb_client_sockfd, ced, sizeof(QVFBCaptionEventData)+len - 4);

}

void OpenQVFB2IME(int bOpen)
{
	QVFBIMEEventData ime={
			IME_TYPE,
			bOpen
	};

	if(__mg_pcxvfb_client_sockfd == -1)
		return ;
	write(__mg_pcxvfb_client_sockfd, &ime, sizeof(ime));
}

void ShowQVFB2(BOOL bshow)
{
	int info [2] = {
			SHOW_HIDE_TYPE,
			bshow
	};
	if(__mg_pcxvfb_client_sockfd == -1)
		return ;
	write(__mg_pcxvfb_client_sockfd, info, sizeof(info));
}


static void (*set_ime_text)(void*,const char*) = NULL;
static void * ime_text_callback_data = NULL;

void SetIMETextCallback(void (*setIMEText)(void *, const char* text), void *user_data)
{
	set_ime_text = setIMEText;
	ime_text_callback_data = user_data;
	return;
}

extern void ExitGUIBuilder(void);

static int read_event(int fd, QVFBEVENT *qvfb_event)
{
	int ret;
	int type;

	ret = read(fd, &type, sizeof(type));
	if(ret < 0)
		return -1;
/*	else if(ret == 0){
		ExitGUISafely(-1);
		return 0;
	}*/

	switch(type)
	{
	case MOUSE_TYPE:
	case KB_TYPE:
		ret = read(fd, &(qvfb_event->data.mouse), sizeof(QVFBMOUSEDATA));
		if(ret != sizeof(QVFBMOUSEDATA))
			return -1;
		qvfb_event->event_type = type;
		return 1;
	case IME_MESSAGE_TYPE:
		{
			int size;
			char szBuff[1024];
			if(read(fd, &size, sizeof(size)) != sizeof(size))
				return 0;
			if(size <= 0)
				return 0;
			if(read(fd, szBuff, size) == size){
				szBuff[size] = '\0';
				set_ime_text(ime_text_callback_data, szBuff);
			}
			return 0;
		}
	case QVFB_CLOSE_TYPE:
		close(__mg_pcxvfb_client_sockfd);
		__mg_pcxvfb_client_sockfd = -1;
		ExitGUIBuilder();
		break;
	}
	return 0;
}

static int wait_event (int which, int maxfd, fd_set *in, fd_set *out,
                      fd_set *except, struct timeval *timeout)
{
	int    retvalue = 0;
	fd_set rfds;
	int    fd, e;

	if (!in) {
		in = &rfds;
		FD_ZERO (in);
	}

	if (which & IAL_MOUSEEVENT && __mg_pcxvfb_client_sockfd >= 0) {
		fd = __mg_pcxvfb_client_sockfd;
		FD_SET (fd, in);
	}

	if (which & IAL_KEYEVENT && __mg_pcxvfb_client_sockfd >= 0) {
		fd = __mg_pcxvfb_client_sockfd;
		FD_SET (fd, in);
	}

	/* FIXME: pass the real set size */
	e = select (FD_SETSIZE, in, out, except, timeout) ;

	if (e > 0) {
		fd = __mg_pcxvfb_client_sockfd;
		/* If data is present on the mouse fd, service it: */
		if (fd >= 0 && FD_ISSET (fd, in)) {
			FD_CLR (fd, in);
			if(read_event(__mg_pcxvfb_client_sockfd, &qvfb_event)!=1)
				return -1;
			if (qvfb_event.event_type == MOUSE_TYPE) {
				if (qvfb_event.data.mouse.button < 0x08) {
					mouse_pt.x = qvfb_event.data.mouse.x;
					mouse_pt.y = qvfb_event.data.mouse.y;
					mouse_buttons = qvfb_event.data.mouse.button;
					read_mouse_result = 1;
				}else {
					read_mouse_result = 0;
				}

				retvalue |= IAL_MOUSEEVENT;
			} else if (qvfb_event.event_type == KB_TYPE) {
				if (read_key (&qvfb_event)) {
					retvalue |= IAL_KEYEVENT;
				}else {
					if (timeout) {
						timeout->tv_sec = 0;
						timeout->tv_usec = 0;
					}
				}
			}
		}
	} else if (e < 0) {
		return -1;
	}
	return retvalue;
}

BOOL InitCustomInput (INPUT* input, const char* mdev, const char* mtype)
{
    input->update_mouse = mouse_update;
    input->get_mouse_xy = mouse_getxy;
    input->set_mouse_xy = NULL;
    input->get_mouse_button = mouse_getbutton;
    input->set_mouse_range = NULL;
    input->suspend_mouse= NULL;
    input->resume_mouse = NULL;

    input->update_keyboard = keyboard_update;
    input->get_keyboard_state = keyboard_getstate;
    input->suspend_keyboard = NULL;
    input->resume_keyboard = NULL;
    input->set_leds = NULL;

    input->wait_event = wait_event;

    return TRUE;
}

void TermCustomInput (void)
{
    char   socket_file [50];

    sprintf(socket_file, "/tmp/qvfb_socket%d", getpid());
    socket_file[49] = '\0';

    if (__mg_pcxvfb_server_sockfd >= 0)
        close(__mg_pcxvfb_server_sockfd);
    __mg_pcxvfb_server_sockfd = -1;
    __mg_pcxvfb_client_sockfd = -1;

    unlink(socket_file);
}

#endif

