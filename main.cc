#include "rx.h"

#include <netdb.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <opus/opus.h>
#include <ortp/ortp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "defaults.h"
#include "device.h"
#include "notice.h"
#include "sched.h"
#include "speech_sdk.h"
#include "kws.h"

int float2char(float *pcm,char *pcm_char,int len){

	int ii = 0;
	for(ii = 0; ii <len; ii ++){
		*(pcm_char+ii) = (char)(*(pcm+ii));
	}

return 0;
}


int run_rx(RtpSession *session,
		OpusDecoder *decoder,
		snd_pcm_t *snd,
		const unsigned int channels,
		const unsigned int rate)
{
	int ts = 0;
	float *pcm;
	char * pcm_char;
	//pcm = (float *)alloca(sizeof(float) * 1920 * 1);
	pcm = (float *)malloc(sizeof(float) * 1920 * 1);
	pcm_char = (char *)malloc(sizeof(char) * 1920 * 1);

	for (;;) {
		int r, have_more;
		char buf[32768];
		void *packet;

		r = rtp_session_recv_with_ts(session, (uint8_t*)buf,
				sizeof(buf), ts, &have_more);
		assert(r >= 0);
		assert(have_more == 0);
		if (r == 0) {
			packet = NULL;
			if (0)//(verbose > 1)
				fputc('#', stderr);
		} else {
			packet = buf;
			if(0) //(verbose > 1)
				fputc('.', stderr);
		}

		r = play_one_frame(packet, r, decoder, snd, channels,pcm);
		int ii = float2char(pcm,pcm_char,1920*1);
		mobvoi_send_speech_frame((char*)pcm_char,sizeof(char) * 1920 * 1);
	
		if (r == -1)
			return -1;

		/* Follow the RFC, payload 0 has 8kHz reference rate */

		ts += r * 8000 / rate;
	}
	free(pcm);
}



int main(int argc, char *argv[])
{

	kws_init();

	int r, error;
	snd_pcm_t *snd;
	OpusDecoder *decoder;
	RtpSession *session;
	/* command-line options */
	const char *device = DEFAULT_DEVICE,
		*addr = DEFAULT_ADDR,
		*pid = NULL;
	unsigned int buffer = DEFAULT_BUFFER,
		rate = DEFAULT_RATE,
		jitter = DEFAULT_JITTER,
		channels = DEFAULT_CHANNELS,
		port = DEFAULT_PORT;

	fputs(COPYRIGHT "\n", stderr);

	for (;;) {
		int c;

		c = getopt(argc, argv, "c:d:h:j:m:p:r:v:");
		if (c == -1)
			break;
		switch (c) {
		case 'c':
			channels = atoi(optarg);
			break;
		case 'd':
			device = optarg;
			break;
		case 'h':
			addr = optarg;
			break;
		case 'j':
			jitter = atoi(optarg);
			break;
		case 'm':
			buffer = atoi(optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'r':
			rate = atoi(optarg);
			break;
		case 'v':
			
			rate = atoi(optarg);
			//verbose = atoi(optarg);
			break;
		case 'D':
			pid = optarg;
			break;
		default:
			usage(stderr);
			return -1;
		}
	}

	decoder = opus_decoder_create(rate, channels, &error);
	if (decoder == NULL) {
		fprintf(stderr, "opus_decoder_create: %s\n",
			opus_strerror(error));
		return -1;
	}

	ortp_init();
	ortp_scheduler_init();
	session = create_rtp_recv(addr, port, jitter);
	assert(session != NULL);


//	r = snd_pcm_open(&snd, device, SND_PCM_STREAM_PLAYBACK, 0);
	if (r < 0) {
		aerror("snd_pcm_open", r);
		return -1;
	}
	if (set_alsa_hw(snd, rate, channels, buffer * 1000) == -1)
		return -1;
	if (set_alsa_sw(snd) == -1)
		return -1;

	if (pid)
		go_daemon(pid);

	go_realtime();
	r = run_rx(session, decoder, snd, channels, rate);

	if (snd_pcm_close(snd) < 0)
		abort();

	rtp_session_destroy(session);
	ortp_exit();
	ortp_global_stats_display();

	opus_decoder_destroy(decoder);

	return r;
}
