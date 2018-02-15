/*
 *  ClassDecoder.cpp
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */

#ifndef ClassDecoder_H_
#include "ClassDecoder.h"
#endif
#include <fstream>
#ifndef AC_HEADER
#include "AC.h"
#endif
#include <bitset>
#include <math.h>
#define logX(x, y) log((double) x)/log((double) y)
#define log2(x) log((double) x)/log(2.0)

/* Constructor */
ClassDecoder::ClassDecoder()
{
}

/* Destructor */
ClassDecoder::~ClassDecoder()
{
}

#ifndef MEMORYSAVE                                          // if MEMORYSAVE is not defined than do two step decoding
int ClassDecoder::Transform_RLE_EOB_decoding()
{
	char *current_row;
	unsigned char *buffer_previous_row;
	if( (buffer_previous_row = (unsigned char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for previous row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}

	if( (current_row = (char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for current row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}

	/*if( (rowPtr = (png_bytep) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for png row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}*/
	unsigned int Rows = height;
	if (Rows > WRITE_ROWBUFFER) 
		Rows = WRITE_ROWBUFFER;

	rowPtrs = new png_bytep[Rows];                                   // allocating rowPtrs memory for first iteration of writing
	data1 = new char[width * Rows * bitdepth / 8];
	int stride = width * bitdepth / 8;
	for (size_t i = 0; i < Rows; i++)
	{
		png_uint_32 q =  i * stride;
		rowPtrs[i] = (png_bytep)data1 + q;                  // setting the values of rowPtrs     
	}

	unsigned long long idx = 0, image_size = ((unsigned long long)width)*((unsigned long long)height);
	unsigned int count = 0,count_row =0, count_edge = 0;
	unsigned int index = 0;
	unsigned int exponent = 1, start_row = 0;
	char temp = 0;
#ifdef PAETH
	int pb,pc,pd,predict;				// variables for PAETH filter implementation
#endif

	for (size_t i=0; i < RLE_length; i++)
	{

		if (RLE[i] < MAX_CORNER_SYMBOL)
		{
			temp = (char) RLE[i];
#ifndef PAETH
			if ((temp%2)==0) temp = temp/2;
			else temp = -((temp+1)/2);
#endif
			//index = idx % width;
			//if ((index) == (width-1))
			current_row[idx % width] = temp;
			if ((idx % width) == (width-1))                        // if reached at the end of row start corner decoding and write to file
		    {
				count_edge++;
				for (unsigned int j=0;j<width;j++)                // Inverse corner transform
			        {
						if (j>0)
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + *(rowPtrs[count_row-start_row]+j-1) + buffer_previous_row[j] - buffer_previous_row[j-1];
							//rowPtr[j] = current_row[j] + rowPtr[j-1] + buffer_previous_row[j] - buffer_previous_row[j-1];
#else
							//if (idx<width)predict=0;
							//else{
								predict = buffer_previous_row[j] + *(rowPtrs[count_row-start_row]+j-1) - buffer_previous_row[j-1];
								pb = abs(predict - *(rowPtrs[count_row-start_row]+j-1));                  //distances to  b, c,d
								pd = abs(predict - buffer_previous_row[j]);
								pc = abs(predict - buffer_previous_row[j-1]);
								if(pb <= pd && pb <= pc) predict = *(rowPtrs[count_row-start_row]+j-1);
								else if (pd <= pc) predict = buffer_previous_row[j];
								else predict = buffer_previous_row[j-1];
							//}
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;
#endif
						}
						else
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + buffer_previous_row[j];
							//rowPtr[j] = current_row[j] + buffer_previous_row[j];
#else						
							predict = buffer_previous_row[j];
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;;
#endif
						}
			        }
				      memcpy(buffer_previous_row,rowPtrs[count_row-start_row],width*sizeof(char));         // put row data into previous row
					//memcpy(buffer_previous_row,rowPtr,width*sizeof(char));         // put row data into previous row

#ifdef POSTPROCESS                                                                 // do postprocessing of pxels if enabled
					for (unsigned int j=0, index=0;j<width;j++)
					{
						//if (rowPtr[j] !=0)
						if (*(rowPtrs[count_row-start_row]+j) !=0)
						{ 
							index = int (*(rowPtrs[count_row-start_row]+j));
							*(rowPtrs[count_row-start_row]+j) = *(Maparray+index);
							//index = int (rowPtr[j]);
							//rowPtr[j] = *(Maparray+index);
						}
					}
					//png_write_row(pngPtr,rowPtr);                                 // write row to PNG file
#else
					//png_write_row(pngPtr,rowPtr);                                 // write row to PNG file

#endif
	               count_row++;
			       if ((count_row % WRITE_ROWBUFFER)== 0 || count_row == height)            // writing WRITE_ROWBUFFER rows to PNG filez
			       {
					   if ((count_row % WRITE_ROWBUFFER) ==0)
					   {
						   WriteRows(start_row,WRITE_ROWBUFFER);
						   start_row += WRITE_ROWBUFFER;
					   }
					   else
						   WriteRows(start_row,(count_row % WRITE_ROWBUFFER));
			       }
			}
			
			idx++;
			//cout << "ID of nonzero symbols : " << idx << endl;
			//cout << "temp :" << int(temp) << endl;
		}
		// If RLE of 0 is detected, reconstruct the run of 0s.
		else if(RLE[i] < MAX_CORNER_SYMBOL + M)
		{
			exponent = 1;
			count = 0;
			// Decode run count from M-ary representation
			while(RLE[i] >= MAX_CORNER_SYMBOL && RLE[i] < MAX_CORNER_SYMBOL + M)
			{
				count += exponent * (RLE[i] - MAX_CORNER_SYMBOL);
//				exponent *= M;
				exponent <<= RSHIFT_M;	 // If M is 2^N, use N bit right shift instead.
				i++;
			}
			i--;
			unsigned int x = idx % width; 
			idx += count;
			for (unsigned int j=x;j<(idx % width);j++)                        // write zero's in current row
			{
				current_row[j] = 0;
			}
		}

		// If RLE of EOB is detected, move to end of the block
		else if(RLE[i] < MAX_CORNER_SYMBOL + M + K)
		{
			exponent = 1;
			count = 0;
			// Decode EOB run count from M-ary representation
			while(RLE[i] >= MAX_CORNER_SYMBOL + M && RLE[i] < MAX_CORNER_SYMBOL + M + K)
			{
				count += exponent * (RLE[i] - MAX_CORNER_SYMBOL - M);
//				exponent *= K;
				exponent <<= RSHIFT_K;	 // If K is 2^N, use N bit right shift instead.
				i++;
			}
			i--;
			// Process the run of EOB
			for(unsigned int count_EOB=0; count_EOB<count; count_EOB++)
			{
				unsigned int x = idx % width;
				if(BlockSize * (x / BlockSize) + BlockSize < width) 
				{
					idx += BlockSize - (x % BlockSize);
					for (unsigned int j=x;j<(idx % width);j++)
			        {
						current_row[j] = 0;
			        }
				}
				else
				{
					idx += width - (x % width);
					for (unsigned int j=x;j< width;j++)                              // write zeros till end of block
			        {
						current_row[j] = 0;
			        }
					
					//cout << "count of row : " << count_row << endl;
					for (unsigned int j=0;j<width;j++)                // Inverse corner transform
			        {
						if (j>0)
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + *(rowPtrs[count_row-start_row]+j-1) + buffer_previous_row[j] - buffer_previous_row[j-1];
							//rowPtr[j] = current_row[j] + rowPtr[j-1] + buffer_previous_row[j] - buffer_previous_row[j-1];
#else
							//if (idx<width)predict=0;
							//else{
								predict = buffer_previous_row[j] + *(rowPtrs[count_row-start_row]+j-1) - buffer_previous_row[j-1];
								pb = abs(predict - *(rowPtrs[count_row-start_row]+j-1));                  //distances to  b, c,d
								pd = abs(predict - buffer_previous_row[j]);
								pc = abs(predict - buffer_previous_row[j-1]);
								if(pb <= pd && pb <= pc) predict = *(rowPtrs[count_row-start_row]+j-1);
								else if (pd <= pc) predict = buffer_previous_row[j];
								else predict = buffer_previous_row[j-1];
							//}
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;
#endif
						}
						else
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + buffer_previous_row[j];
							//rowPtr[j] = current_row[j] + buffer_previous_row[j];
#else						
							predict = buffer_previous_row[j];
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;;
#endif
						}
			        }

					memcpy(buffer_previous_row,rowPtrs[count_row-start_row],width*sizeof(char));         // put row data into previous row
					//memcpy(buffer_previous_row,rowPtr,width*sizeof(char));              // write current row into previous row

#ifdef POSTPROCESS                                                                      // do postprocessing if enabled
					for (unsigned int j=0, index=0;j<width;j++)
					{
						//if (rowPtr[j] !=0)
						if (*(rowPtrs[count_row-start_row]+j) !=0)
						{ 
							index = int (*(rowPtrs[count_row-start_row]+j));
							*(rowPtrs[count_row-start_row]+j) = *(Maparray+index);
							//index = int (rowPtr[j]);
							//rowPtr[j] = *(Maparray+index);
						}
					}
					//png_write_row(pngPtr,rowPtr);                                     // write row to PNG file
#else
					//png_write_row(pngPtr,rowPtr);                                     // write row to PNG file
#endif
					count_row++;
					if ((count_row % WRITE_ROWBUFFER)== 0 || count_row == height)
			        {
						if ((count_row % WRITE_ROWBUFFER) ==0)
						{
						   WriteRows(start_row,WRITE_ROWBUFFER);
						   start_row += WRITE_ROWBUFFER;
						}
					    else
						   WriteRows(start_row,(count_row % WRITE_ROWBUFFER));
			        }
					
                }
				
				//cout << "idx : " << idx << endl;
			}
		}
        
		else
		{
			cout << "[Error] Cannot decode the RLE stream..." << endl;
		}
		
		if(idx > image_size)
		{
			cout << "Out of Bound" << endl;
			break;
		}
		//cout << "count of rows : " << count_row << endl;
		//cout << "ID of nonzero symbols : " << idx << endl;
	}

	png_write_end(pngPtr,infoPtr);                       // end the writing of PNG file

	free(RLE);
	RLE = NULL;
	free(buffer_previous_row);
	free(current_row);
#ifdef DEBUG
	cout << "count of rows : " << count_row << endl;
	cout << "count of edges : " << count_edge << endl;
#endif
	return 0;
}


