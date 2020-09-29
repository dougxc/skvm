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
 * Provide MELP compression/decompression functions on the D'Cryptor platform.
 */
public final class Melp
{
    /**
     * Minimum size of a MELP frame (as an array of byte).
     */
    public static final int MIN_FRAME_SIZE = 6;

    /**
     * Maximum size of a MELP frame (as an array of byte).
     */
    public static final int MAX_FRAME_SIZE = 7;

    /**
     * Initialize MELP compression.
     */
    public native void initCompression (int rate);

    /**
     * Initialize MELP decompression.
     */
    public native void initDecompression (int rate);

    /**
     * Perform MELP compression.
     * @param audio The audio frame to be decompressed.
     * @param melp The result of the decompression.
     * @exception IllegalArgumentException if the length of audio is not
     * Audio.FRAME_SIZE or the length of melp is not in the range
     * [MIN_FRAME_SIZE .. MAX_FRAME_SIZE].
     */
    public void compress (short[] audio, byte[] melp) 
    {
        if (audio.length != Audio.FRAME_SIZE ||
            melp.length < MIN_FRAME_SIZE ||
            melp.length > MAX_FRAME_SIZE)
            throw new IllegalArgumentException();
        nativeCompress (audio, melp) ;
    }

    /**
     * Perform MELP decompression.
     * @param melp A compressed frame.
     * @param audio The result of the decompression.
     * @exception IllegalArgumentException if the length of audio is not
     * Audio.FRAME_SIZE or the length of melp is not in the range
     * [MIN_FRAME_SIZE .. MAX_FRAME_SIZE].
     */
    public void decompress (byte[] melp, short[] audio) 
    {
        if (audio.length != Audio.FRAME_SIZE ||
            melp.length < MIN_FRAME_SIZE ||
            melp.length > MAX_FRAME_SIZE)
            throw new IllegalArgumentException();
        nativeDecompress (melp, audio) ;
    }

    private native void nativeCompress (short[] audio, byte[] melp) ;
    private native void nativeDecompress (byte[] melp, short[] audio) ;
}
