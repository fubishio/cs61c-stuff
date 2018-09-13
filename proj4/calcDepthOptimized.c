// CS 61C Fall 2015 Project 4

// include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

// include OpenMP
#if !defined(_MSC_VER)
#include <pthread.h>
#endif
#include <omp.h>

#include "calcDepthOptimized.h"
#include "calcDepthNaive.h"

/* DO NOT CHANGE ANYTHING ABOVE THIS LINE. */

float displacementSquared(int dx, int dy)
{
	float squaredDisplacement = dx * dx + dy * dy;
	return squaredDisplacement;
}

void calcDepthOptimized(float *depth, float *left, float *right, int imageWidth, int imageHeight, int featureWidth, int featureHeight, int maximumDisplacement)
{
	int boundsx = 2*featureWidth+1;
	int boundsy = 2*featureHeight+1;
	memset(depth, 0, imageWidth*imageHeight*sizeof(float));

	#pragma omp parallel for
	/* The two outer for loops iterate through each pixel */
	for (int y = featureHeight; y < imageHeight - featureHeight; y++)
	{
		for (int x = featureWidth; x < imageWidth - featureWidth; x++)
		{	

				float minimumSquaredDifference = -1;
				int minimumDy = 0;
				int minimumDx = 0;

				int tempDispylow = -maximumDisplacement;
				int tempDispxlow = -maximumDisplacement;
				int tempDispyhigh = maximumDisplacement;
				int tempDispxhigh = maximumDisplacement;
				

				if (y - maximumDisplacement - featureHeight < 0)
				{
					tempDispylow = -y + featureHeight;
				}

				if (x - maximumDisplacement - featureWidth < 0)
				{
					tempDispxlow = -x + featureWidth;
				}

				if (y + maximumDisplacement + featureHeight >= imageHeight)
				{
					tempDispyhigh = imageHeight - y - featureHeight - 1;
				}

				if (x + maximumDisplacement + featureWidth >= imageWidth)
				{
					tempDispxhigh = imageWidth - x - featureWidth - 1;
				}

				for (int dy = tempDispylow; dy <= tempDispyhigh; dy++)
				{
					for (int dx = tempDispxlow; dx <= tempDispxhigh; dx++)
					{
						float squaredDifference = 0;

						__m128 squaredDifferencetemp = _mm_setzero_ps();

						for (int boxY = 0; boxY < boundsy; boxY+=1)
						{
							int leftY = y + boxY - featureHeight;
							int rightY = y + dy + boxY - featureHeight;
							for (int boxX = 0; boxX < (boundsx)/4*4; boxX+=4)
							{
								int leftX = x + boxX - featureWidth;
								int rightX = x + dx + boxX - featureWidth;

								__m128 lefttemp = _mm_loadu_ps(left + leftY * imageWidth + leftX);
								__m128 righttemp = _mm_loadu_ps(right + rightY * imageWidth + rightX);
								__m128 diff = _mm_sub_ps(lefttemp, righttemp);
								diff = _mm_mul_ps(diff, diff);
								squaredDifferencetemp = _mm_add_ps(squaredDifferencetemp, diff);
								
							}

						}

						squaredDifference += squaredDifferencetemp[0] + squaredDifferencetemp[1] + squaredDifferencetemp[2] + squaredDifferencetemp[3];

						if (squaredDifference <= minimumSquaredDifference || minimumSquaredDifference == -1)
						{
							for (int boxY = 0; boxY < boundsy; boxY+=1)
							{
								int leftY = y + boxY - featureHeight;
								int rightY = y + dy + boxY - featureHeight;
								for (int boxX = (boundsx)-((boundsx)%4); boxX < (boundsx); boxX+=1)
								{

									int leftX = x + boxX - featureWidth;
									int rightX = x + dx + boxX - featureWidth;

									float difference = left[leftY * imageWidth + leftX] - right[rightY * imageWidth + rightX];
									squaredDifference += difference * difference;
								}
							}
						}
							if ((minimumSquaredDifference == -1) || ((minimumSquaredDifference == squaredDifference) && (displacementSquared(dx, dy) < displacementSquared(minimumDx, minimumDy))) || (minimumSquaredDifference > squaredDifference))
							{
								minimumSquaredDifference = squaredDifference;
								minimumDx = dx;
								minimumDy = dy;
							}
						
					}

				if (minimumSquaredDifference != -1)
				{
					if (maximumDisplacement == 0)
					{
						depth[y * imageWidth + x] = 0;
					}
					else
					{
						depth[y * imageWidth + x] = displacementNaive(minimumDx, minimumDy);
					}
				}
				else
				{
					depth[y * imageWidth + x] = 0;
				}
			}
		}
	}
}
