/**************************************************************************

    alsaout.cc   - class AlsaOut which represents an alsa client/port pair
    This file is part of LibKMid 0.9.5
    Copyright (C) 2000  Antonio Larrosa Jimenez
    LibKMid's homepage : http://www.arrakis.es/~rlarrosa/libkmid.html            

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/
#include "alsaout.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "sndcard.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include "midispec.h"


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBASOUND
#include <sys/asoundlib.h>

//#define HANDLETIMEINDEVICES

#ifdef HANDLETIMEINDEVICES
#include <sys/ioctl.h>
#endif

SEQ_USE_EXTBUF();

AlsaOut::AlsaOut(int d,int _client, int _port, const char *cname,const char *pname) : MidiOut (d)
{
  seqfd = 0;
  tPCN=1;
  devicetype=KMID_ALSA;
  device= d;
  tgtclient=_client;
  tgtport=_port;
  tgtname=new char[strlen(cname)+strlen(pname)+3];
  strcpy(tgtname, cname);
  strcat(tgtname, "  ");
  strcat(tgtname, pname);
 printf("%d %d %d %s\n",device, tgtclient, tgtport, tgtname);

  handle=0L;

#ifdef HANDLETIMEINDEVICES
  count=0.0;
  lastcount=0.0;
  rate=100;
  convertrate=10;
#endif
  volumepercentage=100;
  ev=new snd_seq_event_t;
  if (ev==NULL) { printf("aaaaaaaargh !\n"); };

  map=new MidiMapper(NULL);

  if (map==NULL) { printfdebug("ERROR : alsaOut : Map is NULL\n"); return; };
  _ok=1;
};

AlsaOut::~AlsaOut()
{
  delete map;
  closeDev();
  delete ev;
  delete tgtname;
}

void AlsaOut::openDev (int)
{
  _ok=1;
  if (snd_seq_open(&handle, SND_SEQ_OPEN) < 0)
                fprintf(stderr, "Couldn't open sequencer: %s", snd_strerror(errno)); 

  queue = snd_seq_alloc_queue(handle);
  client = snd_seq_client_id(handle);
  tgt = new snd_seq_addr_t;
  tgt->client=tgtclient;
  tgt->port=tgtport;

  src = new snd_seq_addr_t;
  src->client = client;
  src->port = snd_seq_create_simple_port(handle, NULL,
	SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE
	| SND_SEQ_PORT_CAP_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC);

  int r=snd_seq_connect_to(handle, src->port, tgt->client, tgt->port);
  if (r < 0) { _ok=0; fprintf(stderr, "Cannot connect to %d:%d\n",tgtclient,tgtport); }
  time=0;
}

void AlsaOut::closeDev (void)
{
  if (!ok()) return;

  snd_seq_delete_simple_port(handle,src->port);
  snd_seq_free_queue(handle, queue);
  snd_seq_close(handle); 
}

void AlsaOut::initDev (void)
{
  int chn;
  if (!ok()) return;
#ifdef HANDLETIMEINDEVICES
  count=0.0;
  lastcount=0.0;
#endif
  uchar gm_reset[5]={0x7e, 0x7f, 0x09, 0x01, 0xf7};
  sysex(gm_reset, sizeof(gm_reset));
  for (chn=0;chn<16;chn++)
  {
    chnmute[chn]=0;
    chnPatchChange(chn,0);
    chnPressure(chn,64);
    chnPitchBender(chn, 0x00, 0x40);
    chnController(chn, CTL_MAIN_VOLUME,110*volumepercentage);
    chnController(chn, CTL_EXT_EFF_DEPTH, 0);
    chnController(chn, CTL_CHORUS_DEPTH, 0);
    chnController(chn, 0x4a, 127);
  }
}

void AlsaOut::eventInit(snd_seq_event_t *ev)
{
  snd_seq_ev_clear(ev);
  snd_seq_real_time_t tmp;
  tmp.tv_sec=(time)/1000;
  tmp.tv_nsec=(time%1000)*1000000;
//  printf("time : %d %d %d\n",(int)time,(int)tmp.tv_sec, (int)tmp.tv_nsec);
  ev->source = *src;
  ev->dest = *tgt; 

  snd_seq_ev_schedule_real(ev, queue, 0, &tmp);

}

void AlsaOut::eventSend(snd_seq_event_t *ev)
{
    int err = snd_seq_event_output(handle, ev);
/*        if (err < 0)
                return;
*/ 
//#ifndef SND_SEQ_IOCTL_GET_CLIENT_POOL
        /*
         * If this is not defined then block mode writes will not be
         * working correctly.  Therefore loop until all events are flushed
         * out.
         */
/*        err = 0;
        do {
                err = snd_seq_flush_output(handle);
                if (err > 0)
                        usleep(2000);
        } while (err > 0);

#endif
 
        return ;  
*/
}

