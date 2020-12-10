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
	const unsigned int nfiles = dy.Load(filePath.c_str(), false); //��Ƭ����
	// if (nfiles <= 0) return 1;
	const std::vector<std::string> fileNames = dy.GetFilenames();
	gdcm::IPPSorter s;
	s.Sort(fileNames);
	const std::vector<std::string> fileNamesV = s.GetFilenames();
	gdcm::Scanner scan;
	scan.AddTag(gdcm::Tag(0x0028, 0x0010));//Row����
	scan.AddTag(gdcm::Tag(0x0028, 0x0011));//Col ����
	// scan.AddTag(gdcm::Tag(0x0020, 0x1041));//Location��λ
	// scan.AddTag(gdcm::Tag(0x0028, 0x0030));//Space ���
	scan.AddTag(gdcm::Tag(0x0028, 0x1053));//RescaleSlope�任б��
	scan.AddTag(gdcm::Tag(0x0028, 0x1052));//RescaleIntercept�任�ؾ�
	// scan.AddTag(gdcm::Tag(0x0028, 0x1050));//Window Center ��λ
	// scan.AddTag(gdcm::Tag(0x0028, 0x1051));//Window Width ����
	scan.Scan(fileNamesV);

	//ͼ����������
	const std::string cRow = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x0010));
	const unsigned int iRow = atoi(cRow.c_str());
	//ͼ����������
	const std::string cCol = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x0011));
	const unsigned int iCol = atoi(cCol.c_str());
	//��Ƭ���
	// const std::string cLocationBegin = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0020, 0x1041));
	// const std::string cLocationEnd = scan.GetValue((*(fileNamesV.end() - 1)).c_str(), gdcm::Tag(0x0020, 0x1041));
	// const float fLocationSize = fabsf(atof(cLocationEnd.c_str()) - atof(cLocationBegin.c_str()));
	// fSpaceZ = fLocationSize / (nfiles - 1);
	//X��Y�������ؼ���
	// const std::string cSpaceXY = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x0030));
	// const int loc = cSpaceXY.find_first_of("\\");
	// const std::string cSpaceX(cSpaceXY, 0, loc);
	// const std::string cSpaceY(cSpaceXY, loc + 1, cSpaceXY.size());
	// fSpaceX = atof(cSpaceX.c_str());
	// fSpaceY = atof(cSpaceY.c_str());
	//�Ҷȱ仯б��
	const std::string cSlope = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x1053));
	slope = atoi(cSlope.c_str());
	//�Ҷȱ任�ؾ�
	const std::string RescaleIntercept = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x1052));
	intercept = atoi(RescaleIntercept.c_str());
	//����ʹ�λ
	// const std::string sWinCenter = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x1050));
	// const std::string sWinWidth = scan.GetValue((*(fileNamesV.begin())).c_str(), gdcm::Tag(0x0028, 0x1051));
	// window = atoi(sWinCenter.c_str());
	// level = atoi(sWinWidth.c_str());


	//��ά���ϵĳߴ��С�����ؼ��
	// const unsigned int dimension[3] = { iRow,iCol,nfiles };
	dimension.push_back(iRow);
	dimension.push_back(iCol);
	dimension.push_back(nfiles);

	// �������ݿռ�
	pData = make_shared<vector<vector<vector<short>>>>(dimension[2], vector<vector<short>>(dimension[0], vector<short>(dimension[1], 0)));

	// �洢����
	for (int page = 0; page < dimension[2]; page++)
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

	// ���㴰��ʹ�λ
	newWindow = window = maxVal - minVal;
	newLevel = level = minVal + window / 2;
}

DicomImage & DicomImage::operator=(const DicomImage & rhs)
{
	pData = rhs.pData;
	slope = rhs.slope;
	intercept = rhs.intercept;
	window = rhs.window;
	level = rhs.level;
	dimension = rhs.dimension;
	minVal = rhs.minVal;
	maxVal = rhs.maxVal;
	valueRange = rhs.valueRange;
	newWindow = rhs.newWindow;
	newLevel = rhs.newLevel;
	newMinVal = rhs.newMinVal;
	newMaxVal = rhs.newMaxVal;
	return *this;
}

