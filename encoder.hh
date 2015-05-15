#ifndef _ENCODER_HH_
#define _ENCODER_HH_
#include <iostream>
#include <cmath>
class BitWriter
{
public:
  BitWriter(std::ofstream*towrite)
  {
    out=towrite;
    buffer = 0;
    bitnum = 0;
  }
  void write(unsigned char bit)
  {
    //add the new bit to the buffer
    buffer = buffer | (bit&1)<<(7-bitnum++);
    //we only write to the stream when 8 bits have been written
    if(bitnum==8)
    {
      *out<<buffer;
      bitnum=0;
    }
  }
  //finish writing the buffer if it is half done;if it is already done, don't!
  void endWrite()
  {
    if(bitnum>0)
      *out<<buffer;
  }
private:
  std::ofstream*out;
  unsigned char buffer;
  int bitnum;
};

class IntQueue
{
public:
  unsigned int cap;
  unsigned int size;
  unsigned int start;
  unsigned int end;
  int*q;
  IntQueue(unsigned int capacity)
  {
    q = (int*)(calloc(capacity,sizeof(int)));
    cap = capacity;
    size = 0;
    start = 0;
    end = 0;
  }
  ~IntQueue()
  {
    free(q);
  }
  bool NQ(int i)
  {
    if( size != cap )
    {
      size++;
      //std::cout<<"\tNQ:" << i <<"|" << size << "/" << cap <<  "\n";
      q[end] = i;
      end = (end+1)%cap;
      return true;
    }
    return false;
  }
  int DQ()
  {
    if(size != 0)
    {
      int ret;
      size--;
      ret = q[start];
      start = (start+1)%cap;
      //std::cout<<"\tDQ:" << ret <<"|" << size<< "/" << cap << "\n";
      return ret;
    }
    return -1;
  }
};
//helpder function for marking entire trees as zerotrees
void markTree(int width, int height, int depth,int x,int y,unsigned char**coded)
{
  //quickly determind what layer we are at
  int layer = depth;
  while(x > width>>layer)
    layer++;
  //first mark this node
  coded[x][y]='T';
  //if this is the deepest level, there are three children to mark
  if(x < width>>depth && y < height>>depth)
  {
    markTree(width,height,depth,
	     x+(width>>depth),y,
	     coded);
    markTree(width,height,depth,
	     x,y+(height>>depth),
	     coded);
    markTree(width,height,depth,
	     x+(width>>depth),y+(height>>depth),
	     coded);
  }
  //if this is the lowest level, there are no children to mark
  else if(x > width/2 && y > height/2)
  {
    return;
  }
  //otherwise there are 4 children to mark
  else
  {
    markTree(width,height,depth,
	     x*2,y*2,
	     coded);
    markTree(width,height,depth,
	     x*2+1,y*2,
	     coded);
    markTree(width,height,depth,
	     x*2,y*2+1,
	     coded);
    markTree(width,height,depth,
	     x*2+1,y*2+1,
	     coded);    
  }
}
//helper function for determining if coeff is positive, negative, zerotree root, or isolated zero
char isPNTZ(double**image,int width,int height,int depth, unsigned char threshold,int x, int y)
{
  //if the image is below the threshold, it may be a zerotree
  if(image[x][y] < threshold)
  {
    int layer = depth;
    while(x > width>>layer)
      layer++;
    //if this is the deepest level, there are three children to check
    if(x < width>>depth && y < height>>depth)
    {
      if(isPNTZ(image,width,height,depth,threshold, 
        x+(width>>depth),y)=='T' &&
	isPNTZ(image,width,height,depth,threshold, 
        x,y+(height>>depth))=='T' &&
	isPNTZ(image,width,height,depth,threshold, 
        x+(width>>depth),y+(height>>depth))=='T')
	return 'T';
      else
	return 'Z';
    }
    //if this is the lowest level, there are no children to check
    else if(x > width/2 && y > height/2)
    {
      return 'Z';
    }
    //otherwise there are 4 children to check
    else
    {
      if(isPNTZ(image,width,height,depth,threshold, 
	x*2,y*2)=='T' &&
	isPNTZ(image,width,height,depth,threshold, 
	x*2+1,y*2)=='T' &&
	isPNTZ(image,width,height,depth,threshold, 
        x*2,y*2+1)=='T' &&
	isPNTZ(image,width,height,depth,threshold, 
        x*2+1,y*2+1)=='T')
	return 'T';
      else
	return 'Z';
    }
  }
  else if(image[x][y] >= threshold)
    return 'P';
  else if(image[x][y] >= threshold)
    return 'N';
  else
    return '?';
}

