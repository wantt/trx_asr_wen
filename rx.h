#ifndef _MAIN_H
#define _MAIN_H


#ifdef __cplusplus
extern "C"
{
#endif

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


//int run_rx(RtpSession *session,OpusDecoder *decoder,    snd_pcm_t *snd,    const unsigned int channels,    const unsigned int rate);
void usage(FILE *fd);
RtpSession* create_rtp_recv(const char *addr_desc, const int port,    unsigned int jitter);
int play_one_frame(void *packet,    size_t len,    OpusDecoder *decoder,    snd_pcm_t *snd,    const unsigned int channels,float *pcm);


#ifdef __cplusplus
}
#endif


#endif