DicomImage::~DicomImage()
{
}

std::vector<unsigned int>& DicomImage::GetDimensions()
{
	return dimension;
}

void DicomImage::GetZImage(unsigned int z, QImage *& imageQt, short minimum, short maximum)
{
	unsigned int dimX = pData->at(0).size();
	unsigned int dimY = pData->at(0)[0].size();
	unsigned char *ubuffer = new unsigned char[dimX*dimY];
	unsigned char *pubuffer = ubuffer;
	const short range = maximum - minimum;
	for (int i = 0; i < dimX; i++)
	{
		for (int j = 0; j < dimY; j++)
		{
			if (pData->at(z)[i][j] < minimum) 
			{
				*pubuffer++ = 0;
				continue;
			}

			if (pData->at(z)[i][j] > maximum)
			{
				*pubuffer++ = 255;
				continue;
			}
			*pubuffer++ = ((float)(pData->at(z)[i][j]) - minimum) / range * 255;
		}
	}

	imageQt = new QImage(ubuffer, dimY, dimX, QImage::Format_Grayscale8);
}

void DicomImage::GetXImage(unsigned int x, QImage *& imageQt, short minimum, short maximum)
{
	unsigned int dimZ = pData->size();
	unsigned int dimY = pData->at(0)[0].size();
	unsigned char *ubuffer = new unsigned char[dimZ*dimY];
	unsigned char *pubuffer = ubuffer;
	const short range = maximum - minimum;
	for (int i = dimZ-1; i >= 0; i--)
	{
		for (int j = 0; j < dimY; j++)
		{
			if (pData->at(i)[x][j] < minimum)
			{
				*pubuffer++ = 0;
				continue;
			}

			if (pData->at(i)[x][j] > maximum)
			{
				*pubuffer++ = 255;
				continue;
			}
			*pubuffer++ = ((float)(pData->at(i)[x][j]) - minimum) / range * 255;
		}
	}

	imageQt = new QImage(ubuffer, dimY, dimZ, QImage::Format_Grayscale8);
}

void DicomImage::GetYImage(unsigned int y, QImage *& imageQt, short minimum, short maximum)
{
	unsigned int dimX = pData->at(0).size();
	unsigned int dimZ = pData->size();
	unsigned char *ubuffer = new unsigned char[dimX*dimZ];
	unsigned char *pubuffer = ubuffer;
	const short range = maximum - minimum;
	for (int i = dimZ-1; i >= 0; i--)
	{
		for (int j = 0; j < dimX; j++)
		{
			if (pData->at(i)[j][y] < minimum)
			{
				*pubuffer++ = 0;
				continue;
			}

			if (pData->at(i)[j][y] > maximum)
			{
				*pubuffer++ = 255;
				continue;
			}
			*pubuffer++ = ((float)(pData->at(i)[j][y]) - minimum) / range * 255;
		}
	}

	imageQt = new QImage(ubuffer, dimX, dimZ, QImage::Format_Grayscale8);
}

int DicomImage::GetWindow()
{
	return window;
}

int DicomImage::GetLevel()
{
	return level;
}


short DicomImage::GetMinVal()
{
	return minVal;
}

short DicomImage::GetMaxVal()
{
	return maxVal;
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
			pData->at(pageIndex)[i][j] = (short)(*buffer16) * slope + intercept;
			*pubuffer++ = *buffer16++;
			if (pData->at(pageIndex)[i][j] < minVal) minVal = pData->at(pageIndex)[i][j];
			if (pData->at(pageIndex)[i][j] > maxVal) maxVal = pData->at(pageIndex)[i][j];
		}
	}
	newMinVal = minVal;
	newMaxVal = maxVal;
}
