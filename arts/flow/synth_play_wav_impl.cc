    /*

    Copyright (C) 1998-2000 Stefan Westerfeld
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

    */

#include "config.h"
#ifdef HAVE_LIBAUDIOFILE
#include "artsflow.h"
#include "stdsynthmodule.h"
#include "utils.h"
#include "cache.h"
#include "convert.h"
#include <stdio.h>

extern "C" {
/* Some versions of libaudiofile seem to lack the extern "C" declaration,
 * so you you may need that extra one.
 *
 * Other versions of libaudiofile seem to have two closing "}" in aupvlist.h,
 * so if you can't compile, this, check that /usr/include/aupvlist.h contains
 * something like that
 *
 * #ifdef __cplusplus
 * }
 * #endif 
 *
 * only once not twice.
 */
#include "audiofile.h"
}

#include <sys/stat.h>
#include <unistd.h>

class CachedWav : public CachedObject
{
protected:
	struct stat oldstat;
	string filename;

	CachedWav(Cache *cache, string filename);
	~CachedWav();

	typedef unsigned char uchar;

public:
	double samplingRate;
	long bufferSize;
	int channelCount;
	int sampleWidth;
	unsigned char *buffer;

	static CachedWav *load(Cache *cache, string filename);
	bool isValid();
	int memoryUsage();
};

CachedWav *CachedWav::load(Cache *cache, string filename)
{
	CachedWav *wav;

	wav = (CachedWav *)cache->get(string("CachedWav:")+filename);
	if(!wav) wav = new CachedWav(cache,filename);

	return(wav);
}

bool CachedWav::isValid()
{
	struct stat newstat;

	lstat(filename.c_str(),&newstat);
	return(newstat.st_mtime == oldstat.st_mtime);
}

int CachedWav::memoryUsage()
{
	return(bufferSize);
}

CachedWav::CachedWav(Cache *cache, string filename) : CachedObject(cache)
{
	int sampleFormat;
	AFframecount	frameCount;
	AFfilehandle	file;
	int				byteorder;

	this->filename = filename;
	setKey(string("CachedWav:")+filename);

	lstat(filename.c_str(),&oldstat);

	file = afOpenFile(filename.c_str(), "r", NULL);
	frameCount = afGetFrameCount(file, AF_DEFAULT_TRACK);
	channelCount = afGetChannels(file, AF_DEFAULT_TRACK);
	afGetSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);

	byteorder = get_byteorder();

	assert(byteorder != ORDER_UNKNOWN);

	if(byteorder == ORDER_BIGENDIAN)
		afSetVirtualByteOrder(file,AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);

	if(byteorder == ORDER_LITTLEENDIAN)
		afSetVirtualByteOrder(file,AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);

	printf("loaded wav %s\n",filename.c_str());
	printf("  sample format: %d, sample width: %d\n",sampleFormat,sampleWidth);
	printf("  channelCount%d\n",channelCount);

	// different handling required for other sample widths
	assert(sampleWidth == 16 || sampleWidth == 8);

	bufferSize = frameCount * (sampleWidth/8) * channelCount;

	buffer = new uchar[bufferSize];
	assert(buffer);

	samplingRate = afGetRate(file, AF_DEFAULT_TRACK);
	afReadFrames(file, AF_DEFAULT_TRACK, buffer, frameCount);

	afCloseFile(file);
}

CachedWav::~CachedWav()
{
	delete[] buffer;
}

class Synth_PLAY_WAV_impl : public Synth_PLAY_WAV_skel, StdSynthModule {
protected:
	float flpos;

	float _speed;
	string _filename;
	bool _finished;
	CachedWav *cachedwav;

	void unload()
	{
		if(cachedwav)
		{
			cachedwav->decRef();
			cachedwav = 0;
		}
	}

	void load()
	{
		// unload the old file if necessary
		unload();

		// load the new (which will reset the position)
		cachedwav = CachedWav::load(Cache::the(), _filename);
		flpos = 0.0;
	}

public:
	float speed() { return _speed; }
	void speed(float newSpeed) { _speed = newSpeed; }

	string filename() { return _filename; }
	void filename(const string& filename) { _filename = filename; load(); }

	bool finished() { return _finished; }