int ClassDecoder::EntropyDecoder_AC(string filename)
{
	// Run Arithmetic Decoding
#ifdef DEBUG
	cout << "Decoding Arithmetic Codes...... " << endl;
#endif
	unsigned char temp = 0;
	ac_decoder acd;
	ac_model acm;
	filename += ".enc";
	ac_decoder_init (&acd, filename.c_str());
	ac_model_init(&acm, MAX_CORNER_SYMBOL+M+K+1, NULL, 1);
	RLE = (unsigned short *) calloc(RLE_length, sizeof(unsigned short));
	//for(size_t i=0; i<RLE_length; i++)
	for(size_t i=0;; i++)
	{
		temp = ac_decode_symbol (&acd, &acm);
		if (temp != EOF_SYMBOL)
			RLE[i] = temp;
		else
		{ 
			cout << "End of file occured" << endl;
			break;
		}
		//RLE[i] = ac_decode_symbol (&acd, &acm);
		//cout << "Decoded RLE symbol : " << RLE[i] << endl;
	}
	ac_decoder_done (&acd);
	ac_model_done (&acm);
#ifdef DEBUG
	cout << "                                [Done]" << endl;
#endif
	return 0;
}

int ClassDecoder::inflate_decompression(string filename)
{
	RLE = (unsigned short *) calloc(RLE_length, sizeof(unsigned short));
	filename = filename + ".dft";
	int ret;
    unsigned have;
    z_stream strm;
	//int CHUNK = 32768;
    unsigned char in[32768];
    unsigned char out[32768];
	unsigned int i,count = 0;
	FILE *source;
	source = fopen(filename.data(),"rb");
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, 32768, source);
		//cout << strm.avail_in << endl;
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = 32768;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            //assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
			//cout << "ret:" << ret << endl;
            have = 32768 - strm.avail_out;
            /*if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }*/
			//cout << "have:" << have << endl;
			for(i = 0; i<have;i++)
			{
				RLE[i + count] = unsigned short(out[i]);
			}
			count = count + have;
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);
	cout << "inflated count : " << count << endl;
    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

