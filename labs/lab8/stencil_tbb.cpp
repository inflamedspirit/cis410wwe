#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <tbb/tbb.h>

using namespace tbb;
using namespace cv;


struct pixel {
	double red;
	double green;
	double blue;
	
	pixel(double r, double g, double b) : red(r), green(g), blue(b) {};
};

class forloop_stencil{ 
  long curr;
  char * passmatch;
  bool * done;     

  const int radius;
  const double stddev;
  const int rows;
  const int cols;
  pixel * const in;
  pixel * const out;

  const int dim;
  double * kernel;

  const int i;

public:  
  forloop_stencil(const int radius_, const double stddev_, const int rows_, const int cols_, pixel * const in_, pixel * const out_, double * kernel_, const int dim_, const int i_) : radius(radius_), stddev(stddev_), rows(rows_), cols(cols_), in(in_), out(out_), kernel(kernel_), dim(dim_), i(i_) {}
  void operator()( blocked_range<int> range ) const { 
    for (int j=range.begin(); j<range.end(); j++ ){  
      const int out_offset = i + (j*rows);
      // For each pixel, do the stencil
      for(int x = i - radius, kx = 0; x <= i + radius; ++x, ++kx) {
	for(int y = j - radius, ky = 0; y <= j + radius; ++y, ++ky) {
	  if(x >= 0 && x < rows && y >= 0 && y < cols) {
	    const int in_offset = x + (y*rows);
	    const int k_offset = kx + (ky*dim);
	    out[out_offset].red   += kernel[k_offset] * in[in_offset].red;
	    out[out_offset].green += kernel[k_offset] * in[in_offset].green;
	    out[out_offset].blue  += kernel[k_offset] * in[in_offset].blue;
	  }
	}
      }
    }
  }
};



class forloop_applyPrew{ 
  long curr;
  char * passmatch;
  bool * done;     

  const int radius;
  const int rows;
  const int cols;
  pixel * const in;
  pixel * const out;

  const int dim;
  double * kernel;

  const int i;

public:  
  forloop_applyPrew(const int radius_, const int rows_, const int cols_, pixel * const in_, pixel * const out_, double * kernel_, const int dim_, const int i_) : radius(radius_), rows(rows_), cols(cols_), in(in_), out(out_), kernel(kernel_), dim(dim_), i(i_) {}
  void operator()( blocked_range<int> range ) const { 
    for (int j=range.begin(); j<range.end(); j++ ){  


      const int out_offset = i + (j*rows);
      // For each pixel, do the stencil	
      for(int x = i - radius, kx = 0; x <= i + radius; ++x, ++kx) {
	for(int y = j - radius, ky = 0; y <= j + radius; ++y, ++ky) {
	  if(x >= 0 && x < rows && y >= 0 && y < cols) {
	    const int in_offset = x + (y*rows);
	    const int k_offset = kx + (ky*dim);
	    double intensity = (in[in_offset].red + in[in_offset].green + in[in_offset].blue)/3.0;						
	    out[out_offset].red   += kernel[k_offset] * intensity;
	    out[out_offset].green += kernel[k_offset] * intensity;
	    out[out_offset].blue  += kernel[k_offset] * intensity;
	  }
	}
      }


    }
  }
};




/*
 * The Prewitt kernels can be applied after a blur to help highlight edges
 * The input image must be gray scale/intensities:
 *     double intensity = (in[in_offset].red + in[in_offset].green + in[in_offset].blue)/3.0;
 * Each kernel must be applied to the blured images separately and then composed:
 *     blurred[i] with prewittX -> Xedges[i]
 *     blurred[i] with prewittY -> Yedges[i]
 *     outIntensity[i] = sqrt(Xedges[i]*Xedges[i] + Yedges[i]*Yedges[i])
 * To turn the out intensity to an out color set each color to the intensity
 *     out[i].red = outIntensity[i]
 *     out[i].green = outIntensity[i]
 *     out[i].blue = outIntensity[i]
 *
 * For more on the Prewitt kernels and edge detection:
 *     http://en.wikipedia.org/wiki/Prewitt_operator
 */
void prewittX_kernel(const int rows, const int cols, double * const kernel) {
	if(rows != 3 || cols !=3) {
		std::cerr << "Bad Prewitt kernel matrix\n";
		return;
	}
	for(int i=0;i<3;i++) {
		kernel[0 + (i*rows)] = -1.0;
		kernel[1 + (i*rows)] = 0.0;
		kernel[2 + (i*rows)] = 1.0;
	}
}

void prewittY_kernel(const int rows, const int cols, double * const kernel) {
        if(rows != 3 || cols !=3) {
                std::cerr << "Bad Prewitt kernel matrix\n";
                return;
        }
        for(int i=0;i<3;i++) {
                kernel[i + (0*rows)] = 1.0;
                kernel[i + (1*rows)] = 0.0;
                kernel[i + (2*rows)] = -1.0;
        }
}

/*
 * The gaussian kernel provides a stencil for blurring images based on a 
 * normal distribution
 */
