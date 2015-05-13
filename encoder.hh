#ifndef _ENCODER_HH_
#define _ENCODER_HH_
#include <iostream>
#include <cmath>
class IntQueue
{
public:
  unsigned int cap;
  unsigned int size;
  unsigned int start;
  unsigned int end;
  int*q;
  IntQueue(uint capacity)
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
class UCharList
{
public:
  int size;
  int capacity;
  unsigned char*list;
  UCharList(int capcity)
  {
    list = new unsigned char[capacity];
  }
  ~UCharList()
  {
    delete list;
  }
  int find(unsigned char uc)
  {
    int begin,end;
    for(begin=0,end=size-1;begin<=end;)
    {
      //3 elems left
      if(end-begin==0)
      {
        if(list[end]==uc)
	  return end;
	else
	  return -1;
      }
      //2 elems left
      else if(end-begin==1)
      {
	if(list[begin]==uc)
	  return begin;
	else if(list[end]==uc)
	  return end;
	else
	  return -1;
      }
      //if is in bottom half
      else if(uc < list[begin+(end-begin)/2])
      {
	end = begin+(end-begin)/2;
      }
      //if is in top half
      else if(uc > list[begin+(end-begin)/2])
      {
	begin = begin+(end-begin)/2;
      }
      //if found
      else
	return begin+(end-begin)/2;
    }
    return -1;
  }
  int add(unsigned char uc)
  {
    if(size==capacity)
      return -1;
    int i;
    for(i=0;i<size;i++)
    {
      if(list[i]==uc)
	return i;
      else if(list[i]>uc)
	break;
    }
    shiftDown(i);
    list[i]=uc;
    size++;
  }
  unsigned char remove(int index)
  {
    if(index>=size || index <0 || size==0)
      return 0;
    unsigned char ret = list[index];
    shiftUp(index);
    size--;
    return ret;
  }
private:
  //shift down for addition into the array
  void shiftDown(int index)
  {
    if(size==capacity)
      return;
    int i;
    for(i=size;i>index;i++)
    {
      list[i]=list[i-1];
    }
    size++;
  }
  //shift up to delete the elem at index
  void shiftUp(int index)
  {
    if(size==0||index>=size)
      return;
    int i;
    for(i=index;i<size;i++)
    {
      if(i!=capacity-1)
	list[i]=list[i+1];
      else
	list[i]=0;
    }
  }
};

//helper function for determining if coeff is positive, negative, zerotree root, or isolated zero
char isPNTZ(unsigned char**image,int width,int height,int depth, unsigned char threshold,int x, int y)
{
  //if the image is below the threshold, it may be a zerotree
  if((char)image[x][y] < threshold)
  {
    int layer = depth;
    while(x > width>>layer)
      layer++;
    //if this is the highest level, there are three children to check
    if(x < width>>depth && y < height>>depth)
    {
      //if(isPNTZ(image,width,height,depth,threshold, (int)(x/(double)width)))
      
    }
    //if this is the lowest level, there are no children to check
    else if(x > width/2 && y > height/2)
    {
      return 'Z';
    }
    //otherwise there are 4 children to check
    else
    {
    }
  }
  else if((char)image[x][y] >= threshold)
  {
    return 'P';
  }
  else if((char)image[x][y] >= threshold)
  {
    return 'N';
  }
}

//as described in the algorithm
void dominant_pass(unsigned char**image,int origin_x,int origin_y,int width,int height,int depth, unsigned char threshold,int level, unsigned char**coded,UCharList*sub_list)
{
  int i,j;
  //if we are splitting px in half the half goes to 2nd half
  if(level!=depth)
  {
    //actually do the coding
    for(i=0;i<width;i++)
    {
      for(j=0;j<height;j++)
      {
	if(image[i][j]>=threshold)
	{
	  //write 1
	}
        else
	{  
          //write 0
	}
	coded[i][j]=true;
      }
    }
  }
  else
  {
    //recurse on the next level
    dominant_pass(image,
		  origin_x,
		  origin_y,
		  width/2,
		  height/2,
		  depth,threshold,level+1,coded,sub_list);
    //TODO THIS ORDER IS NOT CLEAR
     //recurse also on the LH, HL, and HH but they don't recurse themselves (level set to depth)
    //LH?
    dominant_pass(image,
		  origin_x+width/2+width%2,
		  origin_y,
		  width/2,
		  height/2,
		  depth,threshold,depth,coded,sub_list);
    //HL?
    dominant_pass(image,
		  origin_x,
		  origin_y+height/2+height%2,
		  width/2,height/2,
		  depth,threshold,depth,coded,sub_list);
    //HH
    dominant_pass(image,
		  origin_x+width/2+width%2,
		  origin_y+height/2+height%2,
		  width/2,
		  height/2,
		  depth,threshold,depth,coded,sub_list);
  }
}

void subordinate_pass(unsigned char**image,int origin_x,int origin_y,int width,int height,int depth, unsigned char threshold,int level, unsigned char**coded,UCharList*sub_list)
{
  return;
}

//encode an image of coefficients
//   image:  the actual image to encode
//   width:  image's width
//   height: image's height
//   depth:  how many levels there are
//   min_threshold: the threshold to stop at
//   max: the maximum coefficient in image
//find the max coeff beforehand!
int encode(unsigned char**image,int width,int height,int depth,unsigned char min_threshold, unsigned char max, unsigned char**encode_out,std::ofstream*file_out)
{
  unsigned char threshold = max;
  unsigned char**coded;
  UCharList*sub_list;
  int i;
  sub_list = new UCharList(width*height);
  coded = new unsigned char*[height];
  for(i=0;i<width;i++)
    coded[i]=new unsigned char[width];
  //do a dom and sub pass
  //we're going to be doing this at least once
  do
  {
    dominant_pass(image,0,0,width,height,depth,threshold,0,coded,sub_list);
    subordinate_pass(image,0,0,width,height,depth,threshold,0,coded,sub_list);
    threshold/=2;
  }
  while(threshold>min_threshold);
  
  for(i=0;i<width;i++)
    delete coded[i];
  delete coded;
}
#endif
