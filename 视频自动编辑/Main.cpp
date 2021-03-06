#include <iostream>
#include <vector>
#include "ImagesReader.h"
#include "Debug.h"
#include "ObjectDec.h"
#include "ImagesProcessor.h"
#include "VideoCreater.h"


int main(int argc, char *argv[]) {
	std::vector<std::string> imageULR;
	if (argc == 1)
		return 0;
	else {
		for (int i = 1; i < argc; i++) {
			imageULR.push_back(argv[i]);
			if(DE_BUG)
				std::cout << argv[i] << std::endl;
		}
	}
	std::vector<cv::Mat> srcImages = readAllImage(imageULR);
	std::vector<cv::Mat> cutImages = ObjectDec(srcImages);
	std::vector<cv::Mat> outFrames = imagesProcess(cutImages);
	videoCreate(outFrames);

	return 0;
}