#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qmessagebox.h"
#include <QWidget>
#include <QFileDialog>
#include <QContextMenuEvent>
#include <QPainter>
#include <QColor>
#include <QLabel>
#include <QScrollBar>
#include <QtCharts/QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSet>
#include <QTextCodec>


#include <cstring>
#include <cstdint>
#include <vector>
#include <memory>
#include <fstream>
#include <direct.h>
#include <io.h>

#include <opencv.hpp>

#include "gdcmImageReader.h"
#include "gdcmDirectory.h"
#include "gdcmScanner.h"
#include "gdcmIPPSorter.h"

#include "DicomImage.h"
#include "MyWidget.h"

using namespace std;
using namespace QtCharts;
using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
	autoImageMinVal(0),
	autoImageMaxVal(0),
	rangeOfGroup(0)
{
    ui->setupUi(this);
	createActions();
	createMenus();

	setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

	myWidget_1 = new MyWidget(ui->layoutWidget);
	myWidget_1->setObjectName(QString::fromUtf8("myWidget_1"));
	ui->gridLayout_new1->addWidget(myWidget_1, 0, 0, 1, 1);

	myWidget_2 = new MyWidget(ui->layoutWidget);
	myWidget_2->setObjectName(QString::fromUtf8("myWidget_2"));
	ui->gridLayout_new2->addWidget(myWidget_2, 0, 0, 1, 1);

	myWidget_3 = new MyWidget(ui->layoutWidget);
	myWidget_3->setObjectName(QString::fromUtf8("myWidget_3"));
	ui->gridLayout_new3->addWidget(myWidget_3, 0, 0, 1, 1);
}

MainWindow::~MainWindow()
{
    delete ui;
	delete fileMenu;
	delete openAct;

	delete myWidget_1;
	delete myWidget_2;
	delete myWidget_3;
	delete ZZImg; // Ϊ�β��ж��Ƿ�Ϊnullptrֱ��deleteҲ���ᵼ�´���
	delete YYImg;
	delete XXImg;
}


void MainWindow::createActions()
{
	openAct = new QAction(tr("&Open"), this);
	// openAct->setShortcuts(QKeySequence::New); 
	openAct->setStatusTip(tr("Open Dicom directory"));
	connect(openAct, &QAction::triggered, this, &MainWindow::fileOpen);
}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(openAct);
	fileMenu->addSeparator();
}

void MainWindow::showHistogram(int numOfGroups)
{
	rangeOfGroup = (pDicomImg->GetMaxVal() - pDicomImg->GetMinVal()) / numOfGroups;
	auto dims = pDicomImg->GetDimensions();
	long long totalNumOfPixels = dims[0] * dims[1] * dims[2];
	histogramVec.clear();
	histogramVec.resize(numOfGroups);

	pDicomImg->CalculateHistogram(numOfGroups, histogramVec);

	float barWidth = 1;

	QBarSet* set0 = new QBarSet("Image Intensity");//����QBarSetʵ��

	//��QBarSetʵ��
	for (unsigned int num : histogramVec)
	{
		*set0 << (float)num / totalNumOfPixels;
	}

	//�����ݻ���Ϊ���������һϵ�д�ֱ����ÿ�����μ��е�ÿ�����һ������ӵ�ϵ���С�
	QBarSeries* series = new QBarSeries();//��ÿһ�з�������
	series->append(set0);

	series->setBarWidth(barWidth);

	//����Char�����������excelֱ��ͼ�ı�ǩ�͵�������
	QChart* chart = new QChart();
	chart->addSeries(series);//��serise��ӵ�Char��
	chart->setTitle("Histogram");//char �ı�������Ϊ
	chart->setAnimationOptions(QChart::SeriesAnimations); //������ͼ��������


	//�����ַ����б�����X���ǩ
	QStringList categories;
	for (int i=0;i<histogramVec.size();i++)
	{
		string text = to_string(pDicomImg->GetMinVal() + i * rangeOfGroup);
		categories << text.c_str();
	}

	//����X�����
	QBarCategoryAxis* axisX = new QBarCategoryAxis();
	axisX->append(categories);//����X�ܱ�ǩ
	chart->addAxis(axisX, Qt::AlignBottom); //��ϵ�б�ǩ�ŵ�����
	series->attachAxis(axisX);

	//����Y�����
	QValueAxis* axisY = new QValueAxis();
	// axisY->setRange(0, 1);
	chart->addAxis(axisY, Qt::AlignLeft);//�ŵ����
	series->attachAxis(axisY);


	//���ñ�ǩ��Ӧ�ǿ��ӵ�
	chart->legend()->setVisible(true);
	chart->legend()->setAlignment(Qt::AlignBottom);//�ŵ��ײ�

	ui->graphicsView->setChart(chart);
	ui->graphicsView->setRenderHint(QPainter::Antialiasing);
}


