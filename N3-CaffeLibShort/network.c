#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SHIFTFIVE 5
#define SHIFTTEN 10

int convolution(short *bottom, short *top, short *weight, short *bias, int img_size, int bottom_num, int top_num, int kernel, int stride){ //top_num is kernel_num, input_num is batch
	/*input data, output data, kernel and stride have the same heights and width*/
	int input_height = 0;
	int input_width = 0;
	int output_height = 0;
	int output_width = 0;
	int kernel_height = 0;
	int kernel_width = 0;
	int stride_height = 0;
	int stride_width = 0;
	int sum_per_kernel;
	int sum_all_kernel;
	
	int iIN; //iterator of image number
	int iKN; //iterator of kernel number
	int iOH; //iterator of output height
	int iOW; //iterator of output width
	int iKH;  //iterator of kernel height
	int iKW;  //iterator of kernel width


	input_height = img_size;
	input_width = input_height;
	
	output_height = img_size/stride - kernel + stride; //24=28/1-5+1
	output_width = output_height;

	kernel_height = kernel;
	kernel_width = kernel_height;

	stride_height = stride;
	stride_width = stride_height;
	
	
	int i;

	for(i=0; i<bottom_num*top_num*kernel*kernel; i++){
		if((weight[i] > 127) || (weight[i] < -128)){
			printf("conv weight overflow\n");
		}
	}

	
	for(iKN=0; iKN<top_num; iKN++){ //top_num
			for(iOH=0; iOH<output_height; iOH++){
				for(iOW=0; iOW<output_width; iOW++){
					sum_all_kernel = 0.0;
					for(iIN=0; iIN<bottom_num; iIN++){
						sum_per_kernel = 0;
						for(iKH=0; iKH<kernel_height; iKH++){
							for(iKW=0; iKW<kernel_width; iKW++){
								sum_per_kernel += bottom[iKW + iKH*input_width + iOW*stride_width+ iOH*input_height + iIN*input_height*input_width] * weight[iKW + iKH*kernel_width + iIN*kernel_height*kernel_width + iKN*bottom_num*kernel_height*kernel_width];
							}
						}
						sum_all_kernel += sum_per_kernel;
					}
					top[iOW + iOH*output_width + iKN*output_height*output_width] = sum_all_kernel + bias[iKN];
					top[iOW + iOH*output_width + iKN*output_height*output_width] = top[iOW + iOH*output_width + iKN*output_height*output_width] >> SHIFTFIVE;
				}
			}
	}
	return output_height; //since input is a square, return either output_height or output_width is OK for the pooling input size
	
}





int pooling(short *bottom, short *top, int input_size, int input_num, int kernel, int stride){ //top_num is kernel_num
	/*input data, output data, kernel and stride have the same heights and width*/
	int input_height = 0;
	int input_width = 0;
	int output_height = 0;
	int output_width = 0;
	int kernel_height = 0;
	int kernel_width = 0;
	int stride_height = 0;
	int stride_width = 0;
	short max;
	short temp_value;

	int iIN; //iterator of image number
	int iOH; //iterator of output height
	int iOW; //iterator of output width
	int iKH;  //iterator of kernel height
	int iKW;  //iterator of kernel width
	
	input_height = input_size;
	input_width = input_height;
	
	output_height = input_size/stride - kernel + stride; //12=24/2-2+2
	output_width = output_height;

	kernel_height = kernel;
	kernel_width = kernel_height;

	stride_height = stride;
	stride_width = stride_height;
		
	int flag;

	for(iIN=0; iIN<input_num; iIN++){
		for(iOH=0; iOH<output_height; iOH++){
			for(iOW=0; iOW<output_width; iOW++){
				flag = 0;
				for(iKH=0; iKH<kernel_height; iKH++){
					for(iKW=0; iKW<kernel_width; iKW++){
						temp_value = bottom[iKW + iKH*input_width + iOW*stride_width + iOH*stride_height*input_height + iIN*input_height*input_width];						
						if(flag == 0){
							max = temp_value;
							flag = 1;
						}else{
							if(temp_value > max){
								max = temp_value;
							}
						}
						
					}
				
				}
				top[iOW + iOH*output_width + iIN*output_height*output_width] = max;
			}
		}
	}
	return output_height; //since input is a square, return either output_height or output_width is OK for the pooling input size
	
}





void innerproduct(short *bottom, short *top, short *weight, short *bias, int bottom_num, int top_num){//(data, out, weight,bias,datanum,outnum)
	int sum;;
	int i, j;
	
	for(i=0; i<bottom_num*top_num; i++){
		if((weight[i] > 127) || (weight[i] < -128)){
			printf("ip weight overflow\n");
		}
	}

	
	for(i=0; i<top_num; i++){ //outnum
		sum = 0.0;
		for(j=0; j<bottom_num; j++){ //datanum
			sum += bottom[j]*weight[j+i*bottom_num];
		}
		top[i] = sum + bias[i];
		top[i] = top[i] >> SHIFTFIVE;
		//printf("top[%d] = %d\n", i, top[i]);
	}
}

void relu(short *output, int top_num){
	int i;

	for(i=0; i<top_num; i++){
		output[i] = (output[i]>0) ? output[i] : 0;
		//printf("output[%d] = %5.9f\n", i, output[i]);
	}
}

void softmax(short *output, int top_num){
	float bunshi;
	float bunbo;
	float result[10];
	int i,j;
	
	bunshi = 0.0;
	bunbo = 0.0;
	
	float sum;
	sum = 0.0;

	for(i=0; i<top_num; i++){
		output[i] = output[i] >> SHIFTFIVE;
		bunbo += exp(output[i]);
	}
	
	for(i=0; i<top_num; i++){
		bunshi = exp(output[i]);
		result[i] = bunshi/bunbo;
		sum += result[i];
		printf("result[%d] = %5.9f\n", i, result[i]);
		
	}
	printf("sum = %5.9f\n", sum);	
}
