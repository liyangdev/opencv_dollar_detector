#include "Detector.h"

//reads the detector model from the xml model
//for now, it must be like this since the current model
//was not written by this program this will change after we are 
//set on a class structure
void Detector::importDetectorModel(String fileName)
{
	FileStorage xml;
	FileNode currentNode;

	printf("before opening File\n");

	xml.open(fileName, FileStorage::READ);

	if (!xml.isOpened())
	{
		std::cerr << "Failed to open " << fileName << std::endl;
	}
	else
	{
		printf("after opening File\n");
		currentNode = xml["detector"]["opts"]["pPyramid"]["pChns"];
		readChannelFeatures(currentNode);

		currentNode = xml["detector"]["opts"]["pPyramid"];
		readPyramid(currentNode);

		currentNode = xml["detector"]["opts"];
		readOptions(currentNode);


		//there's a problem to read these big matrices because the numbers are not in scientific notation
		currentNode = xml["detector"]["clf"];
		currentNode["fids"] >> clf.fids;
		//all of the elements in fids become zero

		currentNode["thrs"] >> clf.thrs;

		//some more matrices would need to be read here...

		clf.treeDepth = currentNode["treeDepth"];
	
	}
}

void Detector::readColorChannel(FileNode colorNode)
{
	opts.pPyramid.pChns.pColor.enabled = colorNode["enabled"];
	opts.pPyramid.pChns.pColor.smooth = colorNode["smooth"];
	opts.pPyramid.pChns.pColor.colorSpaceType = (string)colorNode["colorSpace"];
	opts.pPyramid.pChns.pColor.nChannels = colorNode["nChns"];
	opts.pPyramid.pChns.pColor.padWith = (string)colorNode["padWith"];
}

void Detector::readGradientMagnitude(FileNode gradMagNode)
{
	opts.pPyramid.pChns.pGradMag.enabled = gradMagNode["enabled"];
	opts.pPyramid.pChns.pGradMag.colorChannelIndex = gradMagNode["colorChn"];
	opts.pPyramid.pChns.pGradMag.normalizationRadius = gradMagNode["normRad"];
	opts.pPyramid.pChns.pGradMag.normalizationConstant = gradMagNode["normConst"];
	opts.pPyramid.pChns.pGradMag.full = gradMagNode["full"];
}

void Detector::readGradientHistogram(FileNode histNode)
{
	opts.pPyramid.pChns.pGradHist.enabled = histNode["enabled"];
	opts.pPyramid.pChns.pGradHist.orientationChannels = histNode["nOrients"];
	opts.pPyramid.pChns.pGradHist.useSoftBinning = histNode["softBin"];
	opts.pPyramid.pChns.pGradHist.useHogNormalization = histNode["useHog"];
	opts.pPyramid.pChns.pGradHist.clipHog = histNode["clipHog"];
}

void Detector::readChannelFeatures(FileNode chFeatNode)
{
	FileNode currentNode;
	currentNode = chFeatNode["pColor"];
	readColorChannel(currentNode);
	
	currentNode = chFeatNode["pGradMag"]; 
	readGradientMagnitude(currentNode);

	currentNode = chFeatNode["pGradHist"];
	readGradientHistogram(currentNode);	

	opts.pPyramid.pChns.complete = chFeatNode["complete"];
}

void Detector::readPyramid(FileNode pyramidNode)
{
	opts.pPyramid.scalesPerOctave = pyramidNode["nPerOct"];
	opts.pPyramid.upsampledOctaves = pyramidNode["nOctUp"];
	opts.pPyramid.approximatedScales = pyramidNode["nApprox"];
	opts.pPyramid.lambdas[0] = pyramidNode["lambdas"][0];
	opts.pPyramid.lambdas[1] = pyramidNode["lambdas"][1];
	opts.pPyramid.lambdas[2] = pyramidNode["lambdas"][2];
	opts.pPyramid.pad[0] = pyramidNode["pad"][0];
	opts.pPyramid.pad[1] = pyramidNode["pad"][1];
	opts.pPyramid.minImgSize[0] = pyramidNode["minDs"][0];
	opts.pPyramid.minImgSize[1] = pyramidNode["minDs"][1];
	opts.pPyramid.smoothRadius = pyramidNode["smooth"];
	opts.pPyramid.concatenateChannels = pyramidNode["concat"];
	opts.pPyramid.completeInput = pyramidNode["complete"];
}

void Detector::readOptions(FileNode optionsNode)
{
	opts.modelDs[0] = optionsNode["modelDs"][0];
	opts.modelDs[1] = optionsNode["modelDs"][1];
	opts.modelDsPad[0] = optionsNode["modelDsPad"][0];
	opts.modelDsPad[1] = optionsNode["modelDsPad"][1];
	opts.stride = optionsNode["stride"];
	opts.cascadeThreshold = optionsNode["cascThr"];
	opts.cascadeCalibration = optionsNode["cascCal"];
	opts.nWeak[0] = optionsNode["nWeak"][0];
	opts.nWeak[1] = optionsNode["nWeak"][1];
	opts.nWeak[2] = optionsNode["nWeak"][2];
	opts.nWeak[3] = optionsNode["nWeak"][3];
	opts.seed = optionsNode["seed"];
	opts.name = (string)optionsNode["name"];
	opts.posGtDir = (string)optionsNode["posGtDir"];
	opts.posImgDir = (string)optionsNode["posImgDir"];
	opts.negImgDir = (string)optionsNode["negImgDir"];
	opts.nPos = optionsNode["nPos"];
	opts.nNeg = optionsNode["nNeg"];
	opts.nPerNeg = optionsNode["nPerNeg"];
	opts.nAccNeg = optionsNode["nAccNeg"];
	opts.winsSave = optionsNode["winsSave"];
}

