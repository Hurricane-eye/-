#include "ObjectDec.h"

std::vector<cv::Mat> ObjectDec(std::vector<cv::Mat> &srcImages,int type) {
	std::vector<cv::Mat> outImages;
	switch (type) {
	case 0:
		outImages = HumanDec2(srcImages);
		break;
	case 1:
		outImages = VehicleDec(srcImages);
		break;
	default:
		break;
	}
	return outImages;
}

std::vector<cv::Mat> HumanDec(std::vector<cv::Mat> &srcImages) {

	std::vector<cv::Mat> outImages;

	cv::HOGDescriptor peopleHOG;

	//cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();

	peopleHOG.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

	std::vector<cv::Rect>found, found_filtered;

	for (size_t k = 0; k < srcImages.size(); k++) {
		found.clear();
		found_filtered.clear();
		cv::Mat srcImage = srcImages.at(k);
		peopleHOG.detectMultiScale(srcImage, found);
		for (size_t i = 0; i < found.size(); i++) {
			size_t j;
			cv::Rect r = found[i];
			for (j = 0; j < found.size(); j++)
				if (j != i && (r&found[j]) == r)
					break;
			if (j == found.size())
				found_filtered.push_back(r);
		}
		for (size_t i = 0; i < found_filtered.size(); i++) {
			cv::Mat outImage;
			cv::Rect r = found_filtered[i];
			outImage = srcImage(r);
			outImages.push_back(outImage);
		}
		if (found_filtered.size() == 0)
			outImages.push_back(srcImage);
	}
	if (DE_BUG) {
		for (size_t i = 0; i < outImages.size(); i++) {
			cv::imshow("out", outImages[i]);
			cv::waitKey(0);
		}
	}
	return outImages;
}

std::vector<cv::Mat> VehicleDec(std::vector<cv::Mat> &srcImages) {
	std::vector<cv::Mat> outImages;

	cv::HOGDescriptor vehicleHOG;
	vehicleHOG.winSize = cv::Size(40,40);
	/*cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::load("vehicle_detector.yml");*/
	cv::Ptr<cv::ml::SVM> svm;
	svm = cv::ml::StatModel::load<cv::ml::SVM>("vehicle_detector.yml");
	if (svm->empty()) {
		std::cerr << "Can't open the svm file!" << std::endl;
		exit(-1);
	}

	std::vector<float> hogDetector;

	//getSvmDetecor();
	cv::Mat sv = svm->getSupportVectors();
	const int svTotal = sv.rows;
	cv::Mat alpha, svidx;
	double rho = svm->getDecisionFunction(0, alpha, svidx);
	CV_Assert(alpha.total() == 1 && svidx.total() == 1 && svTotal == 1);
	CV_Assert((alpha.type() == CV_64F && alpha.at<double>(0) == 1.) ||
		(alpha.type() == CV_32F && alpha.at<float>(0) == 1.f));
	CV_Assert(sv.type() == CV_32F);
	hogDetector.clear();
	hogDetector.resize(sv.cols + 1);
	memcpy(&hogDetector[0], sv.ptr(), sv.cols * sizeof(hogDetector[0]));
	hogDetector[sv.cols] = (float)-rho;


	vehicleHOG.setSVMDetector(hogDetector);

	std::vector<cv::Rect>found, found_filtered;

	for (size_t k = 0; k < srcImages.size(); k++) {
		found.clear();
		found_filtered.clear();
		cv::Mat srcImage = srcImages.at(k);
		vehicleHOG.detectMultiScale(srcImage, found);
		/*for (size_t i = 0; i < found.size(); i++) {
			size_t j;
			cv::Rect r = found[i];
			for (j = 0; j < found.size(); j++)
				if (j != i && (r&found[j]) == r)
					break;
			if (j == found.size())
				found_filtered.push_back(r);
		}*/
		for (size_t i = 0; i < found.size(); i++) {
			cv::Mat outImage;
			cv::Rect r = found[i];
			outImage = srcImage(r);
			outImages.push_back(outImage);
		}
		if (found.size() == 0)
			outImages.push_back(srcImage);
	}
	if (DE_BUG) {
		for (size_t i = 0; i < outImages.size(); i++) {
			cv::imshow("out", outImages[i]);
			cv::waitKey(0);
		}
	}

	return outImages;
}

std::vector<cv::Mat> HumanDec2(std::vector<cv::Mat> &srcImages) {
	std::vector<cv::Mat> outImages;
	//HOG���������������HOG�����ӵ�    
	//��ⴰ��(64,128),��ߴ�(16,16),�鲽��(8,8),cell�ߴ�(8,8),ֱ��ͼbin����9    
	cv::HOGDescriptor hog(cv::Size(64, 128), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 9);

	//HOG�����ӵ�ά������ͼƬ��С����ⴰ�ڴ�С�����С��ϸ����Ԫ��ֱ��ͼbin��������   
	int DescriptorDim;

	//��XML�ļ���ȡѵ���õ�SVMģ��  
	cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::load("SVM_HOG_HUMAN.xml");

	if (svm->empty()) {
		std::cout << "load svm detector failed!!!" << std::endl;
		exit(-1);
	}

	//����������ά������HOG�����ӵ�ά��    
	DescriptorDim = svm->getVarCount();

	//��ȡsvecsmat��Ԫ������Ϊfloat  
	cv::Mat svecsmat = svm->getSupportVectors();
	//��������ά��  
	int svdim = svm->getVarCount();
	int numofsv = svecsmat.rows;

	//alphamat��svindex�����ʼ��������getDecisionFunction()�����ᱨ��  
	cv::Mat alphamat = cv::Mat::zeros(numofsv, svdim, CV_32F);
	cv::Mat svindex = cv::Mat::zeros(1, numofsv, CV_64F);

	cv::Mat Result;
	double rho = svm->getDecisionFunction(0, alphamat, svindex);
	//��alphamatԪ�ص�������������ת��CV_32F  
	alphamat.convertTo(alphamat, CV_32F);
	Result = -1 * alphamat * svecsmat;

	std::vector<float> vec;
	for (int i = 0; i < svdim; ++i) {
		vec.push_back(Result.at<float>(0, i));
	}
	vec.push_back(rho);

	hog.setSVMDetector(vec);

	std::vector<cv::Rect>found, found_filtered;

	for (size_t k = 0; k < srcImages.size(); k++) {
		found.clear();
		found_filtered.clear();
		cv::Mat srcImage = srcImages.at(k);
		hog.detectMultiScale(srcImage, found);
		for (size_t i = 0; i < found.size(); i++) {
			size_t j;
			cv::Rect r = found[i];
			for (j = 0; j < found.size(); j++)
				if (j != i && (r&found[j]) == r)
					break;
			if (j == found.size())
				found_filtered.push_back(r);
		}
		for (size_t i = 0; i < found_filtered.size(); i++) {
			cv::Mat outImage;
			cv::Rect r = found_filtered[i];
			outImage = srcImage(r);
			outImages.push_back(outImage);
		}
		if (found_filtered.size() == 0)
			outImages.push_back(srcImage);
	}
	if (DE_BUG) {
		for (size_t i = 0; i < outImages.size(); i++) {
			cv::imshow("out", outImages[i]);
			cv::waitKey(0);
		}
	}

	return outImages;
}
