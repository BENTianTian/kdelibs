    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

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

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#include "artsflow.h"
#include "debug.h"
#include "artsserver_impl.h"
#include "convert.h"
#include "objectmanager.h"
#include "audiosubsys.h"
#include "dispatcher.h"
#include "iomanager.h"
#include "flowsystem.h"
#include <stdio.h>
#include <iostream>

using namespace std;

class Synth_PLAY_impl :	virtual public Synth_PLAY_skel,
						virtual public ASProducer,
						virtual public IONotify
{
protected:
	AudioSubSystem *as;
	bool haveSubSys;
	/*
	 * these are to prevent the following situation
	 * 1) audio subsystem needs more data
	 * 2) calculation is started
	 * 3) somehow, some module makes a synchronous invocation to the outside
	 *    world and waits for the result
	 * 4) since the audio subsystem still needs data, and since we are in an
	 *    idle state now, another calculation will be started, which will of
	 *    course fail due to reentrancy
	 * 5) repeat 4) until result is there => lots of wasted CPU cycles (when
	 *    running with realtime priority: system freeze)
	 */
	bool inProgress;		// we are just doing some calculations
	bool restartIOHandling;	// I/O handlers removed upon reaching 4: restart

	int audiofd;

	typedef unsigned char uchar;

	unsigned char *outblock;
	unsigned long maxsamples;
	unsigned long channels;

public:
	/*
	 * functions from the SynthModule interface (which is inherited by
	 * SynthPlay)
	 */
	void firstInitialize() {
		//cout << "Synth_PLAY: firstInitialize() called." << endl;
	}

	void initialize() {
		as = AudioSubSystem::the();

		//cout << "Synth_PLAY: initialize() called." << endl;
		channels = as->channels();
		maxsamples = 0;
		outblock = 0;
		inProgress = false;

		haveSubSys = as->attachProducer(this);
		if(!haveSubSys)
		{
			printf("SynthGenericPlay: audio subsystem is already used\n");
			return;
		}

		audiofd = as->open(false);
		if(audiofd < 0)
		{
			printf("SynthGenericPlay: audio subsystem init failed\n");
			printf("ASError = %s\n",as->error());
			return;
		}
	}

	void start() {
		//cout << "Synth_PLAY: start() called." << endl;
		if(audiofd >= 0)
		{
			IOManager *iom = Dispatcher::the()->ioManager();
			iom->watchFD(audiofd,IOType::write|IOType::except,this);
		}
	}

	void deInitialize() {
		cout << "Synth_PLAY: deInitialize() called." << endl;

		artsdebug("SynthGenericPlay: closing audio fd\n");
		if(audiofd >= 0)
		{
			IOManager *iom = Dispatcher::the()->ioManager();
			iom->remove(this,IOType::all);
			audiofd = 0;
		}
		AudioSubSystem::the()->detachProducer();

		if(outblock)
		{
			delete[] outblock;
			outblock = 0;
		}
	}

	void calculateBlock(unsigned long samples)
	{
		// no audio subsystem, no play
		if(!as->running() || !haveSubSys) return;

		if(samples > maxsamples)
		{
			maxsamples = samples;

			if(outblock) delete[] outblock;
			outblock = new uchar[maxsamples * 4]; // 2 channels, 16 bit
		}

		assert(channels);

		if(channels == 1)
			convert_mono_float_16le(samples,invalue_left,outblock);

		if(channels == 2)
			convert_stereo_2float_i16le(samples,invalue_left,invalue_right,
													outblock);

		as->write(outblock,channels * 2 * samples);
	}

	/**
	 * notifyIO from the IONotify interface (IOManager)
	 */
	void notifyIO(int fd, int type)
	{
		if(!as->running())
		{
			printf("SynthGenericPlay: got notifyIO while audio subsystem"
				 	"is down\n");
			return;
		}
		assert(fd == audiofd);

		switch(type)
		{
			case IOType::read: type = AudioSubSystem::ioRead;
					break;
			case IOType::write: type = AudioSubSystem::ioWrite;
					break;
			default: assert(false);
		}
		if(inProgress)
		{
			if(!restartIOHandling)
			{
				// prevent lots of retries - we just can't do calculations
				// now, so we need to wait until the situation has resolved
				Dispatcher::the()->ioManager()->remove(this,IOType::all);
				restartIOHandling = true;
			}
			return;
		}
		restartIOHandling = false;
		inProgress = true;
		as->handleIO(type);
		inProgress = false;
		if(restartIOHandling) start();
	}

	/**
	 * needmore from the ASProducer interface (AudioSubSystem)
	 */
	void needMore()
	{
		_node()->requireFlow();
	}
	
};

REGISTER_IMPLEMENTATION(Synth_PLAY_impl);
