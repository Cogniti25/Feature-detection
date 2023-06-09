#include "blob.h"
#include "filter.h"




void Blob::findBlobKeyPoints(float threshold) {
	int x_ind[3] = { 0, -1, 1 };
	int y_ind[3] = { 0, -1, 1 };
	for (int z = 1; z < scaleSpaceSize - 1; z++) {
		Mat current = LoGImg[z];
		Mat prev = LoGImg[z - 1];
		Mat next = LoGImg[z + 1];
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				bool marked = true;
				if (current.at<double>(i, j) > threshold) {

					for (int x1 = 0; x1 < 3; x1++) {
						for (int y1 = 0; y1 < 3; y1++) {

							if (0 <= x_ind[x1] + i && x_ind[x1] + i < current.rows &&
								0 <= y_ind[y1] + j && y_ind[y1] + j < current.cols &&
								(prev.at<double>(x_ind[x1] + i, y_ind[y1] + j) > current.at<double>(i, j) ||
									next.at<double>(x_ind[x1] + i, y_ind[y1] + j) > current.at<double>(i, j)))
								marked = false;
							if (x1 != 0 && y1 != 0 &&
								0 <= x_ind[x1] + i && x_ind[x1] + i < current.rows &&
								0 <= y_ind[y1] + j && y_ind[y1] + j < current.cols &&
								current.at<double>(x_ind[x1] + i, y_ind[y1] + j) > current.at<double>(i, j))
								marked = false;

						}

					}
					if (marked == true) {
						DogKeypoint kp;
						kp.x = i;
						kp.y = j;
						kp.sigma = sigma[z];
						kp.octave = 0;
						kp.s = z;
						KPs.push_back(kp);
						
					}
				}
			}
		}
	}
	//cout << space[0].convoledImg;
}

void Blob::findScaleSpace() {
	double factor = pow(2.0, 0.5);
	double k = factor;
	for (int i = 1; i < scaleSpaceSize + 2; i++) {
		//if (i % 3 == 0) sigma[i - 1] *= 2;
		sigma[i] = sigma[i - 1] * factor;
	}
}

void Blob::convolveImgWithLoG() {
	for (int i = 0; i < scaleSpaceSize + 2; i++) {
		int kernelSize = floor(sigma[i]) * 6 + 1; // ensure the kernel size wouldnt be too small for a certain sigma
		double* kernel = new double[kernelSize * kernelSize];
		NormLoG(kernel, kernelSize, sigma[i]);
		Mat loGaussian = Mat(kernelSize, kernelSize, CV_64F, kernel);
		filter2D(img, LoGImg[i], -1, loGaussian, Point(-1, -1), 0.0, BORDER_CONSTANT);
		delete[] kernel;
	}
}

vector<vector<Mat>> Blob::getGaussSpace() {
	vector<vector<Mat>> gaussSpace(1);
	for (int i = 1; i < scaleSpaceSize + 1; i++) {
		int kernelSize = floor(sigma[i]) * 6 + 1; // ensure the kernel size wouldnt be too small for a certain sigma
		double* kernel = new double[kernelSize * kernelSize];
		gaussianKernel(kernel, kernelSize, sigma[i]);
		Mat kernelMat = Mat(kernelSize, kernelSize, CV_64F, kernel);
		Mat currentLayer;
		filter2D(img, currentLayer, -1, kernelMat, Point(-1, -1), 0.0, BORDER_CONSTANT);
		gaussSpace[0].push_back(currentLayer);
		delete[] kernel;
	}
	return gaussSpace;
}

Mat detectBlob(Mat img, Mat originalImg, float threshold, double baseSigma, int scaleSpaceSize) {
	//vector<DogKeypoint> KPs = findBlobKeyPoints(img, threshold);
	Blob blob1(img, scaleSpaceSize, baseSigma);
	blob1.findScaleSpace();
	blob1.convolveImgWithLoG();
	blob1.findBlobKeyPoints(threshold);
	vector<DogKeypoint> KPs = blob1.getKeypoints();
	for (int i = 0; i < KPs.size(); i++) {
		//cout << KPs[i].x << "\n";
		circle(originalImg, Point(KPs[i].y, KPs[i].x), KPs[i].sigma * sqrt(2), Scalar(0, 0, 255), 2);
	}
	return originalImg;
}

vector<DogKeypoint> findBlobInterestedPoints(Mat img, vector<vector<Mat>>& gaussSpace,float threshold) {
	int sigmaCount = 17;
	double baseSigma = 1;
	//vector<DogKeypoint> KPs = findBlobKeyPoints(img, threshold);
	cout << "Find blob with " << sigmaCount << "sigma values, base sigma is " << baseSigma << "\n";
	Blob blob1(img, sigmaCount, baseSigma);
	blob1.findScaleSpace();
	blob1.convolveImgWithLoG();
	blob1.findBlobKeyPoints(threshold);
	vector<DogKeypoint> KPs = blob1.getKeypoints();
	cout << "Total keypoints found " << KPs.size() << "\n";
	gaussSpace = blob1.getGaussSpace();
	return KPs;
}


