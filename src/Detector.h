#ifndef DETECTOR_H
#define DETECTOR_H

#include "Options.h"
#include "Info.h"
#include "BoundingBox.h"
#include "utils.h"


struct OddConfig {


	float resizeImage;

	OddConfig(int argc, char *argv[]) : 
		resizeImage(1.0)
	{
		for (int i=0; i < argc; ++i) {
			std::cout << argv[i] << std::endl;
			if (strcmp(argv[i], "--resizeImage")==0) {
				std::stringstream ss;
				ss << argv[i+1];
				ss >> resizeImage;
			}
		}

	};


	// reads index of the first and last frames 
	// if (argc > 3)
	// {
	// 	firstFrame = atoi(argv[3]);
	// }
	// if (argc > 4)
	// {
	// 	lastFrame = atoi(argv[4]);
	// }

};


class Detector
{
public:
	Options opts; //opts contains the Pyramid

	//Clf clf;
	cv::Mat fids;
	cv::Mat thrs;
	cv::Mat child;
	cv::Mat hs;
	cv::Mat weights;
	cv::Mat depth;
	cv::Mat errs;
	cv::Mat losses;
	int treeDepth;
	OddConfig config;

	BB_Array_Array detections;

	double timeSpentInDetection;

	void exportDetectorModel(cv::String);
	void importDetectorModel(cv::String);
	BB_Array applyDetectorToFrame(std::vector<Info> pyramid, int shrink, int modelHt, int modelWd, int stride, float cascThr, float *thrs, 
									float *hs, uint32 *fids, uint32 *child, int nTreeNodes, int nTrees, int treeDepth, int nChns);
	BB_Array applyCalibratedDetectorToFrame(std::vector<Info> pyramid, int shrink, int modelHt, int modelWd, int stride, float cascThr, float *thrs, float *hs, 
	uint32 *fids, uint32 *child, int nTreeNodes, int nTrees, int treeDepth, int nChns, float minPedestrianHeight, float maxPedestrianHeight, cv::Mat projection, cv::Mat homography);
	void acfDetect(std::vector<std::string> imageNames, std::string dataSetDirectoryName, int firstFrame, int lastFrame);
	BB_Array nonMaximalSuppression(BB_Array bbs);

	Detector(OddConfig _config):
		config(_config) {

	};

private:
	BoundingBox pyramidRowColumn2BoundingBox(int r, int c,  int modelHt, int modelWd, int ith_scale, int stride);

	BB_Array generateCandidates(int imageHeight, int imageWidth, cv::Mat_<float> &P, 
							float meanHeight = 1700, float stdHeight = 100, float factorStdHeight = 2.0);

	int findClosestScaleFromBbox(std::vector<Info> &pyramid, BoundingBox &bb,
												int modelHeight, int imageHeight);


	BB_Array applyDetectorToFrameSmart(std::vector<Info> pyramid, int shrink, int modelHt, int modelWd, int stride, float cascThr, float *thrs, float *hs, 
										uint32 *fids, uint32 *child, int nTreeNodes, int nTrees, int treeDepth, int nChns, int imageWidth, int imageHeight, 
										cv::Mat_<float> &P, cv::Mat &debug_image);

	void bbTopLeft2PyramidRowColumn(int *r, int *c, BoundingBox &bb, int modelHt, int modelWd, int ith_scale, int stride);

};

#endif
