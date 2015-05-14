#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
using namespace std;

int main(int argc, char * arg[])

{	
	if (argc == 1){
		std::cout<<"Error: No filename submitted. Please input filename."<<std::endl;
		return 1;
	}

	//for handling the file
	std::string filename = arg[1];
	std::ifstream image_file;
	
	//buffers for reading the header
	unsigned char begin_of_bitmap[4];
	unsigned char width_data[4];
	unsigned char height_data[4];
	unsigned char pixel_bit_data[2];
	unsigned char num_decomps_data;
	
	//secondary buffers for interpretting the header
	int bitmap_offset=0;
	int width =0;
	int height =0;
	int pixel_bits=0;
	int num_decomps=0;
	
	//start reading 10 bytes after the beginning of the file
	image_file.open(filename.c_str(), std::ifstream::in | std::ifstream::binary);
	image_file.seekg(10, std::ifstream::beg);

	//read the first 4 bytes of the file which are interpretted for the header size
	image_file.read(reinterpret_cast<char*>(begin_of_bitmap),4);
	//B0*2^0 + B1*2^8 + B2*2^16 + B3*2^24
	for (int i=0; i<4; i++){
		bitmap_offset+=(int)begin_of_bitmap[i]*pow(2,i*8);
	//	std::cout<<(int)begin_of_bitmap[i]<<" ";
	}
	//std::cout<<std::endl;

	//std::cout<<bitmap_offset<<std::endl;

	//start reading 18 bytes after the beginning of the file
	image_file.seekg(18, std::ifstream::beg);
	//read 4 bytes as the width
	image_file.read(reinterpret_cast<char*>(width_data),4);
	for(int i=0; i<4; i++){
		width+=(int)width_data[i]*pow(2,i*8);
	}
	//read 4 bytes as the height
	image_file.read(reinterpret_cast<char*>(height_data),4);
	for(int i=0; i<4; i++){
		height+=(int)height_data[i]*pow(2,i*8);
	}

	//std::cout<<width<<" "<<height<<std::endl;

	//start reading 28 bytes after the beginning of the file
	image_file.seekg(28, std::ifstream::beg);
	
	//read 2 bytes of data as the number of bits per pixel
	image_file.read(reinterpret_cast<char*>(pixel_bit_data),2);
	for(int i=0; i<2; i++){
		pixel_bits+=(int)pixel_bit_data[i]*pow(2,i*8);
	}

	//we are restricting the program to 24-bit bitmaps
	if(pixel_bits != 24){
		std::cout<<"Image is not a 24-bit bitmap file. Please submit a properly formatted file."<<std::endl;
		return 1;
	}
	
	//holds all the of the header information
	unsigned char* image_header_data= new unsigned char[bitmap_offset];
	//holds the raw input image data BUT also the raw input image output
	unsigned char** imageData=new unsigned char*[height];
	//holds the image data separated into colors
	unsigned char** blue_data=new unsigned char*[height];
	unsigned char** green_data=new unsigned char*[height];
	unsigned char** red_data=new unsigned char*[height];

	//further initialize 2D arrays
	for (int i = 0; i<height; i++){
		imageData[i] = new unsigned char[width*3];
		blue_data[i] = new unsigned char[width];
		green_data[i] = new unsigned char[width];
		red_data[i] = new unsigned char[width];
	}	

	//read the header information(again?) into image_header_data
	image_file.seekg(0, std::ifstream::beg);
	image_file.read(reinterpret_cast<char*>(image_header_data),bitmap_offset);

	//Read in the number of Decompisition Loops:
	image_file.read(reinterpret_cast<char*>(num_decomps_data),1);
	num_decomps=(int)num_decomps_data;

	//read in the raw image data
	for (int i=0; i<height; i++){
		image_file.read(reinterpret_cast<char*>(blue_data[i]),width);
	}
	for (int i=0; i<height; i++){
		image_file.read(reinterpret_cast<char*>(green_data[i]),width);
	}
	for (int i=0; i<height; i++){
		image_file.read(reinterpret_cast<char*>(red_data[i]),width);
	}

//	for (int i=0; i<height; i++){
//		std::cout<<imageData[i]<<" ";
//	}
//	std::cout<<std::endl;

/*	for (int i=0; i<10; i++){
		std::cout<<"[ ";
		for(int j = 0; j<10; j++){
			std::cout<<(int)imageData[i][j]<<" ";
		}
		std::cout<<"]"<<std::endl;
	}*/
	
	//don't need to read the image anymore
	image_file.close();

/*
 *At this point, the image is properly loaded into several char arrays. They can be 
 numerically manipulated like normal numbers (each individual array entry)
 The arrays are:

 blue_data <- all the blue pixel values.
 green_data <- all the green pixel values.
 red_data <- all the red pixel values.

 height <- original image height.
 width <- original image width.
 
 image_header_data <- the original image header
 *
 *
 *                            Subband Recomposition
 *========================================================================
 */
	int recomp_height= height;
	int recomp_width;


//	reconstruct((imageData),height,width*3,image_header_data,bitmap_offset,filename);


/*
 *	So now imageData has the Subband Decomposition of the oringal image (decomposed twice).
 *	the compression portion hasn't happened yet, that involves choosing the threshold and such.
 *
 *
 *
 *
 *
 *=================================================================================
 */
	//clean up memory
	delete image_header_data;
	for(int i=0;i<height;i++)
	{
	  delete imageData[i];
	  delete blue_data[i];
	  delete green_data[i];
	  delete red_data[i];  
	}
	delete imageData;
	delete blue_data;
	delete green_data;
	delete red_data;
   return 0;
}
