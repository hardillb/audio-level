/*
 * audio.cpp
 *
 *  Created on: 9 Nov 2013
 *      Author: hardillb
 */
#include <stdio.h>
#include <alsa/asoundlib.h>
#include <math.h>

#include <node/node.h>
#include <v8.h>
#include <uv.h>

using namespace v8;

short buffer[8*1024];
int buffer_size = sizeof(buffer)>>1;

int64_t counter = 0;

static const char *device = "default";

snd_pcm_t *handle_capture;

uv_loop_t *loop;
uv_timer_t gc_req;
uv_thread_t thread_id;
uv_async_t async;

int dB = 0;

Local<Function> cb;

double rms(short *buffer)
{
	int i;
	long int square_sum = 0.0;
	for(i=0; i<buffer_size; i++) {
		square_sum += (buffer[i] * buffer[i]);
	}

	double result = sqrt(square_sum/buffer_size);
	return result;
}

void callback(uv_timer_s *timer, int i) {
	int err = 0;
	double k = 0.45255;
	double Pvalue = 0;
	int peak = 0;
	snd_pcm_sframes_t frames;

	frames = snd_pcm_readi(handle_capture, buffer, buffer_size);
	if(frames < 0)
		frames = snd_pcm_recover(handle_capture, frames, 0);
	if(frames < 0) {
		printf("snd_pcm_readi failed: %s\n", snd_strerror(err));
	}
	if(frames > 0 && frames < (long)buffer_size)
		printf("Short read (expected %li, wrote %li)\n", (long)buffer_size, frames);

	Pvalue = rms(buffer) * k;

	dB = (int)20*log10(Pvalue);
	if(dB > peak) {
		peak = dB;
	}
    async.data = &dB;
    uv_async_send(&async);

	printf("db=%d peak=%d\n", dB, peak);

}

void sendData(uv_async_t *handle, int status /*UNUSED*/) {
		int foo = *((int*) handle->data);
		const unsigned argc = 1;
		Local<Value> argv[argc] = { Local<Value>::New(Number::New(foo)) };
		printf("about to callback\n");
		cb->Call(Context::GetCurrent()->Global(), argc, argv);
}

Handle<Value> start(const Arguments& args) {
	HandleScope scope;
	int err;

	cb = Local<Function>::Cast(args[0]);

	if((err=snd_pcm_open(&handle_capture, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		printf("Capture open error: %s\n", snd_strerror(err));
	}

	if((err = snd_pcm_set_params(handle_capture,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			1,
			48000,
			1,
			500000)) < 0) {	/* 0.5s */
		printf("Capture open error: %s\n", snd_strerror(err));
	}

	loop = uv_default_loop();
	uv_timer_init(loop, &gc_req);
	uv_timer_start(&gc_req, callback, 0, 1000);
    uv_async_init(loop, &async, sendData);

	printf("started\n");

	return scope.Close(Undefined());
}


Handle<Value> shutdown(const Arguments& args) {
	HandleScope scope;
	printf("stopping\n");
	uv_close((uv_handle_t*) &async, NULL);
	uv_timer_stop(&gc_req);
	return  scope.Close(Undefined());
}


void init(Handle<Object> exports, Handle<Object> module) {
	exports->Set(String::NewSymbol("start"),
	      FunctionTemplate::New(start)->GetFunction());
	exports->Set(String::NewSymbol("stop"),
	      FunctionTemplate::New(shutdown)->GetFunction());
}

NODE_MODULE(audio_level, init)