//as described in the algorithm
void dominant_pass(double**image,int orgwidth,int orgheight,int depth, int newwidth,int newheight,int origin_x,int origin_y,int threshold,int level, unsigned char**coded,IntQueue*sub_list,double**recon_coeff,BitWriter*bitstream)
{
  int i,j;
  char read;
  //if we are not at the roots of the zerotree forest
  if(level!=depth)
  {
    //actually do the coding
    for(i=0;i<newwidth;i++)
    {
      for(j=0;j<newheight;j++)
      {
	//for coded, T is part of a zerotree, C is coded, U is uncoded
	//if the coefficient is either coded already or is part of a zerotree, skip
	if(coded[i][j]=='T'||coded[i][j]=='C')
	  continue;
	//get the state of this specific coefficient
	read = isPNTZ(image,orgwidth,orgheight,depth,threshold,i,j);
	//11 positive
	//10 negative
	//01 zerotree root
	//00 isolated zero
	//0 level 0 zero
	switch(read)
	{
	case 'P':
	  bitstream->write(1);
	  bitstream->write(1);
	  coded[i][j]='C';
	  sub_list->NQ((i&0xFFFF)<<16 | (j&0xFFFF));
	  recon_coeff[i][j]=threshold+threshold/2;
	  break;
	case 'N':
	  bitstream->write(1);
	  bitstream->write(0);
	  coded[i][j]='C';
	  sub_list->NQ((i&0xFFFF)<<16 | (j&0xFFFF));
	  recon_coeff[i][j]=threshold+threshold/2;
	  break;
	case 'T':
	  bitstream->write(0);
	  bitstream->write(1);
	  markTree(orgwidth,orgheight,depth,i,j,coded);
	  break;
	case 'Z':
	  bitstream->write(0);
	  //only write that second zero if we aren't on the lowest level
	  if(i>orgwidth/2 || j>orgheight/2)
	    bitstream->write(0);
	  coded[i][j] = 'Z';
	  break;
	default:
	  std::cout<<"ERROR! UNKNOWN COEFFICIENT STATE ("<<i<<","<<j<<")\n";
	  break;
	}
      }
    }
  }
  else
  {
    //recurse on the next level
    dominant_pass(image,orgwidth,orgheight,depth,
		  newwidth/2,
		  newheight/2,
		  origin_x,
		  origin_y,
		  threshold,level+1,coded,sub_list,recon_coeff,bitstream);
    //TODO THIS ORDER IS NOT CLEAR?! I just do the +width first
    //recurse also on the LH, HL, and HH but they don't recurse themselves
    //LH?
    dominant_pass(image,orgwidth,orgheight,depth,
		  newwidth/2,
		  newheight/2,
		  origin_x+newwidth/2,
		  origin_y,
		  threshold,depth,coded,sub_list,recon_coeff,bitstream);
    //HL?
    dominant_pass(image,orgwidth,orgheight,depth,
		  newwidth/2,
		  newheight/2,
		  origin_x,
		  origin_y+newheight/2,
		  threshold,depth,coded,sub_list,recon_coeff,bitstream);
    //HH
    dominant_pass(image,orgwidth,orgheight,depth,
		  newwidth/2,
		  newheight/2,
		  origin_x+newwidth/2,
		  origin_y+newheight/2,
		  threshold,depth,coded,sub_list,recon_coeff,bitstream);
  }//end if level
}
void subordinate_pass(double**image,int threshold,IntQueue*sub_list,double**recon_coeff,BitWriter*bitstream)
{
  int coord,x,y;
  //empty out the sub_list
  while(sub_list->size>0)
  {
    //separate x and y from the compact form used in the list
    coord = sub_list->DQ();
    x=coord>>16&0xFFFF;
    y=coord&0xFFFF;
    //as in the algorithm
    if(image[x][y]>recon_coeff[x][y])
    {
      bitstream->write(1);
      recon_coeff[x][y]+=threshold/2;
    }
    else
    {
      bitstream->write(0);
      recon_coeff[x][y]-=threshold/2;
    }
  }
}

//encode an image of coefficients
//   image:  the actual image to encode
//   width:  image's width
//   height: image's height
//   depth:  how many levels there are
//   min_threshold: the threshold to stop at
//   max: the maximum coefficient in image
//find the max coeff beforehand!
int encode(double**image,int width,int height,int depth,double min_threshold,double max,std::ofstream*file_out)
{
  int threshold = max;
  //this holds the coding states of all the coefficients
  unsigned char**coded;
  //this colds all the reconstructed coefficients
  double**recon_coeff;
  //this is used to keep track of which coefficients need a subordinate pass
  IntQueue*sub_list;
  //for writing individual bits
  BitWriter*bitstream;
  int i,j;
  //initialization
  sub_list = new IntQueue(width*height);
  coded = new unsigned char*[height];
  recon_coeff = new double*[height];
  bitstream = new BitWriter(file_out);
  for(i=0;i<height;i++)
  {
    coded[i]=new unsigned char[width];
    recon_coeff[i]= new double[width];
    for(j=0;j<width;j++)
    {
      coded[i][j]='U';
    }
  }
  //do a dom and sub pass
  //we're going to be doing this at least once
  do
  {
    dominant_pass(image,width,height,depth,width,height,0,0,threshold,0,coded,sub_list,recon_coeff,bitstream);
    subordinate_pass(image,threshold,sub_list,recon_coeff,bitstream);
    threshold/=2;
  }
  while(threshold>min_threshold);
  for(i=0;i<width;i++)
  {
    delete coded[i];
    delete recon_coeff[i];
  }
  delete coded;
  delete recon_coeff;
  delete sub_list;
  delete bitstream;
}
#endif