	Synth_PLAY_WAV_impl();
	~Synth_PLAY_WAV_impl();

	void calculateBlock(unsigned long samples);
};

Synth_PLAY_WAV_impl::Synth_PLAY_WAV_impl() {
	cachedwav = 0;
	_finished = false;
	_speed = 1.0;
	_filename = "";
}

Synth_PLAY_WAV_impl::~Synth_PLAY_WAV_impl()
{
	unload();
}

static const int samplingRate=44100;

void Synth_PLAY_WAV_impl::calculateBlock(unsigned long samples)
{
	float allSamples = cachedwav->bufferSize*8 /
	                   cachedwav->sampleWidth/cachedwav->channelCount;
	float fHaveSamples = allSamples - flpos;
	float speed = cachedwav->samplingRate / (float)samplingRate * _speed;

	fHaveSamples /= speed;
	fHaveSamples -= 2.0;		// one due to interpolation and another against
								// rounding errors

	unsigned long haveSamples;

	if(fHaveSamples < 0)
	{
		haveSamples = 0;
	}
	else
	{
		haveSamples = (int)fHaveSamples;
		if(haveSamples > samples) haveSamples = samples;
	}

	unsigned long i;

	if(haveSamples)
	{
		if(cachedwav->channelCount == 1)
		{
			if(cachedwav->sampleWidth == 16) {
				interpolate_mono_16le_float(haveSamples,
							flpos,speed,cachedwav->buffer,left);
			}
			else {
				interpolate_mono_8_float(haveSamples,
							flpos,speed,cachedwav->buffer,left);
			}
			memcpy(right,left,sizeof(float)*haveSamples);
		}
		else if(cachedwav->channelCount == 2)
		{
			if(cachedwav->sampleWidth == 16) {
				interpolate_stereo_i16le_2float(haveSamples,
							flpos,speed,cachedwav->buffer,left,right);
			}
			else {
				interpolate_stereo_i8_2float(haveSamples,
							flpos,speed,cachedwav->buffer,left,right);
			}
		} else {
			assert(false);
		}

		flpos += (float)haveSamples * speed;
	}

	_finished = false;
	for(i=haveSamples;i<samples;i++)
	{
		left[i] = right[i] = 0.0;
		_finished = true;
	}
}


REGISTER_IMPLEMENTATION(Synth_PLAY_WAV_impl);

#if 0
class Synth_PLAY_WAV :public SynthModule {
protected:
	CachedWav *cachedwav;

	unsigned char *buffer;
	int channelCount;
	unsigned long bufferSize, position, bytesPerSample;

	// inputs:
	enum { PROP_FILENAME };

	// outputs:
	enum { LEFT, RIGHT, DONE };

public:
	void Initialize();
	void DeInitialize();
	void Calculate() { assert(false); }
	void CalculateBlock(unsigned long samples);
	string getParams() { return("_filename;left,right,done"); }
	static void *Creator() { return new Synth_PLAY_WAV; }
};

ModuleClient MC_Synth_PLAY_WAV(SynthModule::get_MS,"Synth_PLAY_WAV",Synth_PLAY_WAV::Creator);

void Synth_PLAY_WAV::CalculateBlock(unsigned long samples)
{
	unsigned long haveSamples = samples;
	unsigned long remainingSamples;

	remainingSamples = (bufferSize-position)/bytesPerSample;
	if(haveSamples > remainingSamples) haveSamples = remainingSamples;

	float *left = out[LEFT], *right = out[RIGHT], *done = out[DONE];
	unsigned long i;

	if(haveSamples)
	{
		if(channelCount == 1)
		{
			if(bytesPerSample == 2) {
				convert_mono_16le_float(haveSamples,&buffer[position],left);
			}
			else {
				convert_mono_8_float(haveSamples,&buffer[position],left);
			}
			memcpy(right,left,sizeof(float)*haveSamples);
		}
		else if(channelCount == 2)
		{
			if(bytesPerSample == 2) {
				convert_stereo_i16le_2float(haveSamples,&buffer[position],
																	left,right);
			}
			else {
				convert_stereo_i8_2float(haveSamples,&buffer[position],
																	left,right);
			}
		} else {
			assert(false);
		}

		for(i=0;i<haveSamples;i++) done[i] = 0.0;

		position += bytesPerSample*channelCount*haveSamples;
	}

	for(i=haveSamples;i<samples;i++)
	{
		left[i] = right[i] = 0.0; done[i] = 1.0;		// ready, kill me ;)
	}
}