#else                                      // If MEMORYSAVE is defined than do complete decoding in one step
int ClassDecoder::Complete_Decoding(string filename)
{

#ifdef DEBUG
	cout << "Decoding complete file...... " << endl;
#endif
	unsigned char temp_AC = 0;
	ac_decoder acd;
	ac_model acm;
	filename += ".enc";
	ac_decoder_init (&acd, filename.c_str());
	ac_model_init(&acm, MAX_CORNER_SYMBOL+M+K+1, NULL, 1);

	//----------------------perpare for RLE+EOB+Inverse transform --------------------//
	char *current_row;
	unsigned char *buffer_previous_row;
	if( (buffer_previous_row = (unsigned char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for previous row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}

	if( (current_row = (char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for current row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}

	/*if( (rowPtr = (png_bytep) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for png row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}*/
	unsigned int Rows = height;
	if (Rows > WRITE_ROWBUFFER) 
		Rows = WRITE_ROWBUFFER;

	rowPtrs = new png_bytep[Rows];                                   // allocating rowPtrs memory for first iteration of writing
	data1 = new char[width * Rows * bitdepth / 8];
	int stride = width * bitdepth / 8;
	for (size_t i = 0; i < Rows; i++)
	{
		png_uint_32 q =  i * stride;
		rowPtrs[i] = (png_bytep)data1 + q;                  // setting the values of rowPtrs     
	}
	
	unsigned long long idx = 0, image_size = ((unsigned long long)width)*((unsigned long long)height);
	//cout << "size:" << image_size << endl;
	unsigned int count = 0,count_row =0, count_edge = 0;
	unsigned int exponent = 1, start_row = 0;
	char temp = 0,flag = 0;
	unsigned short RLE_element = 0;

#ifdef PAETH
	int pb,pc,pd,predict;				// variables for PAETH filter implementation
#endif

	for (size_t i=0;i < RLE_length; i++)
	{
		if (flag == 0)                                          // flag is for checking whetehr RLE element has already been learned or not.
			RLE_element = ac_decode_symbol (&acd, &acm);

		if (RLE_element < MAX_CORNER_SYMBOL)
		{
			flag=0;
			temp = (char) RLE_element;
#ifndef PAETH
			if ((temp%2)==0) temp = temp/2;
			else temp = -((temp+1)/2);
#endif
			current_row[idx % width] = temp;
			if ((idx % width) == (width-1))                        // if reached at the end of row, start corner decoding and write to file
		    {
				count_edge++;
				for (unsigned int j=0;j<width;j++)                // Inverse corner transform
			        {
						if (j>0)
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + *(rowPtrs[count_row-start_row]+j-1) + buffer_previous_row[j] - buffer_previous_row[j-1];
							//rowPtr[j] = current_row[j] + rowPtr[j-1] + buffer_previous_row[j] - buffer_previous_row[j-1];
#else
								predict = buffer_previous_row[j] + *(rowPtrs[count_row-start_row]+j-1) - buffer_previous_row[j-1];
								pb = abs(predict - *(rowPtrs[count_row-start_row]+j-1));                  //distances to  b, c,d
								pd = abs(predict - buffer_previous_row[j]);
								pc = abs(predict - buffer_previous_row[j-1]);
								if(pb <= pd && pb <= pc) predict = *(rowPtrs[count_row-start_row]+j-1);
								else if (pd <= pc) predict = buffer_previous_row[j];
								else predict = buffer_previous_row[j-1];
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;
#endif
						}
						else
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + buffer_previous_row[j];
							//rowPtr[j] = current_row[j] + buffer_previous_row[j];
#else						
							predict = buffer_previous_row[j];
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;
#endif
						}
			        }
				      memcpy(buffer_previous_row,rowPtrs[count_row-start_row],width*sizeof(char));         // put row data into previous row
					//memcpy(buffer_previous_row,rowPtr,width*sizeof(char));         // put row data into previous row

#ifdef POSTPROCESS                                                                 // do postprocessing of pxels if enabled
					for (unsigned int j=0, index=0;j<width;j++)
					{
						//if (rowPtr[j] !=0)
						if (*(rowPtrs[count_row-start_row]+j) !=0)
						{ 
							index = int (*(rowPtrs[count_row-start_row]+j));
							*(rowPtrs[count_row-start_row]+j) = *(Maparray+index);
							//index = int (rowPtr[j]);
							//rowPtr[j] = *(Maparray+index);
						}
					}
					//png_write_row(pngPtr,rowPtr);                                 // write row to PNG file
#else
					//png_write_row(pngPtr,rowPtr);                                 // write row to PNG file

#endif
	               count_row++;
			       if ((count_row % WRITE_ROWBUFFER)== 0 || count_row == height)            // writing WRITE_ROWBUFFER rows to PNG file
			       {
					   if ((count_row % WRITE_ROWBUFFER) ==0)
					   {
						   WriteRows(start_row,WRITE_ROWBUFFER);
						   start_row += WRITE_ROWBUFFER;
					   }
					   else
						   WriteRows(start_row,(count_row % WRITE_ROWBUFFER));
			       }
			}
			
			idx++;
			//cout << "ID of nonzero symbols : " << idx << endl;
			//cout << "temp :" << int(temp) << endl;
		}
		// If RLE of 0 is detected, reconstruct the run of 0s.
		else if(RLE_element < MAX_CORNER_SYMBOL + M)
		{
			exponent = 1;
			count = 0;
			// Decode run count from M-ary representation
			while(RLE_element >= MAX_CORNER_SYMBOL && RLE_element < MAX_CORNER_SYMBOL + M)
			{
				count += exponent * (RLE_element - MAX_CORNER_SYMBOL);
//				exponent *= M;
				exponent <<= RSHIFT_M;	 // If M is 2^N, use N bit right shift instead.
				RLE_element = ac_decode_symbol (&acd, &acm);
				i++;
			}
			flag =1;                                                   // making flag=1 to show that next RLE element has been read
			i--;
			unsigned int x = idx % width; 
			idx += count;
			for (unsigned int j=x;j<(idx % width);j++)                        // write zero's in current row
			{
				current_row[j] = 0;
			}
		}

		// If RLE of EOB is detected, move to end of the block
		else if(RLE_element < MAX_CORNER_SYMBOL + M + K)
		{
			exponent = 1;
			count = 0;
			// Decode EOB run count from M-ary representation
			while(RLE_element >= MAX_CORNER_SYMBOL + M && RLE_element < MAX_CORNER_SYMBOL + M + K)
			{
				count += exponent * (RLE_element - MAX_CORNER_SYMBOL - M);
//				exponent *= K;
				exponent <<= RSHIFT_K;	 // If K is 2^N, use N bit right shift instead.
				RLE_element = ac_decode_symbol (&acd, &acm);
				i++;
			}
			flag = 1;                                             // making flag=1 to show that next RLE element has been read
			i--;
			// Process the run of EOB
			for(unsigned int count_EOB=0; count_EOB<count; count_EOB++)
			{
				unsigned int x = idx % width;
				if(BlockSize * (x / BlockSize) + BlockSize < width) 
				{
					idx += BlockSize - (x % BlockSize);
					for (unsigned int j=x;j<(idx % width);j++)
			        {
						current_row[j] = 0;
			        }
				}
				else
				{
					idx += width - (x % width);
					for (unsigned int j=x;j< width;j++)                              // write zeros till end of block
			        {
						current_row[j] = 0;
			        }
					
					//cout << "count of row : " << count_row << endl;
					for (unsigned int j=0;j<width;j++)                            // Inverse corner transform of image
			        {
						if (j>0)
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + *(rowPtrs[count_row-start_row]+j-1) + buffer_previous_row[j] - buffer_previous_row[j-1];
							//rowPtr[j] = current_row[j] + rowPtr[j-1] + buffer_previous_row[j] - buffer_previous_row[j-1];
#else
								predict = buffer_previous_row[j] + *(rowPtrs[count_row-start_row]+j-1) - buffer_previous_row[j-1];
								pb = abs(predict - *(rowPtrs[count_row-start_row]+j-1));                  //distances to  b, c,d
								pd = abs(predict - buffer_previous_row[j]);
								pc = abs(predict - buffer_previous_row[j-1]);
								if(pb <= pd && pb <= pc) predict = *(rowPtrs[count_row-start_row]+j-1);
								else if (pd <= pc) predict = buffer_previous_row[j];
								else predict = buffer_previous_row[j-1];
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;
#endif
						}
						else
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + buffer_previous_row[j];
							//rowPtr[j] = current_row[j] + buffer_previous_row[j];
#else						
							predict = buffer_previous_row[j];
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;;
#endif
						}
			        }

					memcpy(buffer_previous_row,rowPtrs[count_row-start_row],width*sizeof(char));         // put row data into previous row
					//memcpy(buffer_previous_row,rowPtr,width*sizeof(char));              // write current row into previous row

#ifdef POSTPROCESS                                                                      // do postprocessing if enabled
					for (unsigned int j=0, index=0;j<width;j++)
					{
						//if (rowPtr[j] !=0)
						if (*(rowPtrs[count_row-start_row]+j) !=0)
						{ 
							index = int (*(rowPtrs[count_row-start_row]+j));
							*(rowPtrs[count_row-start_row]+j) = *(Maparray+index);
							//index = int (rowPtr[j]);
							//rowPtr[j] = *(Maparray+index);
						}
					}
					//png_write_row(pngPtr,rowPtr);                                     // write row to PNG file
#else
					//png_write_row(pngPtr,rowPtr);                                     // write row to PNG file
#endif
					count_row++;
					if ((count_row % WRITE_ROWBUFFER)== 0 || count_row == height)
			        {
						if ((count_row % WRITE_ROWBUFFER) ==0)
						{
						   WriteRows(start_row,WRITE_ROWBUFFER);
						   start_row += WRITE_ROWBUFFER;
						}
					    else
						   WriteRows(start_row,(count_row % WRITE_ROWBUFFER));
			        }
					
                }
				
				//cout << "idx : " << idx << endl;
			}
		}
        
		else
		{
			cout << "[Error] Cannot decode the RLE stream..." << endl;
		}
		
		if(idx > image_size)
		{
			cout << "Out of Bound" << endl;
			break;
		}
		//cout << "count of rows : " << count_row << endl;
		//cout << "ID of nonzero symbols : " << idx << endl;
	}
	if(flag == 0)
	    temp_AC = ac_decode_symbol (&acd, &acm);
	else temp_AC = RLE_element;

	if (temp_AC == EOF_SYMBOL)                            // check for end of file symbol
		cout << "End of file occured" << endl;

	png_write_end(pngPtr,infoPtr);                       // end the writing of PNG file

	ac_decoder_done (&acd);
	ac_model_done (&acm);
	free(RLE);
	RLE = NULL;
	free(buffer_previous_row);
	free(current_row);
#ifdef DEBUG
	cout << "count of rows : " << count_row << endl;
	cout << "count of edges : " << count_edge << endl;
#endif
	return 0;
}

