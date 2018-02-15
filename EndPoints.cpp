/*
 *  EndPoints.cpp
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */


#ifndef EndPoints
#include "EndPoints.h"
#endif

#include <ctime>

#ifdef WIN32
#define TIME_NORMALIZER 1000		// For Microsoft Windows Systems
#else
#define TIME_NORMALIZER 1000000		// For Mac OS X/Linux/Unix Systems
#endif


EndPoint::EndPoint()
{
	Maparray_Endp = NULL;
	//passbool = NULL;
}

EndPoint::~EndPoint()
{
}

int EndPoint::Compression(string filename)
{ 
    
#ifndef DEBUG
	cout << filename << endl;
#else
	//cout << "File Name : " << filename << endl;
#endif

	// Start Compression ------------------------------------------------>
	unsigned int start, end;
	double elapsed;

	if( (Maparray_Endp = (unsigned char *) calloc(MAPSIZE, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for Maparray... Please check memory and try again later..." << endl;
		exit(-1);
	}

	ClassEncoder original, Preprocess;

	start = clock();

#ifdef PREPROCESS
	Preprocess.ReadPNG(filename);
	No_of_Map = Preprocess.Preprocessing();
	memcpy(Maparray_Endp,Preprocess.Maparray, MAPSIZE* sizeof(char)) ;
	Preprocess.Free(); 

#ifdef DEBUG
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << '\t' << '\t'  <<" Preprocessing time = " << elapsed << endl << endl;
	start = clock();
#endif
#endif
	// Read the layout image
	original.ReadPNG(filename);

	// Grab the image dimension
	width = original.width;
	height = original.height;
	bitdepth = original.bitdepth;
	row_bytes = original.row_bytes;

#ifdef DEBUG
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << '\t' << '\t' << " Opening and Reading Info Time = " << elapsed << endl;
	//cout << "Image Size = " << original.width * original.height << endl;
	start = clock();
#else
	cout << "Image Size = " << ((unsigned long long)original.width) * ((unsigned long long)original.height) << endl;
#endif

	// Transform: Image -> Corner (32 Symbol)
	//original.Transform_RLE_EOB(Maparray_Endp, No_of_Map);
	
#ifdef DEBUG
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << '\t' << '\t' << " Reading and corner transform and RLE+EOB Time = " << elapsed << endl;
	start = clock();
#endif

	//-------------------AC Encoding
	//original.EntropyEncoder_AC(filename);
	//RLE_Length = original.RLE_length;
	//original.deflate_compression(filename);

#ifdef LINEDIFF
	Bitlength = original.LineDiff(filename);
	RLE_Length = original.RLE_length;
	//cout << "RLE length :"<< RLE_Length << endl;
	
	/*if( (passbool = (bool *) calloc(Bitlength, sizeof(bool))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for passbool... Please check memory and try again later..." << endl;
		exit(-1);
	}
	
	memcpy(passbool,original.bool_RLE,Bitlength*sizeof(bool));*/
	original.WriteRLE(filename,0);
#endif
	
	//original.WriteRLE(filename,1);
	
#ifdef DEBUG
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << '\t' << '\t' << "                      Arithmetic coding time = " << elapsed << endl;
	start = clock();
	//original.deflate_compression(filename);
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << '\t' << '\t' << "                      Deflate coding time = " << elapsed << endl;
#else
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << "\t" << "\t" << "               Time taken for reading png = " << original.png_time << endl;
	cout << '\t' << '\t' << "                      Total encoding time = " << elapsed << endl;
	
#endif

	//original.histogram();
	// free memory
	original.Free();
#ifndef POSTPROCESS
	free(Maparray_Endp);
#endif
	//cout << unsigned int(char(256)) << endl;
	return 0;
}

unsigned char *EndPoint::Decompression(string filename)
{
	#ifdef DEBUG
	cout << "File Name : " << filename << endl;
    #endif
	
	// Start Decompression ---------------------------------------------->
	unsigned int start, end;
	double elapsed;
	
	ClassDecoder corner;
	
	// Initialize Workspace
	corner.width = width;
	corner.height= height;
	corner.RLE_length = RLE_Length;
#ifdef LINEDIFF
	corner.bitlength = Bitlength;
#endif
	
#ifdef POSTPROCESS
	corner.Maparray = Maparray_Endp;
#endif
	start = clock();
#ifndef MEMORYSAVE
	//corner.EntropyDecoder_AC(filename);
	//corner.inflate_decompression(filename);
	corner.ReadRLE(filename,0);
	
#endif

#ifdef DEBUG
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << "AC Decoding Time = " << elapsed << endl;
	start = clock();
	//corner.inflate_decompression(filename);
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << "inflate Decoding Time = " << elapsed << endl;
	start = clock();
#endif
	corner.WritePNG(filename);
		// Inverse Transform: Corner -> Image
#ifndef MEMORYSAVE
	//corner.Transform_RLE_EOB_decoding();
	
#ifdef LINEDIFF
	//corner.bool_RLE = passbool;
	corner.LineDiff_Decompression();
	
#endif

#else
	corner.Complete_Decoding(filename);
#endif
	end = clock();
	// End Decompression ------------------------------------------------>
	
	// Compute Runtime
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
#ifndef DEBUG
	cout << "\t" << "\t" << "               Time taken for writing png = " << corner.png_time << endl;
	cout << "\t" << "\t" << "                      Total decoding time = " << elapsed << endl;
#else
	cout << "RLE + EOB + Inverse Transform + writing file Time = " << elapsed << endl;
#endif

	corner.Free();
	return NULL;
}
