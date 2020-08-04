// CTImageProcess.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//CT图像格式转换，DCM转png
#include <iostream>
#include <string>

//包含DCMTK 头文件和lib  

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcrleccd.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmimage/diregist.h>
#include <dcmtk/dcmjpeg/djdecode.h>

//#pragma comment(lib, "Netapi32.lib")  
//#pragma comment(lib, "ws2_32.lib") 

#include <opencv2/opencv.hpp>
using namespace cv;

using namespace std;

void loadDCMImage();
void loadDCMFile();
int main()
{
	/*std::cout << "Hello World!\n";
	Mat im = imread("123.jpg");
	imshow("test", im);
	waitKey();   */

//	loadDCMImage();
	loadDCMFile();
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

//显示一片黑
void loadDCMImage()
{
	string name("9d7f4df008702001000.dcm");

	DcmFileFormat fileFormat;
	OFCondition oc = fileFormat.loadFile(name.c_str());

	if (!oc.good())//判断文件是否读取成功
	{
		std::cerr << "文件路径有问题" << std::endl;
		return;
	}

	DcmDataset* dataSet = fileFormat.getDataset();
	E_TransferSyntax xfer = dataSet->getCurrentXfer();//得到传输语法

	OFString patientName;
	dataSet->findAndGetOFString(DCM_BitsStored, patientName);//获取病人姓名
	std::cout << "patientName :" << patientName << std::endl;

	unsigned short bitCount(0);
	dataSet->findAndGetUint16(DCM_BitsStored, bitCount);//获取像素的位数 bit
	cout << "bit_count :" << bitCount << endl;

	OFString isRGB;
	dataSet->findAndGetOFString(DCM_PhotometricInterpretation, isRGB);//DCM图片的图像模式
	std::cout << "isrgb :" << isRGB << std::endl;//monochrome 单色

	unsigned short imgBits(0);
	dataSet->findAndGetUint16(DCM_SamplesPerPixel, imgBits);//单个像素占用多少byte
	std::cout << "img_bits :" << imgBits << std::endl;

	unsigned short m_width;
	unsigned short m_height;
	dataSet->findAndGetUint16(DCM_Rows, m_height);
	dataSet->findAndGetUint16(DCM_Columns, m_width);
	cout << "width :" << m_width << endl;//364
	cout << "height " << m_height << endl;//268

	DcmElement* element = nullptr;
	OFCondition result = dataSet->findAndGetElement(DCM_PixelData, element);
	if (result.bad() | element == nullptr)
	{
		std::cerr << "获取像素数据失败!" << std::endl;
	}

	uchar* image8Data = nullptr;
	Uint16* image16Data = nullptr;
	if (8 == bitCount)
	{
		result = element->getUint8Array(image8Data);
	}
	else if (8 < bitCount)
	{
		result = element->getUint16Array(image16Data);
	}

	if (image8Data)
	{
		if (1 == imgBits)
		{
			cv::Mat dst(m_height, m_width, CV_8UC1, cv::Scalar::all(0));
			unsigned char* data = nullptr;
			for (int i = 0; i < m_height; i++)
			{
				data = dst.ptr<unsigned char>(i);	//取得每一行的头指针 也可使用dst2.at<unsigned short>(i, j) = ?
				for (int j = 0; j < m_width; j++)
				{
					*data++ = (unsigned char)((float)image8Data[i*m_width + j] * 255.0 / std::pow(2.0, bitCount) + 0.5);
				}
			}
			cv::imshow("gray", dst);
			cv::waitKey();
		}
	}
	else if (image16Data)
	{
		DicomImage* m_dcmImage = new DicomImage((DcmObject*)dataSet, xfer);
		m_dcmImage->setWindow(268, 364);//这句话很重要，没有的话会一片黑
		Uint16* pixelData=(Uint16*)(m_dcmImage->getOutputData(16));
		if (pixelData != nullptr)
		{
			cv::Mat dst2(m_height, m_width, CV_16UC1, pixelData);
			cv::imshow("image", dst2);
			cv::waitKey(0);
		}
	}
}

void loadDCMFile()
{
	DicomImage *img = new DicomImage("9d7f4df008703011190.dcm");
	int nWidth = img->getWidth();			//获得图像宽度
	int nHeight = img->getHeight();			//获得图像高度
//	img->setWindow(450, 1600);	//肝脏一般取窗宽为450HU，窗位为45HU，这边的窗宽和窗位要自己设定
	img->setWindow(500, 8000);
	Uint16 *pixelData = (Uint16*)(img->getOutputData(16));
	std::cout << nWidth << ", " << nHeight << std::endl;
	if (pixelData != nullptr)
	{
		cv::Mat dst(nHeight, nWidth, CV_16UC1, pixelData);
		cv::resize(dst, dst, cv::Size(dst.cols*2., dst.rows * 2));
		cv::imshow("image2", dst);
		imwrite("008703011190.png", dst);
		cv::waitKey(0);
	}
}