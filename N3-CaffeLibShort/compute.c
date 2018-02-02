#include<stdio.h>
#include <cv.h>
#include <highgui.h>
//#include "intweightfloatbias.h"
//#include "shortweightfloatbias.h"
//#include "charweightfloatbias.h"
#include "shortlenetparameters.h"

#define SHIFTFIVE 5

/*casting char *imageData in IplImage* img to float data[]*/
//float *castCharToFloat(float *data, IplImage* argimg){
void castCharToFloat(float *data, short *sdata, IplImage* argimg){
	int i,j;
	
	for(i=0; i<argimg->height; i++){
		for(j=0; j<argimg->width; j++){
			data[i*argimg->width+j] = (float) (argimg->imageData[i*argimg->width+j]&0x000000ff)/255;//since argimg->imageData[] is char type, &0x000000ff is needed when cast it to short
			sdata[i*argimg->width+j] = (short) (data[i*argimg->width+j] * (1 << SHIFTFIVE));
		}
	}
	
	/*print out the digitalized image*/
	printf("\n---------------------------------printing out the digit as shown in the window---------------------------------\n\n");
	for(i=0; i<argimg->imageSize; i++){
		printf("%2d ", sdata[i]);
		if((i+1)%argimg->height==0){
			printf("\n");
		}
	}
	/*
	for(i=0; i<argimg->imageSize; i++){
		printf("%1.2f ", data[i]);
		if((i+1)%argimg->height==0){
			printf("\n");
		}
	}*/
}



void main(int argc, char *argv[]){
	int conv1_out_size;
	int pool1_out_size;
	int conv2_out_size;
	int pool2_out_size;

	short *ip1_output;
	short *ip2_output;
	short *conv1_output;
	short *pool1_output;
	short *conv2_output;
	short *pool2_output;

	
	conv1_output = (short *)malloc((1*20*24*24)* sizeof(short));
	pool1_output = (short *)malloc((20*12*12)* sizeof(short));
	conv2_output = (short *)malloc((20*50*8*8)* sizeof(short));
	pool2_output = (short *)malloc((50*4*4)* sizeof(short));
	ip1_output = (short *)malloc(500 * sizeof(short));
	ip2_output = (short *)malloc(10 * sizeof(short));
	

	if(argc < 2){
		printf("error: input image filename please\n");
	}
	
	
	IplImage* img = cvLoadImage(argv[argc - 1], CV_LOAD_IMAGE_GRAYSCALE);  //images' path: eg. "../images/4.bmp" 
	float data[img->imageSize];  //place to store the casted img->imageData[]
	short sdata[img->imageSize];
	
	/*show image in window*/
	cvNamedWindow("MNIST_DIGIT", 10);
	cvShowImage("MNIST_DIGIT", img);

	castCharToFloat(data,sdata,img);

	conv1_out_size = 0;
	pool1_out_size = 0;
	conv2_out_size = 0;
	pool2_out_size = 0;
	
	printf("\n---------------------------------------------predicting the digit----------------------------------------------\n");

	conv1_out_size = convolution(&sdata, conv1_output, conv1_weight, conv1_bias, img->height, 1, 20, 5, 1);
	pool1_out_size = pooling(conv1_output, pool1_output, conv1_out_size, 20, 2, 2);
	conv2_out_size = convolution(pool1_output, conv2_output, conv2_weight, conv2_bias, pool1_out_size, 20, 50, 5, 1);
	pool2_out_size = pooling(conv2_output, pool2_output, conv2_out_size, 50, 2, 2);
	innerproduct(pool2_output, ip1_output, ip1_weight, ip1_bias, 800, 500);
	relu(ip1_output, 500);
	innerproduct(ip1_output, ip2_output, ip2_weight, ip2_bias, 500, 10);
	softmax(ip2_output,10);
	
	printf("-----------------------------------------------recognition ended-----------------------------------------------\n");	
	
	/*waiting the image window to be closed*/
	cvWaitKey(0);
	cvReleaseImage(&img);
	cvDestroyWindow("MNIST_DIGIT");
	//free() function caused segmentation fault (core dumped)
	free(conv1_output);
	free(pool1_output);
	free(conv2_output);
	free(pool2_output);
	free(ip1_output);
	free(ip2_output);	
}
