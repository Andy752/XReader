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

#include <cstring>
#include <cstdint>
#include <vector>
#include <memory>
#include <fstream>
#include <direct.h>
#include <io.h>
#include "gdcmImageReader.h"
#include "gdcmDirectory.h"
#include "gdcmScanner.h"
#include "gdcmIPPSorter.h"

#include "DicomImage.h"
#include "MyWidget.h"

using namespace std;
using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
	numOfBars(100)
{
    ui->setupUi(this);
	createActions();
	createMenus();

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
	delete ZZImg; // 为何不判断是否为nullptr直接delete也不会导致错误？
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
	// fileMenu->addAction(saveAct);
	// fileMenu->addAction(printAct);
	fileMenu->addSeparator();
	// fileMenu->addAction(exitAct);
}

void MainWindow::showHistogram()
{
	int numOfGroups = 60;
	int gap = (pDicomImg->GetMaxVal() - pDicomImg->GetMinVal()) / numOfGroups;
	auto dims = pDicomImg->GetDimensions();
	long long totalNumOfPixels = dims[0] * dims[1] * dims[2];
	vector<unsigned int> histogramVec(numOfGroups, 0);

	pDicomImg->CalculateHistogram(numOfGroups, histogramVec);

#if true

	float barWidth = 1;

	QBarSet* set0 = new QBarSet("Num Of Pixels");//声明QBarSet实例
	// QBarSet* set1 = new QBarSet("ng");
	//QBarSet* set2 = new QBarSet("Axel");
	//QBarSet* set3 = new QBarSet("Mary");
	//QBarSet* set4 = new QBarSet("Samantha");

	//向QBarSet实例
	// string text;
	for (unsigned int num : histogramVec)
	{
		*set0 << (float)num / totalNumOfPixels;
		// text = text + to_string((float)num/ totalNumOfPixels) + " ";
	}
	// QMessageBox::information(NULL, "Title", to_string(totalNumOfPixels).c_str(), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	// QMessageBox::information(NULL, "Title", text.c_str(), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	// *set0 << 11 << 12 << 13 << 14 << 15 << 16;//向set0里不断追加数据
	// *set1 << 1 << 1 << 1 << 2 << 1 << 4;
	//*set2 << 3 << 5 << 8 << 13 << 8 << 5;
	//*set3 << 5 << 6 << 7 << 3 << 4 << 5;
	//*set4 << 9 << 7 << 5 << 3 << 1 << 2;

	//将数据绘制为按类别分组的一系列垂直条，每个条形集中的每个类别一个条添加到系列中。
	QBarSeries* series = new QBarSeries();//给每一列分配区域
	series->append(set0);
	// series->append(set1);
	//series->append(set2);
	//series->append(set3);
	//series->append(set4);

	// series->setBarWidth(barWidth);

	//设置Char，类似于设计excel直方图的标签和导入数据
	QChart* chart = new QChart();
	chart->addSeries(series);//将serise添加到Char中
	chart->setTitle("24 hour operation situation");//char 的标题设置为
	chart->setAnimationOptions(QChart::SeriesAnimations); //动画在图表中启用


	//定义字符串列表，用于X轴标签
	QStringList categories;
	for (int i=0;i<histogramVec.size();i++)
	{
		string text = to_string(pDicomImg->GetMinVal() + i * gap) + "~" + to_string(pDicomImg->GetMinVal() + (i + 1) * gap);
		categories << text.c_str();
	}
	// categories << "8:00~9:00" << "9:00~10:00" << "10:00~11:00" << "11:00~12:00" << "13:00~14:00" << "14:00~15:00";

	//设置X轴参数
	QBarCategoryAxis* axisX = new QBarCategoryAxis();
	axisX->append(categories);//设置X周标签
	chart->addAxis(axisX, Qt::AlignBottom); //将系列标签放到底下
	series->attachAxis(axisX);

	//设置Y轴参数
	QValueAxis* axisY = new QValueAxis();
	// axisY->setRange(0, 1);
	chart->addAxis(axisY, Qt::AlignLeft);//放到左边
	series->attachAxis(axisY);


	//设置标签对应是可视的
	chart->legend()->setVisible(true);
	chart->legend()->setAlignment(Qt::AlignBottom);//放到底部

	// QChartView* chartView = new QChartView(chart);
	// chartView->setRenderHint(QPainter::Antialiasing); //在 painter 上设置给定的渲染

	ui->graphicsView->setChart(chart);
	ui->graphicsView->setRenderHint(QPainter::Antialiasing);
#endif
}