#endif   
//#ifdef DEBUG
int ClassDecoder::LineDiff_Decompression()
{
	
	//-----------------------------LineDiff Prefix decoding-----------------------
	unsigned long long b = 0; 
	unsigned int count_RLE = 0;
	unsigned short RLE_element = 0;
	unsigned long long image_size = ((unsigned long long)width)*((unsigned long long)height);

	/*if( (RLE = (unsigned short *) calloc(RLE_length, sizeof(unsigned short))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for RLE... Please check memory and try again later..." << endl;
		exit(-1);
	}*/
	//cout << bitlength << endl;
	
	if( (bool_RLE = (bool *) calloc(bitlength, sizeof(bool))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for bool RLE... Please check memory and try again later..." << endl;
		exit(-1);
	}
	//cout <<"fine till:" << endl;
	unsigned long long count_bits=0;
	
	for(int i=0;i<RLE_length;i++)
	{
		bitset<16> bits(RLE[i]);
		
		for (int j=0;j<16;j++)
		{
			bool_RLE[count_bits] = bits[j];
			count_bits++;
			if(count_bits==bitlength)
				j=16;
		}
		
		if(count_bits==bitlength)
			i=RLE_length;
	}
	//unsigned short a = 1057;
	/*bitset<16> bits(a);
	for (int j=0;j<16;j++)
		cout << bits[j] << endl;*/

	//cout<<count_bits << endl;
	free(RLE);
	if( (RLE = (unsigned short *) calloc((bitlength/2)+1, sizeof(unsigned short))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for RLE... Please check memory and try again later..." << endl;
		exit(-1);
	}
	
	while(b < bitlength)
	{
		//cout << b << '\t';
		if(bool_RLE[b] == 0)
		{
			b++;
			if(bool_RLE[b] == 0)
			{
				b++;
				RLE[count_RLE] = 1056; 
				count_RLE++;
				continue;
			}
			else
			{
				b++;
				RLE[count_RLE] = 1057; 
				count_RLE++;
				continue;
			}
		}
		else
		{
			b++;
			if(bool_RLE[b] == 0)									// 10 case
			{
				b++;
				if(bool_RLE[b] == 0)								// 100 case
				{
					b++;
					if(bool_RLE[b] == 0)                              // 1000 case
					{
						b++;
						RLE[count_RLE] = 1055; 
						count_RLE++;
						continue;
					}
					else												// 1001 case
					{
						b++;
						RLE[count_RLE] = 1024; 
						count_RLE++;
						continue;
					}
				}
				else													//101 case
				{
					b++;
					for(int j=0;j<5;j++)
					{
						if(j>0)
							RLE_element += (2<<(j-1))*bool_RLE[b];
						else
							RLE_element += bool_RLE[b];
						b++;
					}
					RLE_element = RLE_element + 1024;
					//cout << RLE_element << endl;
					RLE[count_RLE] = RLE_element;
					RLE_element = 0;
					count_RLE++; 
					continue;
				}
			}
			else														// 11 case
			{
				b++;
				if (bool_RLE[b] == 0)									// 110 case
				{
					b++;
					for(int j=0;j<5;j++)
					{
						if(j>0)
							RLE_element += (2<<(j-1))*bool_RLE[b];
						else
							RLE_element += bool_RLE[b];
						b++;
					}
					RLE[count_RLE] = RLE_element;
					//cout << RLE_element << endl;
					RLE_element = 0;
					count_RLE++; 
					continue;
				}
				else													// 111 case
				{
					b++;
					for(int j=0;j<10;j++)
					{
						if(j>0)
							RLE_element += (2<<(j-1))*bool_RLE[b];
						else
							RLE_element += bool_RLE[b];
						b++;
					}
					//RLE_element += 32; 
					RLE[count_RLE] = RLE_element;
					//cout << RLE_element << endl;
					RLE_element = 0;
					count_RLE++; 
					continue;
				}
			}

		}
	}

	//cout << endl << count_RLE << endl;
	//cout << RLE_length << endl;
	RLE_length = count_RLE;

	/*for (int j=0;j<RLE_length;j++)
		cout << RLE[j] << '\t';*/
	
	
	free(bool_RLE);
	//------------------------------LineDiff RLE decompaction-----------------------
	count_RLE = 0;
	unsigned short *temp_RLE;
	
	if( (temp_RLE = (unsigned short *) calloc(image_size/20, sizeof(unsigned short))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for temp RLE... Please check memory and try again later..." << endl;
		exit(-1);
	}

	//cout << "image size : " << image_size << endl;
	
	unsigned int i=0;
	while (i < RLE_length)
	{

		if (RLE[i] == 1057)
		{
			if(i==0)              // for first symbol
			{
				temp_RLE[count_RLE] = 1056;             // write DUP
				count_RLE++;
				temp_RLE[count_RLE] = RLE[i];		// write END
				count_RLE++;
				i++;
				continue;
			}
			if (RLE[i-1] == 1057)						// if previous symbol was also END
			{
				temp_RLE[count_RLE] = 1056;             // write DUP
				count_RLE++;
				temp_RLE[count_RLE] = RLE[i];           // write END
				count_RLE++;
				i++;
				continue;
			}
			else{
				temp_RLE[count_RLE] = RLE[i];
				count_RLE++;
				i++;
				continue;
			}

		}

		if(RLE[i] < 1024 && RLE[i] > 1)
		{
			if (i==0)
			{
				temp_RLE[count_RLE] = 1056;				// write DUP
				temp_RLE[count_RLE+1] = RLE[i];
				count_RLE += 2;
				i++;
				continue;
			}
			if(RLE[i-1]== 1057)							// if start of new line
			{
				temp_RLE[count_RLE] = 1056;				// put DUP at the start
				temp_RLE[count_RLE+1] = RLE[i];
				count_RLE += 2;
				i++;
				continue;
			}
			else{
				temp_RLE[count_RLE] = RLE[i];
				count_RLE++;
				i++;
				continue;
			}
		}
		if(RLE[i] > 1023 && RLE[i] < 1057)				//if image(gray) symbols or DUP 
		{
			if(RLE[i+1] > 1023 && RLE[i+1] < 1057)
			{
				temp_RLE[count_RLE] = RLE[i];
				temp_RLE[count_RLE+1] = 1;
				count_RLE += 2;
				i++;
				continue;
			}
			else{
				temp_RLE[count_RLE] = RLE[i];
				count_RLE++;
				i++;
				continue;
			}
		}

		else{
				temp_RLE[count_RLE] = RLE[i];
				count_RLE++;
				i++;
				continue;
			}
	}

	
	//-----------------------PNG ---------------------------------
	unsigned char *current_row;
	unsigned char *buffer_previous_row;
	if( (buffer_previous_row = (unsigned char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for previous row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}

	if( (current_row = (unsigned char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for current row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}

	unsigned int Rows = height;
	if (Rows > WRITE_ROWBUFFER) 
		Rows = WRITE_ROWBUFFER;

	rowPtrs = new png_bytep[Rows];                                   // allocating rowPtrs memory for first iteration of writing
	data1 = new char[width * Rows * bitdepth / 8];
	int stride = width * bitdepth / 8;
	for (size_t i = 0; i < Rows; i++)
	{
		png_uint_32 q =  i * stride;
		rowPtrs[i] = (png_bytep)data1 + q;                  // setting the values of rowPtrs     
	}

	unsigned long long idx = 0;
	unsigned int count_row =0, start_row = 0,index = 0;
	char temp = 0;
	i=0;
	while (i<count_RLE)
	{
		if(temp_RLE[i] > 1023 && temp_RLE[i] < 1056)               // If the current symbol is a gray symbol
		{
			if(temp_RLE[i+1] != 1057)                 // if not END symbol
			{
				for(unsigned int j=0; j < temp_RLE[i+1];j++)
				{
					*(rowPtrs[count_row-start_row]+(idx%width)) = unsigned char(temp_RLE[i] - 1024);
					//current_row[idx%width] = unsigned char(temp_RLE[i] - 1024);
					//cout << int(current_row[idx%width]) << endl;
					idx++;
				}
				i += 2;
				continue;
			}
			else if (temp_RLE[i+1] == 1057)
			{
				index = idx%width;
				while(index < (width-1)){
					 *(rowPtrs[count_row-start_row]+index) = unsigned char(temp_RLE[i] - 1024);
					//current_row[idx%width] = unsigned char(temp_RLE[i] - 1024);
					//idx++;
					index++;
				} 
				index = index - idx%width;
				idx = idx+index;
				*(rowPtrs[count_row-start_row]+(idx%width)) = unsigned char(temp_RLE[i] - 1024);
				//current_row[idx%width] = unsigned char(temp_RLE[i] - 1024);
				idx++;

				/*for (unsigned int j=0;j<width;j++)
				{
					*(rowPtrs[count_row-start_row]+j) = current_row[j];
				}*/
				memcpy(buffer_previous_row,rowPtrs[count_row-start_row],width*sizeof(char));         // put row data into previous row

				count_row++;
			       if ((count_row % WRITE_ROWBUFFER)== 0 || count_row == height)            // writing WRITE_ROWBUFFER rows to PNG file
			       {
					   if ((count_row % WRITE_ROWBUFFER) ==0)
					   {
						   WriteRows(start_row,WRITE_ROWBUFFER);
						   start_row += WRITE_ROWBUFFER;
					   }
					   else
						   WriteRows(start_row,(count_row % WRITE_ROWBUFFER));
			       }

				i += 2;
				continue;
			}
		}

		if(temp_RLE[i] == 1056)                     // if DUP symbol occurs
		{
			if(temp_RLE[i+1] != 1057)
			{
				index = idx%width;
				for(unsigned int j=0; j < temp_RLE[i+1];j++)
				{
					*(rowPtrs[count_row-start_row]+index) = buffer_previous_row[index];
					//current_row[idx%width] = buffer_previous_row[idx%width];
					idx++;
					index = idx%width;
				}
				i += 2;
				continue;
			}
			else if(temp_RLE[i+1] == 1057)
			{
				index = idx%width;
				while(index < (width-1)){
					*(rowPtrs[count_row-start_row]+index) = buffer_previous_row[index];
					//current_row[idx%width] = buffer_previous_row[idx%width];
					//idx++;
					index++;
				}
				index = index - idx%width;
				idx = idx + index;
				*(rowPtrs[count_row-start_row]+(idx%width)) = buffer_previous_row[idx%width];
				//current_row[idx%width] = buffer_previous_row[idx%width];
				idx++;

				/*for (unsigned int j=0;j<width;j++)
				{
					*(rowPtrs[count_row-start_row]+j) = current_row[j];
				}*/
				memcpy(buffer_previous_row,rowPtrs[count_row-start_row],width*sizeof(char));         // put row data into previous row

				count_row++;
			       if ((count_row % WRITE_ROWBUFFER)== 0 || count_row == height)            // writing WRITE_ROWBUFFER rows to PNG file
			       {
					   if ((count_row % WRITE_ROWBUFFER) ==0)
					   {
						   WriteRows(start_row,WRITE_ROWBUFFER);
						   start_row += WRITE_ROWBUFFER;
					   }
					   else
						   WriteRows(start_row,(count_row % WRITE_ROWBUFFER));
			       }

				i += 2;
				continue;
			}
		}

		if(idx > image_size)
		{
			cout << "Out of Bound" << endl;
			break;
		}
	}

	//cout << endl<< idx << endl;
	free(temp_RLE);
	free(buffer_previous_row);
	free(current_row);
	return 0;
}
//#endif

void ClassDecoder::ReadRLE(string filename,int flag)
{
	unsigned int start,end;
	start = clock();
	if(flag)
	{
		RLE = (unsigned short *) calloc(RLE_length, sizeof(unsigned short));
		filename += ".txt";
		ifstream inTXT;
		inTXT.open(filename);
		char *Data = new char[RLE_length+1];
		inTXT.read(Data, RLE_length+1);
		
		for (int i=0; i<RLE_length;i++)
		{
			RLE[i] = unsigned char (unsigned char(Data[i] - '0') + 128);
			//cout << RLE[i] << '\t' << Data[i] << endl;
		}
		
		inTXT.close();
	}
	else
	{
		RLE = (unsigned short *) calloc(RLE_length, sizeof(unsigned short));
		filename += ".line";
		ifstream in(filename,ios::binary);
		int i = 0;
		
		while ((!in.eof())&&(i<RLE_length))
		{
             std::string inp;
             getline(in,inp);
			 RLE[i] = stoi(inp);
             //cout << RLE[i] << endl;
			 i++;
		}
		
		in.close();
	}
	end = clock();
	cout << "\t" << "\t" << "               Compressed file reading time = " << ((double) (end-start)) / ((double) 1000) << endl;
}