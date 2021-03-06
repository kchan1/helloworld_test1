#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
using namespace std;

void horizontal_convolution(double* &image, int image_length,  double* filter, int filter_length){
	double copy[image_length];
	double pass_value;
	for (int i=0; i<image_length;i++){
		copy[i] = image[i];
	}
	int offset = filter_length/2;

	for (int i = offset; i<image_length+offset; i++){
		pass_value = 0;		
		for (int j = 0; j<filter_length; j++){
			if (i-j < 0  || i-j >= image_length){
				continue;
			}
			pass_value +=filter[j]*copy[i-j];
		}
		image[i-offset]=pass_value;
	}

	return;
}

void vertical_convolution(double** &image, int height, int width,  double* filter, int filter_length){
	double copy[height][width];
	double pass_value;
	for (int i=0; i<height ;i++){
		for (int j = 0; j<width ; j++){
			copy[i][j] = image[i][j];
		}
	}
	int offset = filter_length/2;

	for (int i=0; i<width; i++){
		for(int j = offset ; j<height+offset; j++){
			pass_value=0;
			for (int k =0; k<filter_length; k++){
				if(j-k < 0  || j-k >= height){
					continue;
				}
				pass_value+=filter[k]*copy[j-k][i];
			}
			image[j-offset][i]=pass_value;
		}
	}


	return;
}



void subband_recomp(double** &image_data, int height, int width){
	double low_pass_filter[7] = {.2,.6,.7,.2,-.3,0,.1};
	double high_pass_filter[7] = {-.1,0,.3,.2,-.7,.6,-.2};

	double** round1_low = new double*[height];
	double** round1_high = new double*[height];
	double** round2_low = new double*[height];
	double** round2_high = new double*[height];

	for (int i = 0; i<height; i++){
		round1_low[i] = new double[width];
		round1_high[i] = new double[width];
		round2_low[i] = new double[width];
		round2_high[i] = new double[width];
	}

	for (int i=0; i <height; i++){
		for(int j=0; j<width; j++){
			round1_low[i][j] = 0;
			round1_high[i][j] = 0;
			round2_low[i][j] = 0;
			round2_high[i][j] = 0;
		}		
	}

	//cout<<height/2<<" "<<width/2<<endl;

	for (int i=0; i<(int)(height/2); i++){
		for(int j=0; j<(int)(width/2);j++){
			round1_low[i*2][j*2]=(double)image_data[i][j];
			round2_low[i*2][j*2]=(double)image_data[i+height/2][j];
			round1_high[i*2][j*2]=(double)image_data[i+height/2][j+width/2];
			round2_high[i*2][j*2]=(double)image_data[i][j+width/2];
		}
	}

	/*for (int i = 0; i<height; i++){
		for(int j =0; j<width;j++){
			cout<<(int)round1_low[i][j]<<" ";
		}
		cout<<endl;
	}*/

	vertical_convolution(round1_low,height,width,low_pass_filter,7);
	vertical_convolution(round2_low,height,width,high_pass_filter,7);
	vertical_convolution(round1_high,height,width,high_pass_filter,7);
	vertical_convolution(round2_high,height,width,low_pass_filter,7);

	//COMBINATION SECTION GO!

	for (int i=0; i<height; i++){
		for(int j=0; j<width; j++){
			round1_low[i][j] = round1_low[i][j]+round2_low[i][j];
			//round1_low[i][j] = round2_low[i][j];
			round1_high[i][j] = round1_high[i][j]+round2_high[i][j];
			//round1_high[i][j] = round2_high[i][j];
		}
	}	

	for(int i=0; i<height; i++){
		horizontal_convolution(round1_low[i],width,low_pass_filter,7);
		horizontal_convolution(round1_high[i],width,high_pass_filter,7);
	}

	for (int i=0; i<height; i++){
		for(int j=0; j<width; j++){
			image_data[i][j] = round1_low[i][j]+round1_high[i][j];
			image_data[i][j]*=.80;
			//image_data[i][j];
		}
	}

}

