/************************************************************************/
/*                                                                      */
/*                    A       SSS     PPPP    IIIII                     */
/*                   A A     S   S    P   P     I                       */
/*                  A   A    S        P   P     I                       */
/*                  AAAAA     SSS     PPPP      I                       */
/*                  A   A        S    P         I                       */
/*                  A   A    S   S    P         I                       */
/*                  A   A     SSS     P       IIIII                     */
/*                                                                      */
/*                  D    I    G    I    T    A    L                     */
/*                                                                      */
/*                  T h e   S o u n d   O f   D S P                     */
/*                                                                      */
/*                                                                      */
/*                (c) Copyright 1992-97, ASPI Digital                   */
/*                                                                      */
/*                         All Rights Reserved                          */
/*                                                                      */
/* TRADE SECRET                                                         */
/*                                                                      */
/* CONFIDENTIAL                                                         */
/*                                                                      */
/* MELP.H - user level header file for fixed point melp                 */
/*                                                                      */
/************************************************************************/

#ifndef __MELP
#define __MELP

/************************************************************************/
/* MELP frame size must be fixed to 180 samples/frame                   */
/************************************************************************/

#define MELP_FRAME_SIZE 180

#define MAX_MELP_FRAME_SIZE 7

typedef unsigned char   MELP_FRAME[MAX_MELP_FRAME_SIZE] ;

/************************************************************************/
/* MELP CODEWORD STRUCTURE                                              */
/*                                                                      */
/* MELP_codeword is a structure containing all the coded indices of the */
/* MELP parameters in a frame, plus an optional flag to indicate frame  */
/* erasure (due possibly to channel burst errors detected externally).  */
/* This structure may be passed into a bit-packer to form a bit-stream. */
/* The erase flag should be set to 0 for normal operation.  If it is    */
/* set to 1, melp_syn() treats the frame as an erasure.                 */
/*                                                                      */
/************************************************************************/

typedef struct{
   short    erase,                  /* flag to signal frame erasure     */
            sync,                   /* sync bit                         */
            gain0,                  /* first gain parameter             */
            gain1,                  /* second gain parameter            */
            pitch,                  /* pitch/voicing index              */
            jitter,                 /* aperiodic flag                   */
            bpvc,                   /* bandpass voicing information     */
            lsp_vq0,                /* LSP vector index - 1st stage     */
            lsp_vq1,                /* LSP vector index - 2nd stage     */
            lsp_vq2,                /* LSP vector index - 3rd stage     */
            lsp_vq3;                /* LSP vector index - 4th stage     */
} MELP_codeword;


/************************************************************************/
/* The following constants define the number of bits assigned to each   */
/* quantized parameter in the MELP_codeword structure.                  */
/************************************************************************/

#define MELP_BITS_SYNC		1
#define MELP_BITS_GAIN0		3
#define MELP_BITS_GAIN1		5
#define MELP_BITS_PITCH		7
#define MELP_BITS_JITTER	1
#define MELP_BITS_BPVC		4
#define MELP_BITS_LSP_VQ0	7
#define MELP_BITS_LSP_VQ1	6
#define MELP_BITS_LSP_VQ2	6
#define MELP_BITS_LSP_VQ3	6
#define MELP_BITS_FSMAG_VQ	8



/************************************************************************/
/* Coder & decoder functions                                            */
/************************************************************************/

void melp_ana_init(unsigned int);
                        /* This function is called to initialize MELP   */
                        /* analysis.  It must be called before invoking */
                        /* melp_ana(), but it only needs to be called   */
                        /* once.                                        */

void melp_syn_init(unsigned int);
                        /* This function is called to initialize MELP   */
                        /* synthesis.  It must be called before         */
                        /* invoking melp_syn(), but it only needs to be */
                        /* called once.                                 */

void melp_ana_mod(short sp_in[], unsigned char buf[]) ;
			/* This function calls the MELP encoder.  The   */
                        /* input speech should be placed in sp_in[],    */
                        /* which is an array of size MELP_FRAME_SIZE.   */
                        /* The quantized MELP parameters are returned   */
                        /* placed in the buf[] array. The array size is */
			/* 6 for 2kbps and 7 for 2.4kbps                */

void melp_syn_mod(unsigned char buf[],  short sp_out[]) ;
                        /* This function calls the MELP decoder.  The   */
                        /* output speech is an array of size            */
                        /* MELP_FRAME_SIZE.  The input to this function */
                        /* is the buf[] array. The array size is        */
                        /* 6 for 2kbps and 7 for 2.4kbps                */

#endif /* __MELP */


