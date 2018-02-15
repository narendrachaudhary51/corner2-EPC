/*
 *  ClassEncoder.h
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */

#ifndef ClassEncoder_H_
#define ClassEncoder_H_

#ifndef Parameters_H_
#include "Parameters.h"
#endif

//#ifndef USE_PATTERN_MATCHING
#ifndef ClassPNG_H_
#include "ClassPNG.h"
#endif

#include <iostream>
#include <fstream>
#include <string>
//#include <map.h>

using namespace std;

class ClassEncoder: public ClassPNG

{
public:
	ClassEncoder();
	~ClassEncoder();

	int Preprocessing();

	// Transformation + RLE + EOB: Image -> Corner/Paeth -> RLE+EOB 
	int Transform_RLE_EOB(unsigned char* Map, int No_of_Map);

	// Entropy Encoder: AC
	int EntropyEncoder_AC(string filename);

	// deflate 
	int deflate_compression(string filename);

	// LineDiff encoding
	unsigned long long LineDiff(string filename);

	//Triline compression
	int Triline_Compression();

	// Write RLE data to text
	void WriteRLE(string filename,int flag);           // flag 1 for text, 0 for binary

	int histogram();
};

#endif