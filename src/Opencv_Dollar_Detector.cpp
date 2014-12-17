#include <sstream>
#include "Opencv_Dollar_Detector.h"

// valgrind --tool=memcheck --leak-check=yes --log-file=valgrind.log ./opencv_dollar_detector ../opencv_dollar_detector/detector.xml ../datasets/small




// call: ./opencv_dollar_detector ../opencv_dollar_detector/detector.xml ../datasets/small
int main(int argc, char *argv[]) 
{
	if (argc < 2)
	{
		std::cout << " # Argument Error: this program expects at least two arguments (detector file name and data set directory)." << std::endl;
		return 1;
	}
	else
	{
		clock_t start = clock();
		int firstFrame=0, lastFrame=666666666;

		
		OddConfig odd_config(argc, argv);
		Detector d(odd_config);

		// loads all detector settings from the provided xml file
		cv::String detectorFileName = argv[1];
		d.importDetectorModel(detectorFileName);

		// gets names for all the files inside the data set folder
		std::string dataSetDirectory = argv[2];
		std::vector<std::string> imageNames = getDataSetFileNames(dataSetDirectory);


		// apply the detection on all images
		d.acfDetect(imageNames, dataSetDirectory, firstFrame, lastFrame);

		clock_t end = clock();
		double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;

		std::cout << "\nTotal processing time was " << elapsed_secs << " seconds.\n";
		std::cout << "Time elapsed calculating features: " << d.opts.pPyramid.totalTimeForRealScales << std::endl;
		std::cout << "Time elapsed approximating features: " << d.opts.pPyramid.totalTimeForApproxScales << std::endl;
		std::cout << "Time elapsed during detections: " << d.timeSpentInDetection << std::endl;

		return 0;
	}
}