//creates an image withe name filename that has the same header information
void reconstruct(unsigned char** image_data, int height, int width, 
	       	unsigned char* image_header_data, int bitmap_offset,
		std::string filename){

	filename+=".bmp";
	std::ofstream output_file;

	output_file.open(filename.c_str(), std::ofstream::out | std::ofstream::binary);
	output_file.write(reinterpret_cast<const char*>(image_header_data), bitmap_offset);
	

	for (int i = 0; i< height; i++){
		output_file.write(reinterpret_cast<const char*>(image_data[i]),width);
	}
	output_file.close();
	return;
}

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
	int image_width = 0;
	int height =0;
	int pixel_bits=0;
	int num_decomps=0;
	char char_reader[1];
	unsigned char uchar_reader[1];
	int total_char;
	int limit_char;
	int quant = 20;
	
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
	width = (int)((3*8*width+31)/(32))*4;
	image_width = width;
	width /= 3;

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
	double** image_data = new double*[height];
	//holds the image data separated into colors
	double** blue_data=new double*[height];
	double** green_data=new double*[height];
	double** red_data=new double*[height];

	//further initialize 2D arrays
	for (int i = 0; i<height; i++){
		imageData[i] = new unsigned char[image_width];
		image_data[i] = new double[image_width];
		blue_data[i] = new double[width];
		green_data[i] = new double[width];
		red_data[i] = new double[width];
	}	

	//read the header information(again?) into image_header_data
	image_file.seekg(0, std::ifstream::beg);
	image_file.read(reinterpret_cast<char*>(image_header_data),bitmap_offset);

	//Read in the number of Decompisition Loops:
	image_file.read(reinterpret_cast<char*>(uchar_reader),1);
	num_decomps=(int)uchar_reader[0];
	//cout<<num_decomps<<endl;
	
	image_file.read(reinterpret_cast<char*>(uchar_reader),1);
	quant=(int)uchar_reader[0];
	//cout<<num_decomps<<endl;

	//image_file.seekg(-2, std::ifstream::cur);
	total_char = 0;
	limit_char = width*height;
	int tempi = 0;
	int tempj = 0;
	//int cout_count = 0;

	while(tempi!=height){
		image_file.read(reinterpret_cast<char*>(uchar_reader),1);
		image_file.read(reinterpret_cast<char*>(char_reader),1);
		//if(cout_count <49927){
		//cout<<(int)uchar_reader[0]<<" "<<(int)char_reader[0]<<endl;
		//cout_count++;
		//cout<<cout_count<<endl;
		//}
		for(int k = 0; k<(int)uchar_reader[0]; k++){
			//cout<<"Trying to read..."<<endl;
			blue_data[tempi][tempj] = char_reader[0]*quant;
			total_char++;
			tempj++;
			if(tempj == width){
				tempj = 0;
				tempi++;
				if(tempi == height){
					break;
				}	
			}
			//cout<<tempi<<" "<<tempj<<endl;
		}
				
	}	
	//image_file.seekg(-2, std::ifstream::cur);
	total_char = 0;
	limit_char = width*height;
	tempi = 0;
	tempj = 0;

	while(tempi!=height){
		image_file.read(reinterpret_cast<char*>(uchar_reader),1);
		image_file.read(reinterpret_cast<char*>(char_reader),1);
		//cout<<(int)uchar_reader[0]<<" "<<(int)char_reader[0]<<endl;
		for(int k = 0; k<(int)uchar_reader[0]; k++){
			//cout<<"Trying to read..."<<endl;
			green_data[tempi][tempj] = char_reader[0]*quant;
			total_char++;
			tempj++;
			if(tempj == width){
				tempj = 0;
				tempi++;
				if(tempi == height){
					break;
				}	
			}
			//cout<<tempi<<" "<<tempj<<endl;
		}
				
	}	

	//image_file.seekg(-2, std::ifstream::cur);
	total_char = 0;
	limit_char = width*height;
	tempi = 0;
	tempj = 0;

	while(tempi!=height){
		image_file.read(reinterpret_cast<char*>(uchar_reader),1);
		image_file.read(reinterpret_cast<char*>(char_reader),1);
		//cout<<(int)uchar_reader[0]<<" "<<(int)char_reader[0]<<endl;
		for(int k = 0; k<(int)uchar_reader[0]; k++){
			//cout<<"Trying to read..."<<endl;
			red_data[tempi][tempj] = char_reader[0]*quant;
			//red_data[tempi][tempj] = 0;			
			total_char++;
			tempj++;
			if(tempj == width){
				tempj = 0;
				tempi++;
				if(tempi == height){
					break;
				}	
			}
			//cout<<tempi<<" "<<tempj<<endl;
		}
				
	}	

	//for (int i = 0; i<20; i++){
	//	cout<<red_data[2][i]<<endl;
	//}	

	//read in the raw image data
	//Read in the data from the zero-tree things.
	//reconstruct the zero tree into a matrix of values.

	


//read in the raw image data
	//for (int i=0; i<height; i++){
//image_file.read(reinterpret_cast<char*>(imageData[i]),width*3);
	//}

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

	/*for(int i = 0; i<height; i++){
		for(int j = 0; j<width*3; j++){
			image_data[i][j]=imageData[i][j];
		}
	}

	//separate imageData into the 3 colors
	for (int i = 0; i<height; i++){
		for(int j = 0; j<width*3; j+=3){
			blue_data[i][j/3] = image_data[i][j];
		}
		for(int j = 1; j<width*3; j+=3){
			green_data[i][j/3] = image_data[i][j];
		}
		for(int j=2;j<width*3; j+=3){
			red_data[i][j/3] = image_data[i][j];
		}
	}*/

	for (int i = 0; i<height; i++){
		for(int j = 0; j<width; j++){
			imageData[i][j]=(unsigned char)0;
		}
	}


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
	int recomp_width = width;

	for (int i = num_decomps; i>=1; i--){
		recomp_height = height/pow(2,i-1);
		recomp_width = width/pow(2,i-1);
		subband_recomp(blue_data,recomp_height,recomp_width);
		subband_recomp(green_data,recomp_height,recomp_width);
		subband_recomp(red_data,recomp_height,recomp_width);
	}

	for (int i = 0; i<height; i++){
		for(int j = 0; j<image_width; j+=3){
			image_data[i][j]=blue_data[i][j/3];
		}
		for(int j = 1; j<image_width; j+=3){
			image_data[i][j]=green_data[i][j/3];
		}
		for(int j=2;j<image_width; j+=3){
			image_data[i][j]=red_data[i][j/3];
		}
	}

	for (int i=0; i<height; i++){
		for(int j=0; j<image_width; j++){
			if (image_data[i][j]<0){
				image_data[i][j]=0;
			}
			if (image_data[i][j]>255){
				image_data[i][j]=255;
			}
		}
	}

	for(int i = 0; i<height; i++){
		for(int j = 0; j<image_width; j++){
			imageData[i][j]=image_data[i][j];
		}
	}
	
	reconstruct(imageData,height,image_width,image_header_data,bitmap_offset,filename);


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
