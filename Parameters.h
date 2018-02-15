/*
 *  Parameters.h
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */

#ifndef Parameters_H_
#define Parameters_H_

#define PNGSIGSIZE 8                    // PNG signature
//#define DEBUG
//#define PREPROCESS                   // Preprocessing if input image has pixel value range (0, 255) instead of (0,31)
//#define POSTPROCESS                  // Postprocessing if output image has pixel value range (0, 255) instead of (0,31)
#define ROWBUFFER 5000                 // number of rows reading at a time from PNG. Adjust according to memory constraints
#define WRITE_ROWBUFFER 1              // number of rows writing at a time from PNG. Adjust according to memory constraints
#define MAPSIZE 32                     // Number of different pixel values
#define ENCODER                        // enable if compling code in encoder mode or both
#define DECODER                        // enable if compling code in encoder mode or both
//#define PAETH						   // enables paeth filter instead of corner transform
//#define MEMORYSAVE                   // decoder runs in memory save mode if enabled(recommended to keep it enabled)
#define LINEDIFF

#define MAX_CORNER_SYMBOL 125         //125 for corner and 32 for Paeth

#define M 64					    // Base of M-ary RLE for 0s. Should be a power of 2 
#define RSHIFT_M 6					// log2(M) for faster base M-ary RLE computation

#define K 64     					// Base of K-ary RLE for EOBs. Should be power of 2
#define RSHIFT_K 6					// log2(K) for faster base K-ary RLE computation

#define BlockSize 79050	            // Block Size for EOB Coding

#define EOF_SYMBOL MAX_CORNER_SYMBOL+M+K    // defining end of file symbol for arithmatic encoding

#endif