void Synth_PLAY_WAV::DeInitialize()
{
	cachedwav->decRef();
}

void Synth_PLAY_WAV::Initialize()
{
	cachedwav = CachedWav::load(Synthesizer->getCache(),
									getStringProperty(PROP_FILENAME));

	// may take some speed to access cachedwav every time
	bufferSize = cachedwav->bufferSize;	
	channelCount = cachedwav->channelCount;
	buffer = cachedwav->buffer;
	bytesPerSample = cachedwav->sampleWidth/8;

	haveCalculateBlock = true;
	position = 0;
}

class Synth_PLAY_PITCHED_WAV :public SynthModule {
protected:
	CachedWav *cachedwav;
	float flpos;

	// inputs:
	enum { FREQUENCY,RECFREQUENCY, PROP_FILENAME };

	// outputs:
	enum { LEFT, RIGHT, DONE };

public:
	void Initialize();
	void DeInitialize();
	void Calculate() { assert(false); }
	void CalculateBlock(unsigned long samples);
	string getParams() { return("frequency,recfrequency,_filename;left,right,done"); }
	static void *Creator() { return new Synth_PLAY_PITCHED_WAV; }
};

ModuleClient MC_Synth_PLAY_PITCHED_WAV(SynthModule::get_MS,"Synth_PLAY_PITCHED_WAV",Synth_PLAY_PITCHED_WAV::Creator);

void Synth_PLAY_PITCHED_WAV::CalculateBlock(unsigned long samples)
{
	float frequency = in[FREQUENCY][0], recfrequency = in[RECFREQUENCY][0];
	float allSamples = cachedwav->bufferSize*8 /
	                   cachedwav->sampleWidth/cachedwav->channelCount;
	float fHaveSamples = allSamples - flpos;
	float speed = cachedwav->samplingRate / (float)samplingRate *
				  frequency / recfrequency;

	fHaveSamples /= speed;
	fHaveSamples -= 2.0;		// one due to interpolation and another against
							// rounding errors

	unsigned long haveSamples;

	if(fHaveSamples < 0)
	{
		haveSamples = 0;
	}
	else
	{
		haveSamples = fHaveSamples;
		if(haveSamples > samples) haveSamples = samples;
	}

	float *left = out[LEFT], *right = out[RIGHT], *done = out[DONE];
	unsigned long i;

	if(haveSamples)
	{
		if(cachedwav->channelCount == 1)
		{
			if(cachedwav->sampleWidth == 16) {
				interpolate_mono_16le_float(haveSamples,
							flpos,speed,cachedwav->buffer,left);
			}
			else {
				interpolate_mono_8_float(haveSamples,
							flpos,speed,cachedwav->buffer,left);
			}
			memcpy(right,left,sizeof(float)*haveSamples);
		}
		else if(cachedwav->channelCount == 2)
		{
			if(cachedwav->sampleWidth == 16) {
				interpolate_stereo_i16le_2float(haveSamples,
							flpos,speed,cachedwav->buffer,left,right);
			}
			else {
				interpolate_stereo_i8_2float(haveSamples,
							flpos,speed,cachedwav->buffer,left,right);
			}
		} else {
			assert(false);
		}

		for(i=0;i<haveSamples;i++) done[i] = 0.0;

		flpos += (float)haveSamples * speed;
	}

	for(i=haveSamples;i<samples;i++)
	{
		left[i] = right[i] = 0.0; done[i] = 1.0;		// ready, kill me ;)
	}
}

void Synth_PLAY_PITCHED_WAV::DeInitialize()
{
	cachedwav->decRef();
}

void Synth_PLAY_PITCHED_WAV::Initialize()
{
	cachedwav = CachedWav::load(Synthesizer->getCache(),
									getStringProperty(PROP_FILENAME));

	haveCalculateBlock = true;
	flpos = 0.0;
}
#endif

#else
#warning "No libaudiofile available, that means, you won't be able to play wavs"
#endif
