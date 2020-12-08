#include <vector>
#include <string>
#include <vector>
#include <memory>

#include <QImage>
#include <QMessageBox>

#include "gdcmImageReader.h"
#include "gdcmDirectory.h"
#include "gdcmScanner.h"
#include "gdcmIPPSorter.h"

#include "DicomImage.h"

using namespace  std;

DicomImage::DicomImage()
{
}

DicomImage::DicomImage(const std::string &filePath)
{
	gdcm::Directory dy;
	const unsigned int nfiles = dy.Load(filePath.c_str(), false); //切片层数
	// if (nfiles <= 0) return 1;
	const std::vector<std::string> fileNames = dy.GetFilenames();
	gdcm::IPPSorter s;
	s.Sort(fileNames);
	const std::vector<std::string> fileNamesV = s.GetFilenames();
	gdcm::Scanner scan;
	scan.AddTag(gdcm::Tag(0x0028, 0x0010));//Row行数
	scan.AddTag(gdcm::Tag(0x0028, 0x0011));//Col 列数
	scan.AddTag(gdcm::Tag(0x0020, 0x1041));//Location定位
	scan.AddTag(gdcm::Tag(0x0028, 0x0030));//Space 间距
	scan.AddTag(gdcm::Tag(0x0028, 0x1053));//RescaleSlope变换斜率
	scan.AddTag(gdcm::Tag(0x0028, 0x1052));//RescaleIntercept变换截距
	scan.AddTag(gdcm::Tag(0x0028, 0x1050));//Window Center 窗位
	scan.AddTag(gdcm::Tag(0x0028, 0x1051));//Window Width 窗宽
	scan.Scan(fileNamesV);

	//图像像素行数
	const std::string cRow = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x0010));
	const unsigned int iRow = atoi(cRow.c_str());
	//图像像素列数
	const std::string cCol = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x0011));
	const unsigned int iCol = atoi(cCol.c_str());
	//切片间距
	const std::string cLocationBegin = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0020, 0x1041));
	const std::string cLocationEnd = scan.GetValue((*(fileNamesV.end() - 1)).c_str(), gdcm::Tag(0x0020, 0x1041));
	const float fLocationSize = fabsf(atof(cLocationEnd.c_str()) - atof(cLocationBegin.c_str()));
	fSpaceZ = fLocationSize / (nfiles - 1);
	//X和Y方向体素间间距
	const std::string cSpaceXY = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x0030));
	const int loc = cSpaceXY.find_first_of("\\");
	const std::string cSpaceX(cSpaceXY, 0, loc);
	const std::string cSpaceY(cSpaceXY, loc + 1, cSpaceXY.size());
	fSpaceX = atof(cSpaceX.c_str());
	fSpaceY = atof(cSpaceY.c_str());
	//灰度变化斜率
	const std::string cSlope = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x1053));
	iSlope = atoi(cSlope.c_str());
	//灰度变换截距
	const std::string RescaleIntercept = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x1052));
	iIntercept = atoi(RescaleIntercept.c_str());
	//窗宽和窗位
	const std::string sWinCenter = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x1050));
	const std::string sWinWidth = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x1051));
	iWinCenter = atoi(sWinCenter.c_str());
	iWinWidth = atoi(sWinWidth.c_str());
	//各维度上的尺寸大小和体素间距
	// const unsigned int iDimension[3] = { iRow,iCol,nfiles };
	iDimension.push_back(iRow);
	iDimension.push_back(iCol);
	iDimension.push_back(nfiles);

	// 分配数据空间
	pData = make_shared<vector<vector<vector<short>>>>(iDimension[2], vector<vector<short>>(iDimension[0], vector<short>(iDimension[1], 0)));

	// 存储数据
	for (int page = 0; page < iDimension[2]; page++)
	{
		gdcm::ImageReader reader;
		reader.SetFileName(fileNamesV[page].c_str());
		gdcm::File &file = reader.GetFile();
		if (!reader.Read())
		{
			QMessageBox::information(NULL, "Title", "fail to read Dicom data!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		}
		const gdcm::Image &gimage = reader.GetImage();
		std::vector<char> vbuffer;
		vbuffer.resize(gimage.GetBufferLength());
		char *buffer = &vbuffer[0];
		gimage.GetBuffer(buffer);
		StoreImageData(buffer, page);
	}
}

DicomImage & DicomImage::operator=(const DicomImage & rhs)
{
	pData = rhs.pData;
	fSpaceZ = rhs.fSpaceZ;
	fSpaceX = rhs.fSpaceX;
	fSpaceY = rhs.fSpaceY;
	iSlope = rhs.iSlope;
	iIntercept = rhs.iIntercept;
	iWinCenter = rhs.iWinCenter;
	iWinWidth = rhs.iWinWidth;
	iDimension = rhs.iDimension;
	minVal = rhs.minVal;
	maxVal = rhs.maxVal;
	valueRange = rhs.valueRange;
	return *this;
}

DicomImage::~DicomImage()
{
}

std::vector<unsigned int>& DicomImage::GetDimensions()
{
	return iDimension;
}

void DicomImage::GetZImage(unsigned int z, QImage *& imageQt)
{
	unsigned int dimX = pData->at(0).size();
	unsigned int dimY = pData->at(0)[0].size();
	unsigned char *ubuffer = new unsigned char[dimX*dimY];
	unsigned char *pubuffer = ubuffer;
	const short range = maxVal - minVal;
	for (int i = 0; i < dimX; i++)
	{
		for (int j = 0; j < dimY; j++)
		{
			*pubuffer++ = ((float)(pData->at(z)[i][j]) - minVal) / range * 255;
		}
	}

	imageQt = new QImage(ubuffer, dimY, dimX, QImage::Format_Grayscale8);
}

void DicomImage::GetXImage(unsigned int x, QImage *& imageQt)
{
	unsigned int dimZ = pData->size();
	unsigned int dimY = pData->at(0)[0].size();
	unsigned char *ubuffer = new unsigned char[dimZ*dimY];
	unsigned char *pubuffer = ubuffer;
	const short range = maxVal - minVal;
	for (int i = 0; i < dimZ; i++)
	{
		for (int j = 0; j < dimY; j++)
		{
			*pubuffer++ = ((float)(pData->at(i)[x][j]) - minVal) / range * 255;
		}
	}

	imageQt = new QImage(ubuffer, dimY, dimZ, QImage::Format_Grayscale8);
}

void DicomImage::GetYImage(unsigned int y, QImage *& imageQt)
{
	unsigned int dimX = pData->at(0).size();
	unsigned int dimZ = pData->size();
	unsigned char *ubuffer = new unsigned char[dimX*dimZ];
	unsigned char *pubuffer = ubuffer;
	const short range = maxVal - minVal;
	for (int i = 0; i < dimZ; i++)
	{
		for (int j = 0; j < dimX; j++)
		{
			*pubuffer++ = ((float)(pData->at(i)[j][y]) - minVal) / range * 255;
		}
	}

	imageQt = new QImage(ubuffer, dimX, dimZ, QImage::Format_Grayscale8);
}

void DicomImage::StoreImageData(char * buffer, const int pageIndex)
{
	unsigned int dimX = pData->at(0).size();
	unsigned int dimY = pData->at(0)[0].size();

	short *buffer16 = (short*)buffer;
	unsigned char *ubuffer = new unsigned char[dimX*dimY];
	unsigned char *pubuffer = ubuffer;
	for (int i = 0; i < dimX; i++)
	{
		for (int j = 0; j < dimY; j++)
		{
			pData->at(pageIndex)[i][j] = (short)(*buffer16) * iSlope + iIntercept;
			*pubuffer++ = *buffer16++;
			if (pData->at(pageIndex)[i][j] < minVal) minVal = pData->at(pageIndex)[i][j];
			if (pData->at(pageIndex)[i][j] > maxVal) maxVal = pData->at(pageIndex)[i][j];
		}
	}
}
