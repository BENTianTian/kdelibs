/* Evil evil evil hack to get OSS apps to cooperate with artsd
 * This is based on the original esddsp, which esd uses to do the same.
 *
 * Copyright (C) 1998 Manish Singh <yosh@gimp.org>
 * Copyright (C) 2000 Stefan Westerfeld <stefan@space.twc.de> (aRts port)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_SOUNDCARD_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/soundcard.h>

#include <artsc.h>
#include <dlfcn.h>


/*
 * original C library functions
 */
static int (*orig_open)(const char *pathname, int flags, mode_t mode);
static int (*orig_close)(int fd);
static int (*orig_ioctl)(int fd, int request, ...);
static ssize_t (*orig_write)(int fd, const void *buf, size_t count);
static int orig_init = 0;

#define CHECK_INIT()                            \
  {                                             \
	if(!orig_init) {                            \
	  orig_init = 1;                            \
	  orig_open = dlsym(RTLD_NEXT,"open");      \
	  orig_close = dlsym(RTLD_NEXT,"close");    \
	  orig_write = dlsym(RTLD_NEXT,"write");    \
	  orig_ioctl = dlsym(RTLD_NEXT,"ioctl");    \
    }                                           \
  }

/*
 * NOTE:
 * 
 * To truely support non-blocking I/O, there is some stuff missing. Namely,
 * select should be trapped and redirected, as well as the means for making
 * a stream non-blocking and so on. Maybe poll, too.
 *
 * Currently, only apps that are doing blocking I/O will probably work right.
 */

/**
 * the stream status: sndfd is -1 when unused, otherwise it is a useless fd
 * which points to /dev/null, to ensure compatibility with more weird
 * operations on streams - arts_init has been run (and must be freed again)
 * if sndfd != -1
 *
 * settings contains what has already been set (speed, bits, channels), and
 * is 7 when all of these are true
 *
 * stream contains an aRts stream or 0
 */
static int sndfd = -1;
static int settings;
static arts_stream_t stream = 0;

void artsdspdebug(const char *fmt,...)
{
	int artsdspdebug = -1;

	if(artsdspdebug < 0)
	{
		const char *verbose = getenv("ARTSDSP_VERBOSE");
		artsdspdebug=verbose && !strcmp(verbose,"1");
	}
	if(artsdspdebug)
	{
    	va_list ap;
    	va_start(ap, fmt);
    	(void) vfprintf(stderr, fmt, ap);
    	va_end(ap);
	}
}

int open (const char *pathname, int flags, mode_t mode)
{
  CHECK_INIT();

  if (strcmp(pathname,"/dev/dsp"))    /* original open for anything but sound */
    return orig_open (pathname, flags, mode);

  settings = 0;
  stream = 0;

  artsdspdebug ("aRts: hijacking /dev/dsp open...\n");

  sndfd = orig_open("/dev/null",flags,mode);
  if(sndfd >= 0)
  {
    int rc = arts_init();
    if(rc < 0)
    {
      artsdspdebug("error on aRts init: %s\n", arts_error_text(rc));
      orig_close(sndfd);
      return -1;
    }
  }

  /* success */
  return sndfd;
}

int ioctl (int fd, int request, void *argp)
{
  static int channels;
  static int bits;
  static int speed;

  CHECK_INIT();

  if (fd != sndfd)
    return orig_ioctl (fd, request, argp);
  else if (sndfd != -1)
    {
      int *arg = (int *) argp;
      artsdspdebug("aRts: hijacking /dev/dsp ioctl (%d : %x - %p)\n",
			  fd, request, argp);

      switch (request)
        {
        case SNDCTL_DSP_SETFMT:
          bits = (*arg & 0x30) ? 16 : 8;
          settings |= 1;
          break;

        case SNDCTL_DSP_SPEED:
          speed = *arg;
          settings |= 2;
          break;

        case SNDCTL_DSP_STEREO:
          channels = (*arg)?2:1;
          settings |= 4;
          break;

        case SNDCTL_DSP_GETBLKSIZE:
          *arg = stream?arts_stream_get(stream,ARTS_P_PACKET_SIZE):16384;
          break;

        case SNDCTL_DSP_GETFMTS:
          *arg = 0x38;
          break;

        case SNDCTL_DSP_GETCAPS:
          *arg = 0;
          break;

        default:
          artsdspdebug("aRts: unhandled /dev/dsp ioctl (%x - %p)\n",request, argp);
          break;
        }

      if (settings == 7 && !stream)
        {
		  const char *name = getenv("ARTSDSP_NAME");

          artsdspdebug ("aRts: creating stream...\n");
	      stream = arts_play_stream(speed,bits,channels,name?name:"artsdsp");
        }

      return 0;
    }

  return 0;
}

int close(int fd)
{
  CHECK_INIT();

  if (fd != sndfd)
    return orig_close (fd);
  else if (sndfd != -1)
    {
      artsdspdebug ("aRts: /dev/dsp close...\n");
      if(stream)
      {
        arts_close_stream(stream);
        stream = 0;
      }

      arts_free();

      orig_close(sndfd);
      sndfd = -1;
    }
  return 0;
}

ssize_t write (int fd, const void *buf, size_t count)
{
  CHECK_INIT();

  if(fd != sndfd)
    return orig_write(fd,buf,count);
  else if(sndfd != -1)
  {
    artsdspdebug ("aRts: /dev/dsp write...\n");
	if(stream != 0)
	{
      return arts_write(stream,buf,count);
	}
  }
  return 0;
}
#endif