//this procedure was just copied verbatim
void getChild(float *chns1, uint32_t *cids, uint32_t *fids,
    float *thrs, uint32_t offset, uint32_t &k0, uint32_t &k)
{
	float ftr = chns1[cids[fids[k]]];
	k = (ftr<thrs[k]) ? 1 : 2;
	k0 = k += k0 * 2; k += offset;
}

//bb = acfDetect1(P.data{i},Ds{j}.clf,shrink,modelDsPad(1),modelDsPad(2),opts.stride,opts.cascThr);
BoundingBox* Detector::acfDetect(Mat image)
{
	int detectorLength = 100; //what is the length of the detector model? and why is it useful?

	//teste para ver se o conteudo da imagem eh char, se for aplica a funcao imreadf

	//compute feature pyramid
	opts.pPyramid.computeMultiScaleChannelFeaturePyramid(image);


	//criar uma matriz de bounding boxes bbs[numero de escalas da piramide][numero de elementos do detector]
	BoundingBox bb[opts.pPyramid.computedScales][detectorLength];

	//this loop was just copied from the original file (except some comments)
	for (int i = 0; i < opts.pPyramid.computedScales; i++)
	{
		for (int j = 0; j < detectorLength; j++)
		{
			float* chns = (float*)image.data;
			const int shrink = opts.pPyramid.pChns.shrink;
			const int modelHt = opts.modelDsPad[0];
			const int modelWd = opts.modelDsPad[1];
			const int stride = opts.stride;
			const float cascThr = opts.cascadeThreshold;

			float *thrs = (float*) clf.thrs.data;
			float *hs = (float*) clf.hs;
			uint32_t *fids = (uint32_t*) clf.fids.data;
			uint32_t *child = (uint32_t*) clf.child;
			const int treeDepth = clf.treeDepth;

			const int height = image.rows;
			const int width = image.cols;
			const int nChns = image.dims;

			//nTreeNodes size of the first dimension of fids
			const int nTreeNodes = clf.fids.rows;
			//nTrees size of the second dimension of fids
			const int nTrees = clf.fids.cols;
			const int height1 = (int)ceil(float(height*shrink - modelHt + 1 / stride));
			const int width1 = (int)ceil(float(width*shrink - modelWd + 1 / stride));

			//The number of color channels
			int nChannels = opts.pPyramid.pChns.pColor.nChannels;

			//construct cids array
			int nFtrs = modelHt / shrink*modelWd / shrink*nChannels;
			uint32_t *cids = new uint32_t[nFtrs];
			int m = 0;
			for (int z = 0; z<nChannels; z++)
				for (int c = 0; c<modelWd / shrink; c++)
					for (int r = 0; r<modelHt / shrink; r++)
						cids[m++] = z*width*height + c*height + r;

			// apply classifier to each patch
			vector<int> rs, cs; vector<float> hs1;
			for (int c = 0; c<width1; c++) 
				for (int r = 0; r<height1; r++) 
				{
					float h = 0, *chns1 = chns + (r*stride / shrink) + (c*stride / shrink)*height;
					if (treeDepth == 1) 
					{
						// specialized case for treeDepth==1
						for (int t = 0; t < nTrees; t++) 
						{
							uint32_t offset = t*nTreeNodes, k = offset, k0 = 0;
							getChild(chns1, cids, fids, thrs, offset, k0, k);
							h += hs[k]; if (h <= cascThr) break;
						}
					}	
					else if (treeDepth == 2) {
						// specialized case for treeDepth==2
						for (int t = 0; t < nTrees; t++) {
							uint32_t offset = t*nTreeNodes, k = offset, k0 = 0;
							getChild(chns1, cids, fids, thrs, offset, k0, k);
							getChild(chns1, cids, fids, thrs, offset, k0, k);
							h += hs[k]; if (h <= cascThr) break;
						}
					}
					else if (treeDepth>2) 
					{
						// specialized case for treeDepth>2
						for (int t = 0; t < nTrees; t++) 
						{
							uint32_t offset = t*nTreeNodes, k = offset, k0 = 0;
							for (int i = 0; i<treeDepth; i++)
								getChild(chns1, cids, fids, thrs, offset, k0, k);
							h += hs[k]; if (h <= cascThr) break;
						}
					}
					else 
					{
						// general case (variable tree depth)
						for (int t = 0; t < nTrees; t++) 
						{
							uint32_t offset = t*nTreeNodes, k = offset, k0 = k;
							while (child[k]) 
							{
								float ftr = chns1[cids[fids[k]]];
								k = (ftr<thrs[k]) ? 1 : 0;
								k0 = k = child[k0] - k + offset;
							}
							h += hs[k]; if (h <= cascThr) break;
						}
					}
					if (h>cascThr) { cs.push_back(c); rs.push_back(r); hs1.push_back(h); }
				}
				delete [] cids;
				m=cs.size();

				// convert to bbs
				double *bbs;
				for( int i=0; i<m; i++ )
				{
					bbs[i+0*m]=cs[i]*stride; bbs[i+2*m]=modelWd;
					bbs[i+1*m]=rs[i]*stride; bbs[i+3*m]=modelHt;
					bbs[i+4*m]=hs1[i];
				}
		}
	}
	return NULL;
}