void gaussian_kernel(const int rows, const int cols, const double stddev, double * const kernel) {
	const double denom = 2.0 * stddev * stddev;
	const double g_denom = M_PI * denom;
	const double g_denom_recip = (1.0/g_denom);
	double sum = 0.0;
	for(int i = 0; i < rows; ++i) {
		for(int j = 0; j < cols; ++j) {
			const double row_dist = i - (rows/2);
			const double col_dist = j - (cols/2);
			const double dist_sq = (row_dist * row_dist) + (col_dist * col_dist);
			const double value = g_denom_recip * exp((-dist_sq)/denom);
			kernel[i + (j*rows)] = value;
			sum += value;
		}
	}
	// Normalize
	const double recip_sum = 1.0 / sum;
	for(int i = 0; i < rows; ++i) {
		for(int j = 0; j < cols; ++j) {
			kernel[i + (j*rows)] *= recip_sum;
		}		
	}
}

void apply_stencil(const int radius, const double stddev, const int rows, const int cols, pixel * const in, pixel * const out) {
	const int dim = radius*2+1;
	double kernel[dim*dim];
	gaussian_kernel(dim, dim, stddev, kernel);

	for(int i = 0; i < rows; ++i) {

  parallel_for(blocked_range<int>(0,cols), forloop_stencil(radius,stddev,rows,cols,in,out,kernel,dim,i));

		
	}
}

void apply_kernelY(const int radius, const int rows, const int cols, pixel * const in, pixel * const out) {
	const int dim = radius*2+1;
	double kernel[dim*dim];
	prewittY_kernel(dim, dim, kernel);

	for(int i = 0; i < rows; ++i) {
  //	  #pragma omp parallel for default(shared)

	  parallel_for(blocked_range<int>(0,cols), forloop_applyPrew(radius,rows,cols,in,out,kernel,dim,i));

	}
}

void apply_kernelX(const int radius, const int rows, const int cols, pixel * const in, pixel * const out) {
	const int dim = radius*2+1;
	double kernel[dim*dim];
	prewittX_kernel(dim, dim, kernel);


	for(int i = 0; i < rows; ++i) {
	  //	  #pragma omp parallel for default(shared)
	  parallel_for(blocked_range<int>(0,cols), forloop_applyPrew(radius,rows,cols,in,out,kernel,dim,i));

	}
}


void apply_geoMean(const int rows, const int cols, pixel * const in1, pixel * const in2, pixel * const out) {

  //        #pragma omp parallel for default(shared)
	for(int i = 0; i < rows*cols; ++i) {
			// For each pixel, do the stencil
	  out[i].red   =  sqrt( in1[i].red*in1[i].red + in2[i].red * in2[i].red );
	  out[i].green   = sqrt( in1[i].green*in1[i].green + in2[i].green * in2[i].green );
	  out[i].blue   = sqrt( in1[i].blue*in1[i].blue + in2[i].blue*in2[i].blue );
	
	}
	
}





int main( int argc, char* argv[] ) {

	if(argc != 2) {
		std::cerr << "Usage: " << argv[0] << " imageName\n";
		return 1;
	}

	// Read image
	Mat image;
	image = imread(argv[1], CV_LOAD_IMAGE_COLOR);
	if(!image.data ) {
		std::cout <<  "Error opening " << argv[1] << std::endl;
		return -1;
	}
	
	// Get image into C array of doubles for processing
	const int rows = image.rows;
	const int cols = image.cols;
	pixel * imagePixels = (pixel *) malloc(rows * cols * sizeof(pixel));
	for(int i = 0; i < rows; ++i) {
		for(int j = 0; j < cols; ++j) {
			Vec3b p = image.at<Vec3b>(i, j);
			imagePixels[i + (j*rows)] = pixel(p[0]/255.0,p[1]/255.0,p[2]/255.0);
		}
	}
	
	// Create output arra
	pixel * outPixels = (pixel *) malloc(rows * cols * sizeof(pixel));
	pixel * outPixelsTemp1 = (pixel *) malloc(rows * cols * sizeof(pixel));
	pixel * outPixelsTemp2 = (pixel *) malloc(rows * cols * sizeof(pixel));
	for(int i = 0; i < rows * cols; ++i) {
		outPixels[i].red = 0.0;
		outPixels[i].green = 0.0;
		outPixels[i].blue = 0.0;
	}
	
	// Do the stencil
	apply_stencil(3, 32.0, rows, cols, imagePixels, outPixels);

	// Do the naiive kernels.
	apply_kernelY(1, rows, cols, outPixels, outPixelsTemp1);
	apply_kernelX(1, rows, cols, outPixels, outPixelsTemp2);

	apply_geoMean(rows, cols, outPixelsTemp1, outPixelsTemp2, outPixels);
	//	apply_stencil(3, 32.0, rows, cols, outPixelsTemp, outPixels);

  


	// Create an output image (same size as input)
	Mat dest(rows, cols, CV_8UC3);
	// Copy C array back into image for output
	for(int i = 0; i < rows; ++i) {
		for(int j = 0; j < cols; ++j) {
			const size_t offset = i + (j*rows);
			dest.at<Vec3b>(i, j) = Vec3b(floor(outPixels[offset].red * 255.0),
										 floor(outPixels[offset].green * 255.0),
										 floor(outPixels[offset].blue * 255.0));
		}
	}
	
	imwrite("out.jpg", dest);
	
	
	free(imagePixels);
	free(outPixels);
	free(outPixelsTemp1);
	free(outPixelsTemp2);
	return 0;
}
