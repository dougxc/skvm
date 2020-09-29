/*
 *  Copyright 2000-2001 D'Crypt Pte Ltd
 *  All rights reserved.
 *
 *  Copyright (c) 1999-2001 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 *
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */
package com.dcrypt ;

/**
 * Provide audio recording and playback functions on the D'Cryptor platform.
 * Note that this class does not currently provide any protection against
 * multiple access to the audio subsytem through different instances of
 * this class.
 */
public final class Audio
{
    /**
     * Minimum sample rate.
     */
    public static final int MIN_SAMPLE_RATE = 2400;

    /**
     * Maximum sample rate.
     */
    public static final int MAX_SAMPLE_RATE = 49900;

    /**
     * Minimum speaker gain value.
     */
    public static final int MIN_SPEAKER_GAIN = 0;

    /**
     * Maximum speaker gain value.
     */
    public static final int MAX_SPEAKER_GAIN = 31;

    /**
     * Size of an audio frame (as an array of short).
     */
    public static final int FRAME_SIZE = 180;

    /**
     * Result code for a successful audio operation.
     */
    public static final int OK = 1;

    /**
     * Result code for an unsuccessful audio operation.
     */
    public static final int ERROR = 0;
  
    /**
     * This is a dummy frame used to flush the receive buffer before
     * playback.
     */
    private final short dummyReceiveFrame[] = new short[FRAME_SIZE];

    /**
     * This is the sample rate.
     */
    private int sampleRate = 6000;

    /**
     * Construct an audio object with an initial sampling rate.
     */
    public Audio(int rate) {
        setSampleRate(rate);
    }

    /**
     * Set the sample rate for audio services. The sampleRate argument
     * represents the sampling rate in Hz and should be set to values
     * between 2400 and 49900.
     * @param rate The sampling rate in Hz and should be set to values
     * between MIN_SAMPL_RATE and MAX_SAMPLE_RATE.
     * @exception IllegalArgumantException if sampleRate is not within the
     * range [MIN_SAMPLE_RATE .. MAX_SAMPLE_RATE].
     */
    public void setSampleRate(int rate) throws IllegalArgumentException
    {
        if (rate < MIN_SAMPLE_RATE || rate > MAX_SAMPLE_RATE)
            throw new IllegalArgumentException();
        sampleRate = rate;
        nativeSetSampleRate(rate);
    }

    /**
     * Return the sample rate.
     * @return the sample rate.
     */
    public int getSampleRate() {
        return sampleRate;
    }

    /**
     * Enable audio playback. After this function is called, audio data
     * will be captured from the microphone and stored in the software
     * receive buffer while audio data in the software transmit buffer will
     * be output to the speaker until the disable method is called.
     */
    public native void enable();

    /**
     * Disable audio playback. After this function is called, there will no
     * longer be capture from the microphone or playback through the
     * speaker until the enable method is called again.
     */
    public native void disable(); 

    /**
     * Send an audio frame for playback. The size of the frame must be 
     * @param frame The audio frame to play.
     * @exception IllegalArgumentException if audioFrame is not the
     * correct size.
     */
    public void play (short[] frame) 
    {
        if (frame.length != FRAME_SIZE)
            throw new IllegalArgumentException();

        // Flush the recording buffer. I'm not sure why this is
        // necessary...
        while (nativeReceive (dummyReceiveFrame) == ERROR) ;
        
        nativeTransmit (frame) ;
    }

    /**
     * Set speaker gain. Valid values range from MIN_SPEAKER_GAIN to
     * MAX_SPEAKER_GAIN.
     * @param gain The speaker gain amount.
     * @exception IllegalArgumentException if gain is not valid.
     */
    public void setSpeakerGain (int gain)
    {
        if (gain < MIN_SPEAKER_GAIN || gain > MAX_SPEAKER_GAIN)
            throw new IllegalArgumentException();
        nativeSetSpeakerGain (gain) ;
    }

    private native void nativeSetSampleRate (int rate);
    private native int  nativeTransmit (short[] frame) ;
    private native int  nativeReceive (short[] frame) ;
    private native void nativeSetSpeakerGain (int gain) ;
}