void MainWindow::levelChanged(QString s)
{
	auto newLevel = s.toInt();
	if (newLevel == pDicomImg->newLevel) return;

	pDicomImg->newLevel = newLevel;
	pDicomImg->newMinVal = pDicomImg->newLevel - pDicomImg->newWindow / 2;
	pDicomImg->newMaxVal = pDicomImg->newLevel + pDicomImg->newWindow / 2;
	ui->lineEdit_3->setText(QString(to_string(pDicomImg->newMinVal).c_str()));
	ui->lineEdit_4->setText(QString(to_string(pDicomImg->newMaxVal).c_str()));

	// �½���
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());

	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}

void MainWindow::windowChanged(QString s)
{
	auto newWindow = s.toInt();
	if (newWindow == pDicomImg->newWindow) return;

	pDicomImg->newWindow = newWindow;
	pDicomImg->newMinVal = pDicomImg->newLevel - pDicomImg->newWindow / 2;
	pDicomImg->newMaxVal = pDicomImg->newLevel + pDicomImg->newWindow / 2;
	ui->lineEdit_3->setText(QString(to_string(pDicomImg->newMinVal).c_str()));
	ui->lineEdit_4->setText(QString(to_string(pDicomImg->newMaxVal).c_str()));

	// �½���
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());

	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}

void MainWindow::minimumChanged(QString s)
{
	auto newMinVal = s.toInt();
	if (newMinVal == pDicomImg->newMinVal) return;

	pDicomImg->newMinVal = newMinVal;
	pDicomImg->newWindow = pDicomImg->newMaxVal - pDicomImg->newMinVal;
	pDicomImg->newLevel = pDicomImg->newMinVal + pDicomImg->newWindow / 2;
	ui->lineEdit_1->setText(QString(to_string(pDicomImg->newLevel).c_str()));
	ui->lineEdit_2->setText(QString(to_string(pDicomImg->newWindow).c_str()));

	// ui->horizontalSlider->setMinimum(pDicomImg->newMinVal);

	if(s.toStdString() != to_string(ui->horizontalSlider_2->value()))
	{
		ui->horizontalSlider_2->setValue(atoi(s.toStdString().c_str()));
	}

	// �½���
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());

	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}

void MainWindow::maximumChanged(QString s)
{
	auto newMaxVal = s.toInt();
	if (newMaxVal == pDicomImg->newMaxVal) return;

	pDicomImg->newMaxVal = newMaxVal;
	pDicomImg->newWindow = pDicomImg->newMaxVal - pDicomImg->newMinVal;
	pDicomImg->newLevel = pDicomImg->newMinVal + pDicomImg->newWindow / 2;
	ui->lineEdit_1->setText(QString(to_string(pDicomImg->newLevel).c_str()));
	ui->lineEdit_2->setText(QString(to_string(pDicomImg->newWindow).c_str()));

	// ui->horizontalSlider->setMaximum(pDicomImg->newMaxVal);

	if (s.toStdString() != to_string(ui->horizontalSlider_1->value()))
	{
		ui->horizontalSlider_1->setValue(atoi(s.toStdString().c_str()));
	}

	// �½���
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());

	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}

void MainWindow::resetClicked()
{
	ui->lineEdit_1->setText(QString(to_string(pDicomImg->GetLevel()).c_str()));
	ui->lineEdit_2->setText(QString(to_string(pDicomImg->GetWindow()).c_str()));
	ui->lineEdit_3->setText(QString(to_string(pDicomImg->GetMinVal()).c_str()));
	ui->lineEdit_4->setText(QString(to_string(pDicomImg->GetMaxVal()).c_str()));

	ui->horizontalSlider_1->setValue(pDicomImg->GetMaxVal());
	ui->horizontalSlider_2->setValue(pDicomImg->GetMinVal());

	// �½���
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal(), ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal(), ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal(), ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());

	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}

