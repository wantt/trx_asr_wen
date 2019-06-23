// wantongtang@foxmail.com.

#include <assert.h>
#include <iostream>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <string.h>
#include "speech_sdk.h"
//#include "kws.h"
//#include "alsa_player.h"

static const std::string kAppKey = "";// "2844F2D8DF07F35DB998BDDA9D130708";
static const std::string kOfflineModel = "offline_model";
static const std::string kOfflineModelName = "multi_keywords_test";
static const char
    * kMultiKeywords[] = {"音量大", "音量小", "打开主页","切到hdmi","开机","关机"};
mobvoi_recognizer_handler_vtable* speech_handlers;

		volatile bool in_the_session = true;
  	//type = MOBVOI_RECOGNIZER_ONLINE_ASR ;
  	volatile mobvoi_recognizer_type type = MOBVOI_RECOGNIZER_KEYWORDS;
	 pthread_mutex_t mutex;
    pthread_cond_t cond;
	

	void on_remote_silence_detected() {
  	std::cout << "--------> dummy on_remote_silence_detected" << std::endl;
	}
	void on_partial_transcription(const char* result) {
			//if(kws_detected(result)){
			std::cout <<"\n\n"<< result<<" \n\n" <<std::endl;
	}


	
	void on_final_transcription(const char* result) {
		  if (type == MOBVOI_RECOGNIZER_ONLINE_ASR) {
    	pthread_mutex_lock(&(mutex));
	    in_the_session = false;
  	  pthread_cond_signal(&(cond));
    	pthread_mutex_unlock(&(mutex));
  }
}

void on_result(const char* result) {
//  std::cout << "--------> dummy on_result: " << result << std::endl;
  pthread_mutex_lock(&(mutex));
  in_the_session = false;
  pthread_cond_signal(&(cond));
  pthread_mutex_unlock(&(mutex));
}

void on_error(int error_code) {
 // std::cout << "--------> dummy on_error with error code: " << error_code
 //           << std::endl;
  pthread_mutex_lock(&(mutex));
  in_the_session = false;
  pthread_cond_signal(&(cond));
  pthread_mutex_unlock(&(mutex));
}

void on_local_silence_detected() {
//  std::cout << "--------> dummy on_local_silence_detected" << std::endl;
  mobvoi_recognizer_stop();
}

void on_volume(float spl) {
  // The sound press level is here, do whatever you want
  // std::cout << "--------> dummy on_speech_spl_generated: spl = "
  //           << std::fixed << std::setprecision(6) << spl
  //           << std::endl;
}
int kws_init(){
		
 pthread_mutex_init(&(mutex), NULL);
	 pthread_cond_init(&(cond), NULL);

  mobvoi_sdk_init(kAppKey.c_str());
	mobvoi_recognizer_init_offline();//kws

  speech_handlers =
      new mobvoi_recognizer_handler_vtable;
  assert(speech_handlers != NULL);
  memset(speech_handlers, 0, sizeof(mobvoi_recognizer_handler_vtable));
  speech_handlers->on_error = &on_error;
  speech_handlers->on_partial_transcription = &on_partial_transcription;
  speech_handlers->on_final_transcription = &on_final_transcription;
  speech_handlers->on_local_silence_detected = &on_local_silence_detected;
  speech_handlers->on_remote_silence_detected = &on_remote_silence_detected;
  speech_handlers->on_result = &on_result;
  speech_handlers->on_volume = &on_volume;
  mobvoi_recognizer_set_handler(speech_handlers);
	mobvoi_recognizer_set_keywords(
      sizeof(kMultiKeywords) / sizeof(kMultiKeywords[0]),
      kMultiKeywords, kOfflineModelName.c_str());//kws

	mobvoi_recognizer_set_params(kOfflineModel.c_str(),
                               kOfflineModelName.c_str());  //kws
	mobvoi_recognizer_build_keywords(kOfflineModel.c_str()); //kws

  mobvoi_recognizer_start(type);

    return 0;
}

