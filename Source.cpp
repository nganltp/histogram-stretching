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
	int hist_w = 1024; int hist_h = 300, histSize = 512;
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
			hist[(int)(Msrc.at<uchar>(y,x))] ++;
		}
	for (int i = 0; i < 256; i++)
		cout << i<<": "<<hist[i]<<endl;
	int i, j;
	int T = 10; // giá trị ngưỡng dùng xác định hai biên
	int a = 0, b = 0;
	int A = 0, B = 255;
	unsigned char cdfLut[256]; // bảng tra
	int sum = 0;

	// xác định giá trị a, b
	for (i = 0; i<256; i++)
		if (hist[i] > T)
		{
			if(a==0) a = i;
			for (j = 255; j>i; j--)
				if (hist[j]>T)
				{
					b = j;
					j = 0; i = 300; // thoat
				}
		}
	cout << "a: " << a<<endl;
	cout << "b: " << b << endl;
	// tính giá trị cho bảng tra
	if ((a>A && b <= B) || (a >= A && b<B))
	{
		for (i = 0; i<256; i++)
		{
			//if((b-a)!=0)
			cdfLut[i] = (unsigned char) (int)((/*hist[i]*/ i - a)*(B - A) / (b - a) + A);
			//else cdfLut[i] = 0;
		}
		cout << endl << "------------------------" << endl << "Table" << endl;
		for (int tab = 0; tab < 256; tab++)
			cout << tab << ": " << cdfLut[tab]<<endl;
		//can bang histogram
		return table_lookup(src, dst, cdfLut, cols, rows);
	}
	return false; // các giá trị không được xác định
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

//myMat = convertToMat(buffer);

int main()
{
	Mat src, dst;
	src = imread("1.png",0);
	//imshow("srcb", src);
	//cvtColor(src, dst, COLOR_BGR2GRAY);
	unsigned char *csrc = src.data;
	//for (unsigned int i = 0; i< src.cols*src.rows; i++)
		//cout << csrc[i] << endl;
	unsigned char *cdst = new unsigned char[src.cols*src.rows];	
	histogramStretch(src,csrc, cdst, src.cols, src.rows);
//	for (unsigned int i = 0; i< src.cols*src.rows; i++)
	//	cout << cdst[i]<<endl;
	dst = convertToMat(cdst, src.cols, src.rows);
	//src = con
	namedWindow("src", WINDOW_AUTOSIZE);
	namedWindow("dst", WINDOW_AUTOSIZE);
	
	imshow("src", src);
	imshow("dst", dst);
	//int a;
	//cin >> a;
	waitKey(0);
	return 0;
}