void MainWindow::autoClicked()
{
	auto dim = pDicomImg->GetDimensions();
	long long totalPixels = dim[0] * dim[1] * dim[2];

	const float p = 0.001; // ֱ��ͼ������ֵ�������ڴ���ֵ�ı���Ϊ����������ط�Χ

	for(int i=0;i<histogramVec.size()-2;)
	{
		if (((float)histogramVec[i]/totalPixels) > p) {
			if(((float)histogramVec[i+1] / totalPixels) > p && ((float)histogramVec[i+2] / totalPixels) > p)
			{
				autoImageMinVal = rangeOfGroup * i;
				break;
			}
			else
			{
				i = i + 2;
			}
		}
		else
		{
			i++;
		}
	}
	for (int i = histogramVec.size()-1; i >= 2; )
	{
		if (((float)histogramVec[i]/totalPixels) > p) {
			if(((float)histogramVec[i-1] / totalPixels) > p && ((float)histogramVec[i-2] / totalPixels) > p)
			{
				autoImageMaxVal = rangeOfGroup * i;
				break;
			}
			else
			{
				i = i - 2;
			}
		}
		else
		{
			i--;
		}
	}
	autoImageMinVal = autoImageMinVal + pDicomImg->GetMinVal();
	autoImageMaxVal = autoImageMaxVal + pDicomImg->GetMinVal();
	
	ui->lineEdit_3->setText(QString(to_string(autoImageMinVal).c_str()));
	ui->lineEdit_4->setText(QString(to_string(autoImageMaxVal).c_str()));
	
	// �½���
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, autoImageMinVal, autoImageMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, autoImageMinVal, autoImageMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, autoImageMinVal, autoImageMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	
	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}


