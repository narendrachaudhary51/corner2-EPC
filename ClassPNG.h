/*
 *  ClassPNG.h
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */

#ifndef ClassPNG_H_               // Defining ClassPNG_H if not defined earlier
#define ClassPNG_H_

#ifndef Parameters_H_            // including parameters.h if not included earlier
#include "Parameters.h"
#endif

#include <string>
#include <vector>

#ifdef WIN32                     // include png.h and zlib.h library if windows environment
#include <png.h>
#include <zlib.h>
#pragma comment(lib, "libPNG.lib")
 //#pragma comment(lib, "libtiff.lib")
//#else
//#include "/Users/xosh/Documents/Projects/Library/tiff-3.9.4/libtiff/tiffio.h"
#endif

using namespace std;

class ClassPNG
{
public:
	unsigned int width;
	unsigned int height;
	unsigned int bitdepth;
	unsigned int channels;
	unsigned int row_bytes;                  // number of bytes in a row 
	double png_time;

	png_structp pngPtr;                      // structure pointer for libpng 
	png_infop infoPtr;                       // info pointer for libpng

	unsigned short *RLE;                     // pointer for run length encoding
	bool *bool_RLE;
	size_t RLE_length;   
	unsigned long long bitlength;

    unsigned char *Maparray;                          // pointer for Maparray
	char *data1;
	png_bytep *rowPtrs;
	//png_bytep rowPtr;
	bool is_read;
	
	ClassPNG();
	~ClassPNG();

	// Read TIFF image to memory
	void ReadPNG(string filename);

	// Read number of rows of image
	void ReadRows(unsigned int row_number,unsigned int nRows);

	// Write number of rows of image
	void WriteRows(unsigned int row_number,unsigned int nRows);

	// Write memory to TIFF image
	void WritePNG(string filename);

	// Write memory to JBIG image
	void WriteJBIG(string filename);

	// Free memory
	void Free();

	// Compute entropy of the memory
	double Entropy();
	double RLE_Entropy(int nSymbols);		// Set nSymbols = MAX_CORNER_SYMBOL+MAX_SYMBOL+M+K+1 

	// Compare two images
	unsigned long long Compare(ClassPNG cmp);

	// Get variables
	//void GetVars(unsigned int &_width, unsigned int &_height, unsigned int row_bytes);
};

#endif