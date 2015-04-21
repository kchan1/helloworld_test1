#include <iostream>
#include <fstream>
#include <stdlib.h>

int main(int argc, char * arg[])

{	
	if (argc == 1){
		std::cout<<"Error: No filename submitted. Please input filename."<<std::endl;
		return 1;
	}
	else if (argc == 2){
		std::cout<<"Error: Image dimensions were not provided. Please provide image dimensions."
			<<std::endl;
		return 1;
	}

	std::string filename = arg[1];
	int HEIGHT = atoi(arg[2]);
	int WIDTH = atoi(arg[3]);
	std::ifstream* image_file;
	unsigned char imageHeaderData[1078];
	unsigned char** imageData;
	unsigned char** blue_data;
	unsigned char** green_data;
	unsigned char** red_data;

	imageData = new unsigned char* [HEIGHT];
	blue_data = new unsigned char* [HEIGHT];
	green_data = new unsigned char* [HEIGHT];
	red_data = new unsigned char* [HEIGHT];

	for (int i = 0 ; i< HEIGHT; i++){
		imageData[i] = new unsigned char [WIDTH*3];
		blue_data[i] = new unsigned char [WIDTH];
		green_data[i] = new unsigned char [WIDTH];
		red_data[i] = new unsigned char [WIDTH];
	}
	
	image_file = new std::ifstream;
	image_file->open(filename.c_str(), std::ifstream::in | std::ifstream::binary);
	image_file->seekg(0, std::ifstream::beg);

	//For now I am just skipping the header, we can techincally use this to get the image size
	//If someone wants to figure that out, then a user wont have to know their file dimensions.
	image_file->read(reinterpret_cast<char*>(imageHeaderData),1078);

	for (int i=0; i<HEIGHT; i++){
		image_file->read(reinterpret_cast<char*>(imageData[i]),WIDTH);
	}

//	for (int i=0; i<HEIGHT; i++){
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

	for (int i = 0; i<HEIGHT; i++){
		for(int j = 0; j<WIDTH*3; j+=3){
			blue_data[i][j/3] = imageData[i][j];
		}
		for(int j = 1; j<WIDTH*3; j+=3){
			green_data[i][j/3] = imageData[i][j];
		}
		for(int j=2;j<WIDTH*3; j+=3){
			red_data[i][j/3] = imageData[i][j];
		}
	}

	image_file->close();


   return 1;
}

