/*
 *  ClassEncoder.cpp
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */
#ifndef ClassEncoder_H_
#include "ClassEncoder.h"
#endif

#ifndef AC_HEADER
#include "AC.h"
#endif
#include <algorithm>
#include <math.h>
#include <bitset>
#define logX(x, y) log((double) x)/log((double) y)
#define log2(x) log((double) x)/log(2.0)

/* Constructor */
ClassEncoder::ClassEncoder()
{
	//MAX_SYMBOL = 0;
}

/* Destructor */
ClassEncoder::~ClassEncoder()
{
}

//corner transform
int ClassEncoder::Transform_RLE_EOB(unsigned char *Map, int No_of_Map)
{
    #ifdef DEBUG
	cout << "Peforming Transform............ " << endl;
    #endif

	#ifdef DEBUG
	cout << "Part II: Corner Coding......... " << endl;
    #ifdef PREPROCESS
	cout << "Number of Map elements        : " << No_of_Map << endl;
    #endif
    #endif

	size_t count_EOB = 0, count_RLE = 0;
    RLE_length = 0;
	//cout << "RLE size :" << ((unsigned long long)width)*((unsigned long long)height)/10 << endl;
	if( (RLE = (unsigned short *) calloc(((unsigned long long)width)*((unsigned long long)height)/10, sizeof(unsigned short))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for RLE... Please check memory and try again later..." << endl;
		exit(-1);
	}

	unsigned char *buffer_previous_row;
	if( (buffer_previous_row = (unsigned char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}

	int temp = 0, Maxtemp = 0, Mintemp = 0;
	unsigned char a = 0, b = 0, c = 0, d = 0;

#ifdef PAETH
	int pb,pc,pd;				// variables for PAETH filter implementation
#endif
#ifdef DEBUG
	unsigned int count = 0,count1 = 0;
#endif

	unsigned int new_buffer_length = ROWBUFFER;
	for(unsigned int row_number=0; row_number<height;)                //reading number of rows starting from row_number
	{   
		if ((row_number + ROWBUFFER) < height)
			ReadRows(row_number,ROWBUFFER);                           
		else
		{  
			new_buffer_length = (height - row_number);                // check and modify for last chunk of row buffer
            ReadRows(row_number,new_buffer_length);
		}
		for(unsigned int y=0; y<ROWBUFFER,y<new_buffer_length; y++)            // loop for rows
		{

		    for(unsigned int x=0; x<width; x++)                               // loop for column
		    {
				if (x > 0)
				{
					a = (*(rowPtrs[y]+x));
					b = (*(rowPtrs[y]+x-1));
					c = buffer_previous_row[x-1];
					d = buffer_previous_row[x];
#ifdef PREPROCESS
					if (!(a == 0 && b == 0 && c == 0 && d == 0))                
					{
					    for(int z=1; z < No_of_Map; z++)
					    {
						   if (a == Map[z]) a = z;
						   if (b == Map[z]) b = z;
						   if (c == Map[z]) c = z;
						   if (d == Map[z]) d = z;
					    }
					}
#endif
#ifndef PAETH
					temp = a - b - (d - c);          // corner transform of grayscale images
#else
					//if (y==0 && row_number)temp=0;             // temp is prdicted value
					//else{
						temp = b + d - c;
						pb = abs(temp - b);                  //distances to a, b, c
						pd = abs(temp - d);
						pc = abs(temp - c);
						if(pb <= pd && pb <= pc) temp = b;
						else if (pd <= pc) temp = d;
						else temp = c;
					//}
													// predicted value in PAETH filter
#endif
#ifdef DEBUG
					if(a!=0) count1++;                         // checking number of non-zero elements
#endif

				}

				else if (x == 0)                     // corner transform for first column
				{
					a = (*(rowPtrs[y]+x));
					d = buffer_previous_row[x];
#ifdef PREPROCESS
					if (!(a == 0 && d == 0))
					{
					    for(int z=1; z < No_of_Map; z++)
					    {
						   if (a == Map[z]) a = z;
						   if (d == Map[z]) d = z;
					    }
					}
#endif
#ifndef PAETH
					temp = a - d;                  // corner transform for first column
#else
					temp = d;					   // predicted value in PAETH filter
#endif

#ifdef DEBUG
					if(a != 0) count1++;           // counting nonzero elements in original image
#endif
				}
#ifdef DEBUG
				if (temp > Maxtemp) Maxtemp = temp;
				if (temp < Mintemp) Mintemp = temp;
				if (temp!=0)                                             // checking for nonzero elements after corner transform
				{
					count++;
				    //cout << "x:" << x << '\t' << "y:" << y << '\t' << "value:" << temp << endl;
			    }
				
#endif

#ifndef PAETH
				if (temp < 0) temp = -2*temp - 1;                               // make negative symbols as positive
				else temp = 2*temp;                                             //make +ve symbols twice there number
#else
					temp = (a - temp)%32;									// Paeth filter
				if(temp<0) temp = temp+32;
#endif

				// start of RLE Encoding ..........................................
				if (temp == 0)                                                  
				{
					//count the run length
					count_RLE++;
					if((x % BlockSize == BlockSize-1) || (x == width-1))
					//if(x % BlockSize == BlockSize-1)
					{
						count_RLE = 0;
						count_EOB++;
					}
				}
				//if (temp !=0 && (x == width-1)) count_EOB++;

				if((temp > 0 && temp < MAX_CORNER_SYMBOL) || ((x == width-1) && (y == height-row_number-1)))
			    {

					if(count_EOB > 0)
				    {
						// Replace count with K-ary representation (MAX_CORNER_SYMBOL+MAX_SYMBOL+M, ..., MAX_CORNER_SYMBOL+M+K-1)
					    unsigned int repeat = (unsigned int) (logX(count_EOB, K));
					    for(unsigned int r=0; r<=repeat; r++)
					    {
							RLE[RLE_length] = MAX_CORNER_SYMBOL + M + (count_EOB % K);
						    RLE_length++;
//						    count_EOB /= K;
						    count_EOB >>= RSHIFT_K; // If K is 2^N, use N bit right shift instead.
					    }
					    count_EOB = 0;
				    }
					
			        if (count_RLE > 0)
					{
						unsigned int repeat = (unsigned int) (logX(count_RLE, M));
					    for(unsigned int r=0; r<=repeat; r++)
					    {
							RLE[RLE_length] = MAX_CORNER_SYMBOL + (count_RLE % M);
						    RLE_length++;
//						    count /= M;
						    count_RLE >>= RSHIFT_M; // If M is 2^N, use N bit right shift instead.
					    }
						count_RLE = 0;
			        }
					if (temp != 0)
					{
						RLE[RLE_length] = temp;
				        RLE_length++;
					}
				 }
				else if (temp >= MAX_CORNER_SYMBOL)
				{
					cout << "corner symbol out of range :" << endl;
					exit(1);
				}

			}
			
			memcpy(buffer_previous_row,rowPtrs[y],width*sizeof(char));             // make current row as previous row and update current row
		}

		row_number = row_number + ROWBUFFER;
		
	}
	free(buffer_previous_row);                                                   // free buffer_previous_row
#ifdef DEBUG

	cout << " Total elements in image                    : " << ((unsigned long long) width)*((unsigned long long)height) << endl;
	cout << " original number of nonzero elements        : " << count1 << endl;
	cout << " Number of nonzero elements in corner image : " << count << endl;
	cout << " Maximum value of corner symbol             : " << Maxtemp << endl;
	cout << " Minimum value of corner symbol             : " << Mintemp << endl;
	cout << " RLE length                                 : " << RLE_length << endl;
	/*for (int i=0; i<RLE_length;i++)
	{
		cout << RLE[i] << '\t';
	}
	cout << endl;*/
#endif

	return 0;
}

int ClassEncoder::EntropyEncoder_AC(string filename)
{
	/* Arithmetic Coding */
	// Initialize Arithmetic Coding
	unsigned int ac_bytes = 0;	// Size of the compressed file
	unsigned int ac_bits = 0;
	filename += ".enc";
	ac_encoder ace;
	ac_model acm;
	ac_encoder_init (&ace, filename.c_str());
	
	// Encode each symbol using AC
	ac_model_init (&acm, MAX_CORNER_SYMBOL+M+K+1, NULL, 1);
	for(unsigned int j=0; j<RLE_length; j++)
	{
		ac_encode_symbol(&ace, &acm, RLE[j]);
	}
	ac_encode_symbol(&ace, &acm, EOF_SYMBOL);
	// Finalize Arithmetic Coder
	ac_encoder_done (&ace);
	ac_model_done (&acm);
	ac_bits = ac_encoder_bits (&ace);
	if ((ac_bits%8) !=0)
		ac_bytes = (int) ((ac_bits/8)+1);
	else
		ac_bytes = (int) (ac_bits/8);
	
#ifdef DEBUG
	cout << "                                [Done]" << endl;
	//cout << "Compressed File Size after AC Encoding in byte = " << ac_bytes << endl;
	cout << "Compressed File Size after AC Encoding in bits = " << ac_bits << endl;
#endif
	cout << "RLE length               : " << RLE_length << endl;
	cout << "Arithmatic encoded bytes : " << ac_bytes << endl;
	cout.precision(15);
	double c_ratio;
	c_ratio = double(width)*double(height)/double(ac_bytes+12);
	cout << "\t" << "approx. compression ratio : " << c_ratio << endl;
	return ac_bytes;
}


int ClassEncoder::deflate_compression(string filename)
{
	//filename +=".txt";
	string filename_dest = filename + ".dft"; 
	int ret, flush;
	unsigned int i,count =0;
    unsigned have;
    z_stream strm;
    unsigned char in[32768];
    unsigned char out[32768];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFLATED);
    if (ret != Z_OK)
        return ret;
	FILE *dest;
	//FILE *source;
	//source = fopen(filename.data(),"r");
	dest = fopen(filename_dest.data(),"wb");
    /* compress until end of file */
    do {
        //strm.avail_in = fread(in, 1, 32768, source);
		count++;
		
		for (i =(count-1)*32768; i<(count*32768) && i<RLE_length;i++)
		{
			in[i-(count-1)*32768] = unsigned char(RLE[i]) ;
		}
		strm.avail_in = i - (count-1)*32768;
		
        /*if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }*/
		if((i)%(RLE_length)==0) {
			flush = Z_FINISH;
		}
		else flush = Z_NO_FLUSH;
       // flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;

        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = 32768;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            //assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = 32768 - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when last data in file processed */
    } while (flush != Z_FINISH);

    /* clean up and return */
    (void)deflateEnd(&strm);
	//fclose(source);
	fclose(dest);
	return 0;
}


int ClassEncoder::Preprocessing()
{
	#ifdef DEBUG
	cout << "Peforming Preprocessing............ " << endl;
    #endif

	if( (Maparray = (unsigned char *) calloc(MAPSIZE, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for Maparray... Please check memory and try again later..." << endl;
		exit(-1);
	}

	unsigned int new_buffer_length = ROWBUFFER, count = 1, flag = 1;
	int temp = 0;
	for(unsigned int row_number=0; row_number<height;)               //reading number of rows starting from row_number
	{   
		if ((row_number + ROWBUFFER) < height)
			ReadRows(row_number,ROWBUFFER);                           
		else
		{  
			new_buffer_length = (height - row_number);               // check and modify for last chunk of row buffer
            ReadRows(row_number,new_buffer_length);
		}
		for(unsigned int y=0; y<ROWBUFFER,y<new_buffer_length; y++)  // loop for rows
		{

		   for(unsigned int x=0; x<width; x++)                      // loop for columns
		   {
		     temp = (*(rowPtrs[y]+x));                            // putting value in temp for faster processing
			 if (temp != 0)
			 {

				for (unsigned int z=0; z<=count, z<MAPSIZE; z++)                           // checking values in Maparray
				{
                    if(Maparray[z] == temp)
					{
						flag=0;
						break;
					}
				}

			    if (flag==1)
				{
						Maparray[count] = temp;
						count++;
						if (count == 32) break; 
						flag = 0;
				}
				if (flag ==0)flag++;
			 }
			 if (count == 32) break; 

		    }
			
		}
		if (count == 32) break;
		row_number = row_number + ROWBUFFER;                           //updating starting row
		
	}

	sort(Maparray,Maparray + count);                                  // sorting Maparray
#ifdef DEBUG
	for (unsigned int i=0; i < count; i++)
	{
		cout << " Maparray : " << int(Maparray[i]) << endl;
	}
#endif
	return count;
}

int ClassEncoder::histogram()
{
	unsigned int histo[256];
	for (int i=0; i <256;i++) histo[i] = 0;

	for(int i=0; i< RLE_length ;i++)
	{
		histo[RLE[i]]++;
	}
	for(int j=0; j<256; j++)
	{
		cout << histo[j] << endl;
	}
	return 0;
}


int ClassEncoder::Triline_Compression()
{
	#ifdef DEBUG
	cout << "Triline coding......... " << endl;
    #endif

	size_t count_RLE = 0,count_zero = 0;
    RLE_length = 0;

	//cout << "RLE size :" << ((unsigned long long)width)*((unsigned long long)height)/10 << endl;
	if( (RLE = (unsigned short *) calloc(((unsigned long long)width)*((unsigned long long)height)/10, sizeof(unsigned short))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for RLE... Please check memory and try again later..." << endl;
		exit(-1);
	}

	unsigned short *superrow;
	unsigned int new_buffer_length = ROWBUFFER;
	if( (superrow = (unsigned short *) calloc(width, sizeof(unsigned short))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for superrow... Please check memory and try again later..." << endl;
		exit(-1);
	}

	for(unsigned int row_number=0; row_number<height;)               //reading number of rows starting from row_number
	{   
		if ((row_number + ROWBUFFER) < height)
			ReadRows(row_number,ROWBUFFER);                           
		else
		{  
			new_buffer_length = (height - row_number);               // check and modify for last chunk of row buffer
            ReadRows(row_number,new_buffer_length);
		}

		for(unsigned int x=0; x<width; x++)
		{
			superrow[x] = 1024*(*(rowPtrs[0]+x)) + 32*(*(rowPtrs[1]+x)) + (*(rowPtrs[2]+x)) ;

			if (superrow[x] == 0)
			{
				count_RLE = 0;							// handle the zero case
				count_zero++;
			}
			
			if (x>0 && superrow[x] == superrow[x-1])
			{
				count_zero = 0;						// handle RLE case
				count_RLE++;
			}

		}


		row_number = row_number + ROWBUFFER;                           //updating starting row
	}


	return 0;
}

//---------------------------------------------------Line DIff Compression starts------------------------------------------------------

unsigned long long ClassEncoder::LineDiff(string filename)
{
	#ifdef DEBUG
	cout << "LineDiff coding......... " << endl;
    #endif

	size_t count_RLE = 0;
    RLE_length = 0;

	//cout << "RLE size :" << ((unsigned long long)width)*((unsigned long long)height)/10 << endl;
	if( (RLE = (unsigned short *) calloc(((unsigned long long)width)*((unsigned long long)height)/10, sizeof(unsigned short))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for RLE... Please check memory and try again later..." << endl;
		exit(-1);
	}

	unsigned char *buffer_previous_row;
	if( (buffer_previous_row = (unsigned char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}

	int firstOP_flag = 1;                                // first OP in scanline
	unsigned int L = 0, OP = 1056;
	int flag =0;

#ifdef DEBUG
	unsigned int count = 0,count1 = 0;
#endif

	unsigned int new_buffer_length = ROWBUFFER;
	for(unsigned int row_number=0; row_number<height;)                //reading number of rows starting from row_number
	{   
		if ((row_number + ROWBUFFER) < height)
			ReadRows(row_number,ROWBUFFER);                           
		else
		{  
			new_buffer_length = (height - row_number);                // check and modify for last chunk of row buffer
            ReadRows(row_number,new_buffer_length);
		}
		for(unsigned int y=0; y<ROWBUFFER,y<new_buffer_length; y++)            // loop for rows
		{

		    for(unsigned int x=0; x<width; x++)   
			{
				if(x == width-1)							// handling cases for last colummn
				{
					if((*(rowPtrs[y]+x)) == buffer_previous_row[x])				
					{
						if (OP == 1056)								// if current running OP symbol is DUP 
						{
							RLE[RLE_length] = OP;
							RLE[RLE_length + 1] = 1057;  	
							RLE_length = RLE_length + 2;
							L = 0;
						}
						else if(OP == (*(rowPtrs[y]+x)+1024))
						{
							RLE[RLE_length] = OP;
							RLE[RLE_length + 1] = 1057;  	
							RLE_length = RLE_length + 2;
							L = 0;
						}
						else										// if current running OP symbol is not DUP 
						{
							RLE[RLE_length] = OP;
							RLE[RLE_length + 1] = L;               
							RLE[RLE_length + 2] =(*(rowPtrs[y]+x)+1024);
							RLE[RLE_length + 3] = 1057;
							RLE_length = RLE_length + 4;
							L = 0;
						}
					}
					else if ((*(rowPtrs[y]+x)+1024) != OP)
					{
						RLE[RLE_length] = OP;
						RLE[RLE_length + 1] = L;               
						RLE[RLE_length + 2] =(*(rowPtrs[y]+x)+1024);
						RLE[RLE_length + 3] = 1057;
						RLE_length = RLE_length + 4;
						L = 0;
					}
					else
					{
						RLE[RLE_length] = OP;
						RLE[RLE_length + 1] = 1057;  	
						RLE_length = RLE_length + 2;
						L = 0;
					}
				}

				else if((*(rowPtrs[y]+x)) == buffer_previous_row[x])
				{
					if (!(x>0 && (*(rowPtrs[y]+x)+1024) == OP))
					{
					 if(OP != 1056)                 // write collected symbols
					 { 
						if (L <=1023 && L > 0)
						{
							RLE[RLE_length] = OP;
							RLE[RLE_length + 1] = L;
							RLE_length = RLE_length + 2;
							L = 0;
						}
						if(L > 1023)
						{
							while (L > 1023)
							{
								RLE[RLE_length] = OP;
								RLE[RLE_length + 1] = 1023;
								RLE_length = RLE_length + 2;
								L = L - 1023;
							}
							RLE[RLE_length] = OP;
							RLE[RLE_length + 1] = L;
							RLE_length = RLE_length + 2;
							L = 0;
						}
						
						OP = 1056;                // OP = DUP
					}
				   }

				L++;	
				}

				else
				{
					if(OP != *(rowPtrs[y]+x) + 1024)
					{
						if (L <=1023 && L > 0)
						{
							RLE[RLE_length] = OP;
							RLE[RLE_length + 1] = L;
							RLE_length = RLE_length+2;
							L = 0;
						}
						if (L > 1023)
						{
							while (L > 1023)
							{
								RLE[RLE_length] = OP;
								RLE[RLE_length + 1] = 1023;
								RLE_length = RLE_length + 2;
								L = L - 1023;
							}

							RLE[RLE_length] = OP;
							RLE[RLE_length + 1] = L;
							RLE_length = RLE_length + 2;
							L = 0;
						}

						OP = *(rowPtrs[y]+x) + 1024;
					}

					L++;
				}

			}

			memcpy(buffer_previous_row,rowPtrs[y],width*sizeof(char));             // make current row as previous row and update current row
		}

		row_number = row_number + ROWBUFFER;
	}

	//RLE_length++;                //needed just in case
	
	unsigned short *temp_RLE;
	if( (temp_RLE = (unsigned short *) calloc(RLE_length, sizeof(unsigned short))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for temp RLE... Please check memory and try again later..." << endl;
		exit(-1);
	}

	//------------------------LineDiff compaction----------------------------------//
	unsigned int i = 0;
	while (i<RLE_length)                                           // temp RLE for LineDiff compaction
	{
		if (RLE[i] == 1056 && i != 0)
		{
			if(RLE[i-1] == 1057 && RLE[i+1] != 1)
			{
				i++;
				continue;
			}
			else if(RLE[i-1] == 1057 && RLE[i+1] == 1)                     // case where DUP and L=1 occur at the start
			{
				temp_RLE[count_RLE] = RLE[i];
				temp_RLE[count_RLE+1] = RLE[i+1];
				count_RLE += 2;
				i += 2;
				continue;
			}
			else
			{
				temp_RLE[count_RLE] = RLE[i];
				count_RLE++;
				i++;
				continue;
			}
		}

		else if (RLE[i] == 1)
		{
			i++;
			continue;
		}

		else
		{
			temp_RLE[count_RLE] = RLE[i];
			count_RLE++;
			i++;
			continue;
		}
	}
	if(temp_RLE[0] == 1056)
		RLE_length = count_RLE-1;
	else
		RLE_length = count_RLE;

	//unsigned long long bitlength = 0;


	//---------------------------------------------PREFIX coding----------------------------------------------
	free(RLE);
	//bool *bool_RLE;
	if( (bool_RLE = (bool *) calloc(RLE_length*8, sizeof(bool))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for bool RLE... Please check memory and try again later..." << endl;
		exit(-1);
	}


	for (unsigned int j = 1,r=0;j < (RLE_length+1);j++,r++)                 //start from 1 to remove first DUP element
	{
		if(temp_RLE[0] != 1056 && r==0)
		{
			j--;
			RLE_length--;
		}
		//cout<< temp_RLE[j] << '\t';
		if(temp_RLE[j] == 1056)
		{
			bool_RLE[bitlength] = 0;	bitlength++;
			bool_RLE[bitlength] = 0;	bitlength++;
		}
		if(temp_RLE[j] == 1057)
		{
			bool_RLE[bitlength] = 0;	bitlength++;
			bool_RLE[bitlength] = 1;	bitlength++;
		}
		if(temp_RLE[j] == 1024)
		{
			bool_RLE[bitlength] = 1;	bitlength++;
			bool_RLE[bitlength] = 0;	bitlength++;
			bool_RLE[bitlength] = 0;	bitlength++;
			bool_RLE[bitlength] = 1;	bitlength++;
		}
		if(temp_RLE[j] == 1055)
		{
			bool_RLE[bitlength] = 1;	bitlength++;
			bool_RLE[bitlength] = 0;	bitlength++;
			bool_RLE[bitlength] = 0;	bitlength++;
			bool_RLE[bitlength] = 0;	bitlength++;
		}

		if(temp_RLE[j] > 1024 && temp_RLE[j] < 1055)
		{
			bool_RLE[bitlength] = 1;	bitlength++;
			bool_RLE[bitlength] = 0;	bitlength++;
			bool_RLE[bitlength] = 1;	bitlength++;
			bitset<5> bits(temp_RLE[j] - 1024);
			for (i=0;i<5;i++)
			{
				bool_RLE[bitlength] = bits[i]; bitlength++;
			}
		}
		if(temp_RLE[j] > 1 && temp_RLE[j] < 32)
		{
			bool_RLE[bitlength] = 1;	bitlength++;
			bool_RLE[bitlength] = 1;	bitlength++;
			bool_RLE[bitlength] = 0;	bitlength++;
			bitset<5> bits(temp_RLE[j]);
			for (i=0;i<5;i++)
			{
				bool_RLE[bitlength] = bits[i]; bitlength++;
			}
		}
		if(temp_RLE[j] > 31 && temp_RLE[j] < 1024)
		{
			bool_RLE[bitlength] = 1;	bitlength++;
			bool_RLE[bitlength] = 1;	bitlength++;
			bool_RLE[bitlength] = 1;	bitlength++;
			bitset<10> bits(temp_RLE[j]);
			for (i=0;i<10;i++)
			{
				bool_RLE[bitlength] = bits[i]; bitlength++;
			}
		}
	}

	if(temp_RLE[0] != 1056)
		RLE_length++;
	cout << endl << "RLE length : " << RLE_length << endl;
	free(temp_RLE);

	count_RLE = 0;
	unsigned char r = 0;
	if( (RLE = (unsigned short *) calloc((bitlength/8 + 1), sizeof(unsigned short))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for bool RLE... Please check memory and try again later..." << endl;
		exit(-1);
	}
	bitset<16> bits;

	for (unsigned int j = 0; j < bitlength;j++)
	{
		bits[j%16] = bool_RLE[j];
		if(j%16 == 15)
		{
				RLE[count_RLE] = unsigned short(bits.to_ulong());
				count_RLE++;
		}
		if(j == bitlength-1)
		{
			RLE[count_RLE] = unsigned short(bits.to_ulong()); 
			count_RLE++;
		}
		/*if(j != 0)
			if(j%16 == 0)
			{
				RLE[count_RLE] = r; count_RLE++;
				r = 0;
			}
		if(bool_RLE[j])
			r |= 1 << (15 - (j%16)) ;

		if(j == bitlength-1)
		{
			RLE[count_RLE] = r; count_RLE++;
		}*/
	}

	//cout << count_RLE << endl;
	RLE_length = count_RLE;
	free(bool_RLE);


	//if(RLE[0] == 1056)
	//{
	//	for (unsigned int j = 1; j <(RLE_length+1); j++)				//start from 1 to remove first DUP element
	//	{
	//		RLE[j-1] = temp_RLE[j];
	//		//cout << RLE[j-1] << '\t';
	//		if(RLE[j-1] == 1056 || RLE[j-1] == 1057)
	//			bitlength += 2;
	//		if(RLE[j-1] == 1024 || RLE[j-1] == 1055)
	//			bitlength += 4;
	//		if(RLE[j-1] > 1024 && RLE[j-1] < 1055)
	//			bitlength += 8;
	//		if(RLE[j-1] > 1 && RLE[j-1] < 32)
	//			bitlength += 8;
	//		if(RLE[j-1] > 31 && RLE[j-1] < 1024)
	//			bitlength += 13;
	//	}
	//}
	//else
	//{
	//	for (unsigned int j = 0; j <RLE_length; j++)				
	//	{
	//		RLE[j] = temp_RLE[j];
	//		//cout << RLE[j-1] << '\t';
	//		if(RLE[j] == 1056 || RLE[j] == 1057)
	//			bitlength += 2;
	//		if(RLE[j] == 1024 || RLE[j] == 1055)
	//			bitlength += 4;
	//		if(RLE[j] > 1024 && RLE[j] < 1055)
	//			bitlength += 8;
	//		if(RLE[j] > 1 && RLE[j] < 32)
	//			bitlength += 8;
	//		if(RLE[j] > 31 && RLE[j] < 1024)
	//			bitlength += 13;
	//	}
	//}

	
	cout << "Prefix code bitlength  : " << bitlength << endl;
	cout << "Encoded prefix data in bytes : " << (bitlength)/8 << endl;
	//cout << '0' - 48 << endl;
	return bitlength;
}

// Write memory to text
void ClassEncoder::WriteRLE(string filename,int flag)
{
	unsigned int start,end;
	start = clock();
	if (flag)
	{
		filename += ".txt";
		ofstream outTXT(filename);
		char *Data = new char[RLE_length+1];
		Data[RLE_length] = 0;
		
		for (int i=0; i<RLE_length;i++)
		{
			Data[i] = '0' + (RLE[i] - 128);
			//Data[i] = char(RLE[i]);
		}
		
		outTXT << Data;
		outTXT.close();

	}
	else
	{
		filename += ".line";
		ofstream out(filename,ios::binary);
		for(int i=0;i<RLE_length;i++)
		{
			out << RLE[i] << endl;
			//cout<< RLE[i] << '\t';
		}

		out.close();
	}

	end = clock();
	cout << "\t" << "\t" << "               Compressed file writing time = " << ((double) (end-start)) / ((double) 1000) << endl;
}
