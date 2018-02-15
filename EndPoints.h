/*
 *  EndPoints.h
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */

#ifndef EndPoints
#define EndPoints

#ifndef Parameters_H_
#include "Parameters.h"
#endif

#ifndef ClassEncoder_H_
#include "ClassEncoder.h"
#endif

#ifndef ClassDecoder_H_
#include "ClassDecoder.h"
#endif

using namespace std;

class EndPoint
{
public:
	unsigned int width;
	unsigned int height;
	unsigned int bitdepth;
	unsigned int channels;
	unsigned int row_bytes;

	unsigned char *Maparray_Endp;
	//unsigned short *pass;
	//bool *passbool;
	int No_of_Map;
	size_t RLE_Length;
	unsigned long long Bitlength;

	EndPoint();
	~EndPoint();
	
	int Compression(string filename);
	unsigned char *Decompression(string filename);
	//unsigned char *Decompression(string filename, unsigned int _width, unsigned int _height, unsigned int _RLE_length, string mapfile);
	size_t Verify(unsigned char *Dec, string filename);
};

#endif