void MainWindow::fileOpen()
{
	QString directory = "";
	directory = QFileDialog::getExistingDirectory(this, tr("Please Select A Dicom Directory"), directory);
	if(directory == "")
	{
		return;
	}

	// ɾ���ɵ�widget
	delete myWidget_1;
	delete myWidget_2;
	delete myWidget_3;

	myWidget_1 = new MyWidget(ui->layoutWidget);
	myWidget_1->setObjectName(QString::fromUtf8("myWidget_1"));
	ui->gridLayout_new1->addWidget(myWidget_1, 0, 0, 1, 1);

	myWidget_2 = new MyWidget(ui->layoutWidget);
	myWidget_2->setObjectName(QString::fromUtf8("myWidget_2"));
	ui->gridLayout_new2->addWidget(myWidget_2, 0, 0, 1, 1);

	myWidget_3 = new MyWidget(ui->layoutWidget);
	myWidget_3->setObjectName(QString::fromUtf8("myWidget_3"));
	ui->gridLayout_new3->addWidget(myWidget_3, 0, 0, 1, 1);

	connect(myWidget_1, SIGNAL(emitSelectedX(int)), this, SLOT(setVerticalScrollBar2Value(int)));
	connect(myWidget_1, SIGNAL(emitSelectedY(int)), this, SLOT(setVerticalScrollBar3Value(int)));
	connect(myWidget_2, SIGNAL(emitSelectedX(int)), this, SLOT(setVerticalScrollBar3Value(int)));
	connect(myWidget_2, SIGNAL(emitSelectedY(int)), this, SLOT(setVerticalScrollBar1Value(int)));
	connect(myWidget_3, SIGNAL(emitSelectedX(int)), this, SLOT(setVerticalScrollBar2Value(int)));
	connect(myWidget_3, SIGNAL(emitSelectedY(int)), this, SLOT(setVerticalScrollBar1Value(int)));
	connect(myWidget_1, SIGNAL(emitWheelDelta(int)), this, SLOT(changeVerticalScrollBar1Value(int)));
	connect(myWidget_2, SIGNAL(emitWheelDelta(int)), this, SLOT(changeVerticalScrollBar2Value(int)));
	connect(myWidget_3, SIGNAL(emitWheelDelta(int)), this, SLOT(changeVerticalScrollBar3Value(int)));

	pDicomImg = make_shared<DicomImage>(directory.toStdString().c_str());
	auto iDimension = pDicomImg->GetDimensions();

	ui->lineEdit_1->setText(QString(to_string(pDicomImg->GetLevel()).c_str()));
	ui->lineEdit_2->setText(QString(to_string(pDicomImg->GetWindow()).c_str()));
	ui->lineEdit_3->setText(QString(to_string(pDicomImg->GetMinVal()).c_str()));
	ui->lineEdit_4->setText(QString(to_string(pDicomImg->GetMaxVal()).c_str()));

	// �½���
	ui->verticalScrollBar_new1->setMaximum(iDimension[2] - 1);
	ui->verticalScrollBar_new2->setMaximum(iDimension[1] - 1);
	ui->verticalScrollBar_new3->setMaximum(iDimension[0] - 1);
	ui->verticalScrollBar_new1->setValue(iDimension[2] / 2);
	ui->verticalScrollBar_new2->setValue(iDimension[1] / 2);
	ui->verticalScrollBar_new3->setValue(iDimension[0] / 2);

	short threshold = atoi(ui->lineEdit_5->text().toStdString().c_str());
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal(), ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal(), ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal(), ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());

	myWidget_1->setDrawCoordinateXY(ui->verticalScrollBar_new2->value(), ui->verticalScrollBar_new3->value());
	myWidget_2->setDrawCoordinateXY(ui->verticalScrollBar_new3->value(), ui->verticalScrollBar_new1->value());
	myWidget_3->setDrawCoordinateXY(ui->verticalScrollBar_new2->value(), ui->verticalScrollBar_new1->value());

	ZZImg->save(QString("ZZ.jpg"));
	YYImg->save(QString("YY.jpg"));
	XXImg->save(QString("XX.jpg"));

	myWidget_1->loadPicture(QString("ZZ.jpg"));
	myWidget_2->loadPicture(QString("YY.jpg"));
	myWidget_3->loadPicture(QString("XX.jpg"));

	ui->horizontalSlider_1->setMinimum(pDicomImg->GetMinVal());
	ui->horizontalSlider_1->setMaximum(pDicomImg->GetMaxVal());
	ui->horizontalSlider_1->setValue(pDicomImg->GetMaxVal());
	ui->horizontalSlider_2->setMinimum(pDicomImg->GetMinVal());
	ui->horizontalSlider_2->setMaximum(pDicomImg->GetMaxVal());
	ui->horizontalSlider_2->setValue(pDicomImg->GetMinVal());
	

	showHistogram(50);
}

void MainWindow::verticalScrollBarValueChangedNew1(int z)
{
	if (ZZImg == nullptr || XXImg == nullptr || YYImg == nullptr) return;
	pDicomImg->GetZImage(z, ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());

	myWidget_1->setDrawCoordinateXY(ui->verticalScrollBar_new2->value(), ui->verticalScrollBar_new3->value());
	myWidget_2->setDrawCoordinateXY(ui->verticalScrollBar_new3->value(), z);
	myWidget_3->setDrawCoordinateXY(ui->verticalScrollBar_new2->value(), z);

	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}

void MainWindow::verticalScrollBarValueChangedNew2(int y)
{
	if (ZZImg == nullptr || XXImg == nullptr || YYImg == nullptr) return;
	pDicomImg->GetYImage(y, YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());

	myWidget_1->setDrawCoordinateXY(y, ui->verticalScrollBar_new3->value());
	myWidget_2->setDrawCoordinateXY(ui->verticalScrollBar_new3->value(), ui->verticalScrollBar_new1->value());
	myWidget_3->setDrawCoordinateXY(y, ui->verticalScrollBar_new1->value());

	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}

void MainWindow::verticalScrollBarValueChangedNew3(int x)
{
	if (ZZImg == nullptr || XXImg == nullptr || YYImg == nullptr) return;
	pDicomImg->GetXImage(x, XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), ui->lineEdit_5->text().toInt());

	myWidget_1->setDrawCoordinateXY(ui->verticalScrollBar_new2->value(), x);
	myWidget_2->setDrawCoordinateXY(x, ui->verticalScrollBar_new1->value());
	myWidget_3->setDrawCoordinateXY(ui->verticalScrollBar_new2->value(), ui->verticalScrollBar_new1->value());

	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}

