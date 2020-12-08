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
	void GetZImage(unsigned int z, QImage* &imageQt);
	void GetXImage(unsigned int x, QImage* &imageQt);
	void GetYImage(unsigned int y, QImage* &imageQt);

private:
	std::shared_ptr<std::vector<std::vector<std::vector<short>>>> pData = nullptr; // �洢����б�ʺͽؾ�任���Dicom����ά����
	float fSpaceZ; // Z�������ؼ��ࣨԭʼ��Ƭ��ࣩ
	float fSpaceX; // X�������ؼ���
	float fSpaceY; // Y�������ؼ���
	int iSlope; // �Ҷȱ仯б��
	int iIntercept; // �Ҷȱ任�ؾ�
	int iWinCenter; // ����
	int iWinWidth; // ��λ
	std::vector<unsigned int> iDimension; // ��ά���ϵĳߴ��С�����ؼ�� [x,y,z]
	short minVal; // ����б�ʺͽؾ�任���������Сֵ
	short maxVal; // ����б�ʺͽؾ�任����������ֵ
	short valueRange; // ����ֵ�仯��Χ

private:
	void StoreImageData(char *buffer, const int pageIndex);
};

