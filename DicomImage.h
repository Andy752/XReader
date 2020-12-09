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

	int newWindow; // ������Ĵ���
	int newLevel; // ������Ĵ�λ
	short newMinVal; // �������������Сֵ
	short newMaxVal; // ��������������ֵ

private:
	std::shared_ptr<std::vector<std::vector<std::vector<short>>>> pData = nullptr; // �洢����б�ʺͽؾ�任���Dicom����ά����
	// float fSpaceZ; // Z�������ؼ��ࣨԭʼ��Ƭ��ࣩ
	// float fSpaceX; // X�������ؼ���
	// float fSpaceY; // Y�������ؼ���
	int slope; // �Ҷȱ仯б��
	int intercept; // �Ҷȱ任�ؾ�
	int window; // ����
	int level; // ��λ 
	std::vector<unsigned int> dimension; // ��ά���ϵĳߴ��С�����ؼ�� [x,y,z]
	short minVal; // ����б�ʺͽؾ�任���ԭʼ������Сֵ
	short maxVal; // ����б�ʺͽؾ�任���ԭʼ�������ֵ
	short valueRange; // ����ֵ�仯��Χ

	void StoreImageData(char *buffer, const int pageIndex);
};