void MainWindow::setVerticalScrollBar1Value(int z)
{
	ui->verticalScrollBar_new1->setValue(z);
}

void MainWindow::setVerticalScrollBar2Value(int y)
{
	ui->verticalScrollBar_new2->setValue(y);
}

void MainWindow::setVerticalScrollBar3Value(int x)
{
	ui->verticalScrollBar_new3->setValue(x);
}

void MainWindow::changeVerticalScrollBar1Value(int d)
{
	int currentValue = ui->verticalScrollBar_new1->value();
	ui->verticalScrollBar_new1->setValue(currentValue - d / 120);
}

void MainWindow::changeVerticalScrollBar2Value(int d)
{
	int currentValue = ui->verticalScrollBar_new2->value();
	ui->verticalScrollBar_new2->setValue(currentValue - d / 120);
}

void MainWindow::changeVerticalScrollBar3Value(int d)
{
	int currentValue = ui->verticalScrollBar_new3->value();
	ui->verticalScrollBar_new3->setValue(currentValue - d / 120);
}

void MainWindow::horizontalSlider1ValueChanged(int max)
{
	int min = ui->horizontalSlider_2->value();
	if(max < min)
	{
		ui->horizontalSlider_1->setValue(min);
		if (to_string(min) != ui->lineEdit_4->text().toStdString())
		{
			ui->lineEdit_4->setText(to_string(min).c_str());
		}
	}
	else
	{
		if (to_string(max) != ui->lineEdit_4->text().toStdString())
		{
			ui->lineEdit_4->setText(to_string(max).c_str());
		}
	}
}

void MainWindow::horizontalSlider2ValueChanged(int min)
{
	int max = ui->horizontalSlider_1->value();
	if (min > max)
	{
		ui->horizontalSlider_2->setValue(max);
		if(to_string(max) != ui->lineEdit_3->text().toStdString())
		{
			ui->lineEdit_3->setText(to_string(max).c_str());
		}
	}
	else
	{
		if (to_string(min) != ui->lineEdit_3->text().toStdString())
		{
			ui->lineEdit_3->setText(to_string(min).c_str());
		}
	}
}

void MainWindow::useThreshold(bool state)
{
	if (ZZImg == nullptr || XXImg == nullptr || YYImg == nullptr) return;

	if(state && ui->lineEdit_5->text() == "")
	{
		int middleValue = pDicomImg->GetMinVal() + (pDicomImg->GetMaxVal() - pDicomImg->GetMinVal()) / 2;
		ui->lineEdit_5->setText(QString(to_string(middleValue).c_str()));

		ui->horizontalSlider->setMinimum(pDicomImg->GetMinVal());
		ui->horizontalSlider->setMaximum(pDicomImg->GetMaxVal());
		ui->horizontalSlider->setValue(middleValue);
	}

	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, state, ui->lineEdit_5->text().toInt());
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, state, ui->lineEdit_5->text().toInt());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, state, ui->lineEdit_5->text().toInt());

	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}

void MainWindow::thresholdChanged(QString s)
{
	int threshold = s.toInt();

	if(s.toStdString() != to_string(ui->horizontalSlider->value()))
	{
		ui->horizontalSlider->setValue(threshold);
	}

	if (ZZImg == nullptr || XXImg == nullptr || YYImg == nullptr) return;
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), threshold);
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), threshold);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), threshold);

	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}

void MainWindow::horizontalSliderValueChanged(int value)
{
	if(ui->lineEdit_5->text().toStdString() != to_string(value))
	{
		ui->lineEdit_5->setText(QString(to_string(value).c_str()));
	}

	if (ZZImg == nullptr || XXImg == nullptr || YYImg == nullptr) return;
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), value);
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), value);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal, ui->checkBox->isChecked(), value);

	ZZImg->save("ZZ.jpg");
	myWidget_1->action = MyWidget::JustUpdate;
	myWidget_1->update();
	YYImg->save("YY.jpg");
	myWidget_2->action = MyWidget::JustUpdate;
	myWidget_2->update();
	XXImg->save("XX.jpg");
	myWidget_3->action = MyWidget::JustUpdate;
	myWidget_3->update();
}

