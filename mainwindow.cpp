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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
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

void MainWindow::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu menu(this);
	menu.addAction(openAct);
	menu.exec(event->globalPos());
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

void MainWindow::drawCoordinatesLines(int da,int db)
{
#if false
	db = da ; // 保证db大于da，db-da就是虚线的短线间距像素个数,da是短线的像素个数
	auto iDimension = pDicomImg->GetDimensions();
	auto currentZ = ui->verticalScrollBar_new1->value();
	auto currentY = ui->verticalScrollBar_new2->value();
	auto currentX = ui->verticalScrollBar_new3->value();
	QColor c(255, 255, 255);

	for (int i = 0; i < iDimension[0]; i = i + db)
	{
		for(int j=i;j<(i+da);++j)
		{
			ZZImg->setPixelColor(currentY, j, c);
		}
	}
	for (int i = 0; i < iDimension[1]; i = i + db)
	{
		for (int j = i; j < (i + da); ++j)
		{
			ZZImg->setPixelColor(j, currentX, c);
		}
	}
	for (int i = 0; i < iDimension[2]; i = i + db)
	{
		for (int j = i; j < (i + da); ++j)
		{
			YYImg->setPixelColor(currentX, j, c);
		}
		
	}
	for (int i = 0; i < iDimension[0]; i = i + db)
	{
		for (int j = i; j < (i + da); ++j)
		{
			YYImg->setPixelColor(j, currentZ, c);
		}
		
	}
	for (int i = 0; i < iDimension[2]; i = i + db)
	{
		for (int j = i; j < (i + da); ++j)
		{
			XXImg->setPixelColor(currentY, j, c);
		}
		
	}
	for (int i = 0; i < iDimension[1]; i = i + db)
	{
		for (int j = i; j < (i + da); ++j)
		{
			XXImg->setPixelColor(j, currentZ, c);
		}
		
	}
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

	drawCoordinatesLines();

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

	drawCoordinatesLines();

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

	// 新界面
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

	drawCoordinatesLines();

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

	// 新界面
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

	drawCoordinatesLines();

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

	drawCoordinatesLines();

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

	drawCoordinatesLines();
	myWidget_1->setDrawCoordinateXY(ui->verticalScrollBar_new2->value(), ui->verticalScrollBar_new3->value());
	myWidget_2->setDrawCoordinateXY(ui->verticalScrollBar_new3->value(), ui->verticalScrollBar_new1->value());
	myWidget_3->setDrawCoordinateXY(ui->verticalScrollBar_new2->value(), ui->verticalScrollBar_new1->value());
	
	myWidget_1->saveAndLoadPicture(ZZImg, QString("ZZ.jpg"));
	myWidget_2->saveAndLoadPicture(YYImg, QString("YY.jpg"));
	myWidget_3->saveAndLoadPicture(XXImg, QString("XX.jpg"));
}

void MainWindow::verticalScrollBarValueChangedNew1(int z)
{
	if (ZZImg == nullptr || XXImg == nullptr || YYImg == nullptr) return;
	pDicomImg->GetZImage(z, ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

	drawCoordinatesLines();
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

	drawCoordinatesLines();
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

	drawCoordinatesLines();
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



