/*
*	Header File	: audio.h
*	Original Author	: K H Ong
*	Created		: 3 Nov 2000
*
* Copyright 2000 D'Crypt Pte Ltd
* All rights reserved.
*
* This file contains definitions and declarations related to audio recording
* and playback.
*
* $Id: audio.h,v 1.1.1.1 2002/05/21 20:38:47 dougxc Exp $
*
*/
#ifndef _AUDIO_H
#define _AUDIO_H

#include "dcrypt_global.h"

/* Define size of an audio frame in samples */
#define	AUDIO_FRAME_SIZE		180

/*
* Audio sampling rate in Hz. Valid values range from 2.4 kHz to 49.9 kHz.
* Not used any more as audio_init(...) now accepts sampling rate as an
* argument.
*/
#define	AUDIO_SAMPLE_RATE		8000

/* Set the default speaker gain. Valid values range from 0 to 31. */
#define AUDIO_SPK_GAIN			23

/* Set the default microphone gain. Valid values range from 0 to 31 */
#define AUDIO_MIC_GAIN			4

/* Define the structure of an audio frame */
typedef short	AUDIO_FRAME[AUDIO_FRAME_SIZE] ;

/*
* Size of the software buffers in audio frames. Currently, the transmit and
* receive buffers are configured to each contain a maximum of two frames. At
* any instant, the DMA controller will be transferring one of the two frames
* while the application will be manipulating the remaining frame. This 
incurs
* the lowest possible latency of 1 frame delay in the data transfer. In 
other
* words, an audio frame generated by the application and placed into the
* transmit buffer will wait at most 1 frame-time before it gets output to
* the speaker via DMA.
*/
#define SZ_AUDIO_BUFFER		3

typedef AUDIO_FRAME		AUDIO_BUFFER[SZ_AUDIO_BUFFER] ;

/* Define the maximum number of samples for single playback and recording */
#define	MAX_AUDIO_SAMPLES	10000

typedef short			AUDIO_SAMPLES[MAX_AUDIO_SAMPLES] ;

/*
* Initialize UCB audio codec and configure DMA controller for DMA transfer.
* The application only needs to call this function once. Thereafter, the
* application can call audio_enable() and audio_disable to start and stop
* continuous playback and recording. Alternatively, if the application only
* intends to output an audio waveform to the speaker without performing any
* recording, it can make use of functions such as audio_play_start(...) and
* audio_play_stop() to do so. Similarly, the application can call functions
* such as audio_record_start(...) and audio_record_stop() to capture audio
* samples from the microphone without doing any playback. The sample_rate
* argument represents the sampling rate in Hz and should be set to values
* between 2400 and 49900.
*/
extern void	audio_init (uint sample_rate) ;

/*
* Enable UCB audio codec and start DMA transfers. After this function is
* called, audio data will be captured from the microphone and stored in the
* software receive buffer while audio data in the software transmit buffer
* will be output to the speaker.
*/
extern void	audio_enable(void) ;

/*
* Disable UCB audio codec and stop DMA transfers. After this function is
* called, there will no longer be any data transfer between the audio codec
* and the internal software buffers.
*/
extern void	audio_disable(void) ;

/*
* Put an audio frame into the software transmit buffer for output to the
* speaker. Return OK if successfully and ERROR otherwise. A return value
* of ERROR simply means that the transmit buffer is currently full. Since
* audio frames are constantly being retrieved from this buffer to be output
* to the speaker, the ERROR condition exists only for a short while and will
* clear once there is enough space in the transmit buffer to put another
* audio frame.
*/
extern RESULT	audio_xmit(void) ;

/*
* Get an audio frame from the software receive buffer which contains audio
* data captured from the microphone. Return OK if successfully and ERROR
* otherwise. A return value of ERROR simply means that the receive buffer
* is currently empty. Since audio frames are constantly being captured from
* the microphone and placed into this buffer, the ERROR condition exists 
only
* for a short while and will clear once the receive buffer is filled with at
* least 1 audio frame.
*/
extern RESULT	audio_recv(void) ;

/*
* Set the speaker gain. Valid values range from 0 to 31.
*/
extern void	audio_set_spk_gain (uint spk_gain) ;

/*
* Set the microphone gain. Valid values range from 0 to 31.
*/
extern void	audio_set_mic_gain (uint mic_gain) ;

/*
* Play audio samples passed in from the application. This continues until 
all
* the samples have been played OR the user has explicitly stop the playback
* via audio_play_stop(). The num_samples argument indicates the number of
* samples to be played up to a maximum of MAX_AUDIO_SAMPLES. If num_samples
* is not a multiple of AUDIO_FRAME_SIZE, 'zeroized' samples will be padded 
up
* to the next multiple of AUDIO_FRAME_SIZE. Set cont_play to 1 if the 
samples
* should be played repeatedly, 0 otherwise. Note that the shared variable
* audio_shared_samples should not be written while playback is in progress.
*/
extern void	audio_play_start (int num_samples, int cont_play) ;

/*
* Stop playing audio samples passed in from the application earlier on via
* audio_play_start(...).
*/
extern void	audio_play_stop(void) ;

/*
* After audio_play_start(...) has been called, the application can poll the
* flag returned from this function to check whether all the audio samples
* have been played. This function returns OK when audio playback has 
stopped,
* ERROR otherwise.
*/
extern RESULT	audio_play_done() ;

/*
* Record audio samples from the microphone. This continues until the
* specified number of samples have been captured OR the application has
* explicitly stopped the recording via audio_record_stop(). The application
* can check whether all the samples have been captured by constantly polling
* the flag returned from audio_record_done(). The num_samples argument
* indicates the number of samples to be recorded up to a maximum of
* MAX_AUDIO_SAMPLES. Note that the shared variable audio_shared_samples
* should not be written while recording is in progress.
*/
extern void	audio_record_start (int num_samples) ;

/*
* Stop recording audio samples from the microphone.
*/
extern void	audio_record_stop() ;

/*
* After audio_record_start(...) has been called, the application can poll 
the
* flag returned from this function to check whether all the audio samples
* have been recorded. This function returns OK when audio recording has
* stopped, ERROR otherwise.
*/
extern RESULT	audio_record_done() ;

/*
* Shared variable for transferring data between this module and the calling
* module when any of the following functions is called:
*
* extern RESULT	audio_xmit(void) ;
* extern RESULT	audio_recv(void) ;
*/
extern AUDIO_FRAME	audio_shared ;

/*
* Shared variable for transferring data between this module and the calling
* module when the following function is called:
*
* extern void		audio_play_start() ;
* extern void		audio_record_start() ;
*/
extern AUDIO_SAMPLES	audio_shared_samples ;

#endif
