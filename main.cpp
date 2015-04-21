#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>

int main(int argc, char * arg[])

{	
	if (argc == 1){
		std::cout<<"Error: No filename submitted. Please input filename."<<std::endl;
		return 1;
	}

	std::string filename = arg[1];
	std::ifstream image_file;
	unsigned char begin_of_bitmap[4];
	unsigned char width_data[4];
	unsigned char height_data[4];
	unsigned char pixel_bit_data[2];
	int bitmap_offset=0;
	int width =0;
	int height =0;
	int pixel_bits=0;
	
	image_file.open(filename.c_str(), std::ifstream::in | std::ifstream::binary);
	image_file.seekg(10, std::ifstream::beg);

	image_file.read(reinterpret_cast<char*>(begin_of_bitmap),4);

	for (int i=0; i<4; i++){
		bitmap_offset+=(int)begin_of_bitmap[i]*pow(2,i*8);
	//	std::cout<<(int)begin_of_bitmap[i]<<" ";
	}
	//std::cout<<std::endl;

	//std::cout<<bitmap_offset<<std::endl;

	
	image_file.seekg(18, std::ifstream::beg);
	image_file.read(reinterpret_cast<char*>(width_data),4);

	for(int i=0; i<4; i++){
		width+=(int)width_data[i]*pow(2,i*8);
	}

	image_file.read(reinterpret_cast<char*>(height_data),4);
	
	for(int i=0; i<4; i++){
		height+=(int)height_data[i]*pow(2,i*8);
	}

	//std::cout<<width<<" "<<height<<std::endl;

	image_file.seekg(28, std::ifstream::beg);
	image_file.read(reinterpret_cast<char*>(pixel_bit_data),2);

	for(int i=0; i<2; i++){
		pixel_bits+=(int)pixel_bit_data[i]*pow(2,i*8);
	}

	if(pixel_bits != 24){
		std::cout<<"Image is not a 24-bit bitmap file. Please submit a properly formatted file."<<std::endl;
		return 1;
	}
	unsigned char image_header_data[bitmap_offset];
	unsigned char imageData[height][width*3];
	unsigned char blue_data[height][width];
	unsigned char green_data[height][width];
	unsigned char red_data[height][width];

	image_file.seekg(0, std::ifstream::beg);
	image_file.read(reinterpret_cast<char*>(image_header_data),bitmap_offset);

	for (int i=0; i<height; i++){
		image_file.read(reinterpret_cast<char*>(imageData[i]),width);
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

	for (int i = 0; i<height; i++){
		for(int j = 0; j<width*3; j+=3){
			blue_data[i][j/3] = imageData[i][j];
		}
		for(int j = 1; j<width*3; j+=3){
			green_data[i][j/3] = imageData[i][j];
		}
		for(int j=2;j<width*3; j+=3){
			red_data[i][j/3] = imageData[i][j];
		}
	}
	

	image_file.close();

/*
 *At this point, the image is properly loaded into several char arrays. They can be 
 numerically manipulated like noraml numbers (each individual array entry)
 The arrats are:

 imageData <- all the pixels of the image.
 blue_data <- all the blue pixel values.
 green_data <- all the green pixel values.
 red_data <- all the red pixel values.

 height <- original image height.
 width <- original image width.
 
 image_header_data <- the original image header
 *
 *
 *
 *========================================================================
 */


   return 1;
}

