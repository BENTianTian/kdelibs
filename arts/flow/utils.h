    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Simple utilities for the synthesizer stuff...
 */

#define ORDER_BIGENDIAN		0
#define ORDER_LITTLEENDIAN	1
#define ORDER_UNKNOWN		2

int get_byteorder();

#define BENCH_BEGIN			0
#define BENCH_END			1

float benchmark(int what);

#ifdef __cplusplus
}
#endif
