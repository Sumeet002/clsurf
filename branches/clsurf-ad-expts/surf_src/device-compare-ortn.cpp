#include <CL/cl.h>
#include <math.h>
#include <eventlist.h>
#include "ad_rule_vec.h"

#include "device-compare-ortn.h"

#include "fissionutils.h"

//enum image_similarity;

/**
 * Call base constructor too.
 */
compare_ortn::compare_ortn():analysis_device()
{
	printf("Derived Class - Compare Orientation Device\n");
	THRESHOLD = 2.0f;
	set_device_state(ENABLED);
	run_orientation_stage_status = ENABLED;
}

void compare_ortn::init_buffers(size_t mem_size)
{

	printf("Allocating %d bytes for orntn",mem_size);
    cl_int status;
    p_features = clCreateBuffer(getContext(),
						CL_MEM_READ_WRITE ,
						mem_size, NULL, &status);
    ad_errChk(status, "Error allocating pinned memory", true);
    n_features = clCreateBuffer(getContext(),
						CL_MEM_READ_WRITE ,
						mem_size, NULL, &status);
	ad_errChk(status, "Error allocating  memory", true);

	opbuff.allocate_buffer(mem_size,getContext());

	//analysis_rules.add_rule(i);

}

/**
 * Assign data to the analysis device's buffers
 * @param prev Previous image
 * @param next Next image
 * @param mem_size Data size
 */
void compare_ortn::assign_buffers_copy(float * prev, float * next, size_t mem_size)
{
	//! Uses Copy calls to move data between
	//! buffer objects
	//print_warning("Copy %d bytes for orntn",mem_size);

 	copyHostToAd(p_features,prev,mem_size);
	copyHostToAd(n_features,next,mem_size);

	//p_img = NULL;
	//n_img = NULL;
}


/**
 * Assign data to the analysis device's buffers
 * @param prev Previous image
 * @param next Next image
 * @param mem_size Data size
 */
void compare_ortn::assign_buffers_mapping(cl_mem prev, cl_mem next, size_t mem_size)
{
	//! Uses the cl_map calls to map the pointers passed to the


	//printf("Assigning %d bytes for orntn",mem_size);
	//p_features = prev;
	//n_features = next;
	if(run_orientation_stage_status == ENABLED)
	{
		cl_int status = CL_SUCCESS;
		status =  clEnqueueCopyBuffer (	cl_getCommandQueue(),
			prev,p_features,
			0,0,
			mem_size,
			0,NULL,	NULL);
		ad_errChk(status,"error copying buffer1",TRUE);
		status =  clEnqueueCopyBuffer (	cl_getCommandQueue(),
			next,n_features,
			0,0,
			mem_size,
			0,NULL,	NULL);
		ad_errChk(status,"error copying buffer2",TRUE);

	}
	//clFinish(cl_getCommandQueue());
	//copyHostToAd(p_features,prev,mem_size);
	//copyHostToAd(n_features,next,mem_size);


}

//int foo = 0;
bool compare_ortn::get_analysis_result(bool run_orientation_stage_status_ip)
{
	static int foo = 0;
	if(run_orientation_stage_status_ip == DISABLED)
	{
		foo = foo+1;
		if(foo%5 == 0)
		{
			run_orientation_stage_status = ENABLED;
			return run_orientation_stage_status;
		}
		else
		{
			run_orientation_stage_status = DISABLED;
			return run_orientation_stage_status;
		}

	}


	//! Read results from processing
	//! Assume that the kernel injection is finished now
	sync();
	int mem_to_map = sizeof(float)*(kernel_vec.at(0)->globalws[0]/kernel_vec.at(0)->localws[0]);
 	float * data = (float *)mapBuffer(opbuff.buffer,mem_to_map,CL_MAP_READ);
 	sync();
 	float diff_value = 0.0f;
	//for(int i=0;i < (kernel_vec.at(0)->globalws[0]); i++)
	for(int i=0;i < (kernel_vec.at(0)->globalws[0]/kernel_vec.at(0)->localws[0]) ; i++)
	{
 		diff_value = diff_value + data[i];
	}

	if(abs(diff_value) > THRESHOLD)
		run_orientation_stage_status = ENABLED;
	else
		run_orientation_stage_status = DISABLED;
	printf("Diff is %f \n",diff_value);
	return run_orientation_stage_status;
}

//! Configure the analysis kernel.
//! At this stage the kernel should be allocated and compiled
//! \param p_img Present Image
//! \param p_img Next Image
void compare_ortn::configure_analysis_kernel( int numIpts)
{
	//	printf("Setting Arguments and Config Analysis Kernel\n");

	//! If present_image and next_image
	kernel_vec.at(0)->dim_globalws = 1;
	kernel_vec.at(0)->dim_localws = 1;
	kernel_vec.at(0)->localws[0] = 64;
	//printf("No of ipts %d as seen from AD\n",numIpts);
	kernel_vec.at(0)->globalws[0] = idivup(numIpts,kernel_vec.at(0)->localws[0]);

	kernel_vec.at(0)->localmemsize = (sizeof(float)*(kernel_vec.at(0)->localws[0]));

 	ad_setKernelArg(getKernel(0), 0,sizeof(cl_mem),(void *)&p_features);
	ad_setKernelArg(getKernel(0), 1,sizeof(cl_mem),(void *)&n_features);
	ad_setKernelArg(getKernel(0), 2,sizeof(cl_mem),(void *)&(opbuff.buffer));
	ad_setKernelArg(getKernel(0), 3,kernel_vec.at(0)->localmemsize, NULL);
	ad_setKernelArg(getKernel(0), 4,sizeof(cl_int), (void *)&numIpts);
	//ad_setKernelArg(getKernel(0), 5,sizeof(cl_int), (void *)&numIpts);


}


void compare_ortn::set_ortn_compare_threshold()
{
	printf("Enter Threshold\n");
	scanf("%f",&THRESHOLD);
}

//! Input- Feature set
//! Output- Sorted set (sorted as per all Xcord followed by all Ycoord)
/*
int sort_features()
{


}

int orntn_check()
{
	cl_mem buff1;
	cl_mem buff2;
	cl_kernel ortn_compare;
	//! Get Feature Sets
	cl_setKernelArg()

	//! Get Feature Set2


}
*/

