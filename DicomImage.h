#pragma once

#include <vector>
#include <string>
#include <vector>
#include <memory>

#include <QImage>

#include "gdcmImageReader.h"
#include "gdcmDirectory.h"
#include "gdcmScanner.h"

class DicomImage
{
public:
	DicomImage();
	DicomImage(const std::string &filePath);
	DicomImage& operator=(const DicomImage& rhs);
	~DicomImage();
	std::vector<unsigned int>& GetDimensions();
	void GetZImage(unsigned int z, QImage* &imageQt,short minimum,short maximum);
	void GetXImage(unsigned int x, QImage* &imageQt,short minimum,short maximum);
	void GetYImage(unsigned int y, QImage* &imageQt,short minimum,short maximum);
	int GetWindow();
	int GetLevel();
	// int GetSlope();
	// int GetIntercept();
	short GetMinVal();
	short GetMaxVal();

	int newWindow; // 调整后的窗宽
	int newLevel; // 调整后的窗位
	short newMinVal; // 调整后的像素最小值
	short newMaxVal; // 调整后的像素最大值

private:
	std::shared_ptr<std::vector<std::vector<std::vector<short>>>> pData = nullptr; // 存储经过斜率和截距变换后的Dicom的三维数据
	// float fSpaceZ; // Z方向体素间间距（原始切片间距）
	// float fSpaceX; // X方向体素间间距
	// float fSpaceY; // Y方向体素间间距
	int slope; // 灰度变化斜率
	int intercept; // 灰度变换截距
	int window; // 窗宽
	int level; // 窗位 
	std::vector<unsigned int> dimension; // 各维度上的尺寸大小和体素间距 [x,y,z]
	short minVal; // 进行斜率和截距变换后的原始像素最小值
	short maxVal; // 进行斜率和截距变换后的原始像素最大值
	short valueRange; // 像素值变化范围

	void StoreImageData(char *buffer, const int pageIndex);
};