void MainWindow::levelChanged(QString s)
{
	auto newLevel = atoi(s.toStdString().c_str());
	if (newLevel == pDicomImg->newLevel) return;

	pDicomImg->newLevel = newLevel;
	pDicomImg->newMinVal = pDicomImg->newLevel - pDicomImg->newWindow / 2;
	pDicomImg->newMaxVal = pDicomImg->newLevel + pDicomImg->newWindow / 2;
	ui->lineEdit_3->setText(QString(to_string(pDicomImg->newMinVal).c_str()));
	ui->lineEdit_4->setText(QString(to_string(pDicomImg->newMaxVal).c_str()));

	// 新界面
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

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
	auto newWindow = atoi(s.toStdString().c_str());
	if (newWindow == pDicomImg->newWindow) return;

	pDicomImg->newWindow = newWindow;
	pDicomImg->newMinVal = pDicomImg->newLevel - pDicomImg->newWindow / 2;
	pDicomImg->newMaxVal = pDicomImg->newLevel + pDicomImg->newWindow / 2;
	ui->lineEdit_3->setText(QString(to_string(pDicomImg->newMinVal).c_str()));
	ui->lineEdit_4->setText(QString(to_string(pDicomImg->newMaxVal).c_str()));

	// 新界面
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

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
	auto newMinVal = atoi(s.toStdString().c_str());
	if (newMinVal == pDicomImg->newMinVal) return;

	pDicomImg->newMinVal = newMinVal;
	pDicomImg->newWindow = pDicomImg->newMaxVal - pDicomImg->newMinVal;
	pDicomImg->newLevel = pDicomImg->newMinVal + pDicomImg->newWindow / 2;
	ui->lineEdit_1->setText(QString(to_string(pDicomImg->newLevel).c_str()));
	ui->lineEdit_2->setText(QString(to_string(pDicomImg->newWindow).c_str()));

	if(s.toStdString() != to_string(ui->horizontalSlider_2->value()))
	{
		ui->horizontalSlider_2->setValue(atoi(s.toStdString().c_str()));
	}

	// 新界面
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

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
	auto newMaxVal = atoi(s.toStdString().c_str());
	if (newMaxVal == pDicomImg->newMaxVal) return;

	pDicomImg->newMaxVal = newMaxVal;
	pDicomImg->newWindow = pDicomImg->newMaxVal - pDicomImg->newMinVal;
	pDicomImg->newLevel = pDicomImg->newMinVal + pDicomImg->newWindow / 2;
	ui->lineEdit_1->setText(QString(to_string(pDicomImg->newLevel).c_str()));
	ui->lineEdit_2->setText(QString(to_string(pDicomImg->newWindow).c_str()));

	if (s.toStdString() != to_string(ui->horizontalSlider_1->value()))
	{
		ui->horizontalSlider_1->setValue(atoi(s.toStdString().c_str()));
	}

	// 新界面
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

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

	// 新界面
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());

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

}


void MainWindow::fileOpen()
{
	QString directory = "";
	directory = QFileDialog::getExistingDirectory(this, tr("Please Select A Dicom Directory"), directory);
	if(directory == "")
	{
		return;
	}

	// 删除旧的widget
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

	pDicomImg = make_shared<DicomImage>(directory.toStdString().c_str());
	auto iDimension = pDicomImg->GetDimensions();

	ui->lineEdit_1->setText(QString(to_string(pDicomImg->GetLevel()).c_str()));
	ui->lineEdit_2->setText(QString(to_string(pDicomImg->GetWindow()).c_str()));
	ui->lineEdit_3->setText(QString(to_string(pDicomImg->GetMinVal()).c_str()));
	ui->lineEdit_4->setText(QString(to_string(pDicomImg->GetMaxVal()).c_str()));

	// 新界面
	ui->verticalScrollBar_new1->setMaximum(iDimension[2] - 1);
	ui->verticalScrollBar_new2->setMaximum(iDimension[1] - 1);
	ui->verticalScrollBar_new3->setMaximum(iDimension[0] - 1);
	ui->verticalScrollBar_new1->setValue(iDimension[2] / 2);
	ui->verticalScrollBar_new2->setValue(iDimension[1] / 2);
	ui->verticalScrollBar_new3->setValue(iDimension[0] / 2);

	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());

	myWidget_1->setDrawCoordinateXY(ui->verticalScrollBar_new2->value(), ui->verticalScrollBar_new3->value());
	myWidget_2->setDrawCoordinateXY(ui->verticalScrollBar_new3->value(), ui->verticalScrollBar_new1->value());
	myWidget_3->setDrawCoordinateXY(ui->verticalScrollBar_new2->value(), ui->verticalScrollBar_new1->value());
	
	myWidget_1->saveAndLoadPicture(ZZImg, QString("ZZ.jpg"));
	myWidget_2->saveAndLoadPicture(YYImg, QString("YY.jpg"));
	myWidget_3->saveAndLoadPicture(XXImg, QString("XX.jpg"));

	ui->horizontalSlider_1->setMinimum(pDicomImg->GetMinVal());
	ui->horizontalSlider_1->setMaximum(pDicomImg->GetMaxVal());
	ui->horizontalSlider_1->setValue(pDicomImg->GetMaxVal());
	ui->horizontalSlider_2->setMinimum(pDicomImg->GetMinVal());
	ui->horizontalSlider_2->setMaximum(pDicomImg->GetMaxVal());
	ui->horizontalSlider_2->setValue(pDicomImg->GetMinVal());

	showHistogram();
}

void MainWindow::verticalScrollBarValueChangedNew1(int z)
{
	if (ZZImg == nullptr || XXImg == nullptr || YYImg == nullptr) return;
	pDicomImg->GetZImage(z, ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

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
	pDicomImg->GetYImage(y, YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

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
	pDicomImg->GetXImage(x, XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

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



