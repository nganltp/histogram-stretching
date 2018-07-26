#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
using namespace cv;
using namespace std;

/*Draw histogram for gray Image*/
void drawHis(Mat src, Mat &histImage)
{
	vector<Mat> bgr_planes;
	int hist_w = 512; int hist_h = 300, histSize = 512;
	int bin_w = cvRound((double)hist_w / histSize);
	split(src, bgr_planes);
	float range[] = { 0, 256 };
	const float* histRange = { range };
	bool uniform = true; bool accumulate = false;
	Mat hist;
	calcHist(&bgr_planes[0], 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);
	normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(hist.at<float>(i))),
			Scalar(255, 0, 0), 2, 8, 0);
	}

}

bool table_lookup(unsigned char * src, unsigned char * dst, unsigned char * cdfLut, int cols, int rows)
{
	unsigned char*s = src;
	unsigned char*d = dst;
	int i, j;
	//unsigned char temp, temp2;
	for (j = 0; j<rows; j++)
		for (i = 0; i<cols; i++)
		{
			*d++ = cdfLut[*s++];
		}
	return true;
}

bool histogramStretch(Mat Msrc, unsigned char * src, unsigned char *dst, int cols, int rows)
{
	int hist[256] = { 0 };
	/*tinh hist*/
	for (int x = 0; x < cols; x++)
		for (int y = 0; y < rows; y++)
		{
			hist[(int)(Msrc.at<uchar>(y, x))] ++;
		}
	for (int i = 0; i < 256; i++)
		cout << i << ": " << hist[i] << endl;

	/*Init*/
	int i, j;
	int T = 10; // thresh
	int a = 0, b = 0;
	int A = 0, B = 255;
	unsigned char cdfLut[256]; // lookup
	int sum = 0;

	// detect value for a,b
	for (i = 0; i<256; i++)
		if (hist[i] > T)
		{
			if (a == 0) a = i;
			for (j = 255; j>i; j--)
				if (hist[j]>T)
				{
					b = j;
					j = 0; i = 300; // out
				}
		}
	cout << "a: " << a << endl;
	cout << "b: " << b << endl;
	// calculator for lookup table
	if ((a>A && b <= B) || (a >= A && b<B))
	{
		for (i = 0; i<256; i++)
		{
			//if((b-a)!=0)
			cdfLut[i] = (unsigned char)(int)((/*hist[i]*/ i - a)*(B - A) / (b - a) + A);
			//else cdfLut[i] = 0;
		}
		cout << endl << "------------------------" << endl << "Table" << endl;
		for (int tab = 0; tab < 256; tab++)
			cout << tab << ": " << cdfLut[tab] << endl;
		//update new value
		return table_lookup(src, dst, cdfLut, cols, rows);
	}
	return false; // not exist value
}

Mat convertToMat(unsigned char *buffer, int width, int height) {
	Mat tmp(height, width, CV_8UC1);
	for (int x = 0; x < height; x++)
		for (int y = 0; y < width; y++) {
			{
				int value = (int)buffer[x * width + y];
				tmp.at<uchar>(x, y) = value;
			}
		}
	return tmp;
}

int main()
{
	Mat src, dst;

	/*Init histogram*/
	int hist_w = 512; int hist_h = 300, histSize = 512;
	int bin_w = cvRound((double)hist_w / histSize);
	Mat histSrc(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	Mat histDst(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

	string path;
	cin >> path;
	src = imread(path);
	//src = imread("1.png");
	cvtColor(src, src, COLOR_BGR2GRAY);

	drawHis(src, histSrc);

	unsigned char *csrc = src.data;
	unsigned char *cdst = new unsigned char[src.cols*src.rows];

	/*STRETCHING*/
	histogramStretch(src, csrc, cdst, src.cols, src.rows);
	//	for (unsigned int i = 0; i< src.cols*src.rows; i++)
	//	cout << cdst[i]<<endl;

	/* convert unsigned char to Mat */
	dst = convertToMat(cdst, src.cols, src.rows);
	drawHis(dst, histDst);

	imwrite("Output.png", dst);//save Output Image

	namedWindow("src", WINDOW_AUTOSIZE);
	//	namedWindow("dst", WINDOW_AUTOSIZE);
	//	namedWindow("HistSrc");
	//	namedWindow("HistDst");

	imshow("Src", src);
	imshow("Dst", dst);
	imshow("HistSrc", histSrc);
	imshow("HistDst", histDst);
	waitKey(0);
	return 0;
}