void AlsaOut::noteOn  (uchar chn, uchar note, uchar vel)
{

  if (vel==0)
  {
    noteOff(chn,note,vel);
  }
  else
  {
    eventInit(ev);
    snd_seq_ev_set_noteon(ev,map->channel(chn), map->key(chn,chnpatch[chn],note), vel);
    eventSend(ev);
  }
  
#ifdef MIDIOUTDEBUG
  printfdebug("Note ON >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void AlsaOut::noteOff (uchar chn, uchar note, uchar vel)
{
  eventInit(ev);
  snd_seq_ev_set_noteoff(ev,map->channel(chn), map->key(chn,chnpatch[chn],note), vel);
  eventSend(ev);
#ifdef MIDIOUTDEBUG
  printfdebug("Note OFF >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void AlsaOut::keyPressure (uchar chn, uchar note, uchar vel)
{
  eventInit(ev);
  snd_seq_ev_set_keypress(ev,map->channel(chn), map->key(chn,chnpatch[chn],note), vel);
  eventSend(ev);
}

void AlsaOut::chnPatchChange (uchar chn, uchar patch)
{
#ifdef MIDIOUTDEBUG
  printfdebug("PATCHCHANGE [%d->%d] %d -> %d\n",
      chn,map->channel(chn),patch,map->patch(chn,patch));
#endif
  eventInit(ev);
  snd_seq_ev_set_pgmchange(ev,map->channel(chn), map->patch(chn,patch));
  eventSend(ev);
  chnpatch[chn]=patch;
}

void AlsaOut::chnPressure (uchar chn, uchar vel)
{
  eventInit(ev);
  snd_seq_ev_set_chanpress(ev,map->channel(chn), vel);
  eventSend(ev);

  chnpressure[chn]=vel;
}

void AlsaOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
  map->pitchBender(chn,lsb,msb);
  chnbender[chn]=(msb << 8) | (lsb & 0xFF);

  eventInit(ev);
  snd_seq_ev_set_pitchbend(ev,map->channel(chn), chnbender[chn]);
  eventSend(ev);
}

void AlsaOut::chnController (uchar chn, uchar ctl, uchar v) 
{
  map->controller(chn,ctl,v);
  if ((ctl==11)||(ctl==7))
  {
    v=(v*volumepercentage)/100;
    if (v>127) v=127;
  }

  eventInit(ev);
  snd_seq_ev_set_controller(ev,map->channel(chn), ctl, v);
  eventSend(ev);

  chncontroller[chn][ctl]=v;
}

void AlsaOut::sysex(uchar *data, ulong size)
{
  eventInit(ev);
  snd_seq_ev_set_sysex(ev, size, data);
  eventSend(ev);

#ifdef MIDIOUTDEBUG
  printfdebug("sysex\n");
#endif
}

void AlsaOut::channelSilence (uchar chn)
{
  uchar i;
  for ( i=0; i<127; i++)
  {
    noteOff(chn,i,0);
  };
}

void AlsaOut::channelMute(uchar chn, int a)
{
  if (a==1)
  {
    chnmute[chn]=a;
    channelSilence(chn);
  }
  else if (a==0)
  {
    chnmute[chn]=a;
  }
  /*  else ignore the call to this procedure */
}

void AlsaOut::seqbuf_dump (void)
{
  printf("You shouldn't be here.\n");
}

void AlsaOut::seqbuf_clean(void)
{
  printf("You shouldn't be here neither.\n");
}

void AlsaOut::wait(double ticks)
{
//  SEQ_WAIT_TIME(((int)(ticks/convertrate)));
  time=(long int)ticks;
  
#ifdef MIDIOUTDEBUG
  printfdebug("Wait  >\t ticks: %g\n",ticks);
#endif
}

void AlsaOut::tmrSetTempo(int v)
{
/*  eventInit(ev);
  ev->type = SND_SEQ_EVENT_TEMPO;
  ev->data.queue.queue = queue;
  ev->data.queue.param.value = v;
printf("tempo _ : _ : _ : %d\n",v);
  ev->dest.client = SND_SEQ_CLIENT_SYSTEM;
  ev->dest.port = SND_SEQ_PORT_SYSTEM_TIMER;
  eventSend(ev);
*/
#ifdef MIDIOUTDEBUG
  printfdebug("SETTEMPO  >\t tempo: %d\n",v);
#endif
  snd_seq_queue_tempo_t ev;
  ev.queue=queue;
  ev.tempo=v;
  ev.ppq=tPCN;
  snd_seq_set_queue_tempo(handle,queue,&ev);
}

void AlsaOut::sync(int i)
{
//#ifdef MIDIOUTDEBUG
  printf("Alsa Sync %d\n",i);
//#endif
  if (i==1) 
  {    
    snd_seq_flush_output(handle);
    snd_seq_drain_output(handle);
  }

  printf("Alsa 1\n");

  eventInit(ev);
  ev->dest = *src; 
  eventSend(ev);   
  snd_seq_flush_output(handle);
  printf("Alsa 2\n");
  snd_seq_event_input(handle,&ev);

  printf("Alsa 3\n");
}

void AlsaOut::timerEventSend(int type)
{
  snd_seq_event_t ev;

  ev.queue = queue;
  ev.dest.client = SND_SEQ_CLIENT_SYSTEM;
  ev.dest.port = SND_SEQ_PORT_SYSTEM_TIMER;

  ev.data.queue.queue = queue;

  ev.flags = SND_SEQ_TIME_STAMP_REAL | SND_SEQ_TIME_MODE_REL;
  ev.time.time.tv_sec = 0;
  ev.time.time.tv_nsec = 0;

  ev.type = type;

  snd_seq_event_output(handle, &ev);
  snd_seq_flush_output(handle);

}


void AlsaOut::tmrStart(int tpcn)
{
  snd_seq_queue_tempo_t queuetempo;
  int  ret;
 
  memset(&queuetempo, 0, sizeof(queuetempo));
  queuetempo.queue = queue;
  queuetempo.ppq = tpcn;
  tPCN=tpcn;
  queuetempo.tempo = 60*1000000/120;
 
  ret = snd_seq_set_queue_tempo(handle, queue, &queuetempo);  

  timerEventSend(SND_SEQ_EVENT_START);
  snd_seq_start_queue(handle,queue,NULL);



}

void AlsaOut::tmrStop(void)
{
  timerEventSend(SND_SEQ_EVENT_STOP);
}

void AlsaOut::tmrContinue(void)
{
  printf("Is this used ?\n");
}


const char * AlsaOut::deviceName(void) const
{
  return tgtname;
}

#endif // HAVE_LIBASOUND
