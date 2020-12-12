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
}

MainWindow::~MainWindow()
{
    delete ui;
	delete XImg; // 为何不判断是否为nullptr直接delete也不会导致错误？
	delete YImg;
	delete ZImg;
	delete fileMenu;
	delete openAct;

	if (myWidget_1 != nullptr) delete myWidget_1;
	if (myWidget_2 != nullptr) delete myWidget_2;
	if (myWidget_3 != nullptr) delete myWidget_3;
	delete ZZImg;
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

void MainWindow::drawCoordinatesLines()
{
	auto iDimension = pDicomImg->GetDimensions();
	auto currentZ = ui->verticalScrollBar_new1->value();
	auto currentY = ui->verticalScrollBar_new2->value();
	auto currentX = ui->verticalScrollBar_new3->value();
	QColor c(255, 255, 255);
	// for (int i = 0; i < iDimension[0]; i = i + 3)
	// {
	// 	ZImg->setPixelColor(currentY, i, c);
	// }
	// for (int i = 0; i < iDimension[1]; i = i + 3)
	// {
	// 	ZImg->setPixelColor(i, currentX, c);
	// }
	// for (int i = 0; i < iDimension[2]; i = i + 3)
	// {
	// 	YImg->setPixelColor(currentX, i, c);
	// }
	// for (int i = 0; i < iDimension[0]; i = i + 3)
	// {
	// 	YImg->setPixelColor(i, currentZ, c);
	// }
	// for (int i = 0; i < iDimension[2]; i = i + 3)
	// {
	// 	XImg->setPixelColor(currentY, i, c);
	// }
	// for (int i = 0; i < iDimension[1]; i = i + 3)
	// {
	// 	XImg->setPixelColor(i, currentZ, c);
	// }

	// 新界面
	for (int i = 0; i < iDimension[0]; i = i + 2)
	{
		ZZImg->setPixelColor(currentY, i, c);
	}
	for (int i = 0; i < iDimension[1]; i = i + 2)
	{
		ZZImg->setPixelColor(i, currentX, c);
	}
	for (int i = 0; i < iDimension[2]; i = i + 2)
	{
		YYImg->setPixelColor(currentX, i, c);
	}
	for (int i = 0; i < iDimension[0]; i = i + 2)
	{
		YYImg->setPixelColor(i, currentZ, c);
	}
	for (int i = 0; i < iDimension[2]; i = i + 2)
	{
		XXImg->setPixelColor(currentY, i, c);
	}
	for (int i = 0; i < iDimension[1]; i = i + 2)
	{
		XXImg->setPixelColor(i, currentZ, c);
	}
}

void MainWindow::verticalScrollBar1ValueChanged(int z)
{
	if (ZImg == nullptr || XImg == nullptr || YImg == nullptr) return;
	pDicomImg->GetZImage(z, ZImg,pDicomImg->newMinVal,pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_2->value(), YImg, pDicomImg->newMinVal,pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_4->value(), XImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	drawCoordinatesLines();
	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));
	ui->label_2->setPixmap(QPixmap::fromImage(*YImg));
	ui->label_4->setPixmap(QPixmap::fromImage(*XImg));
}

void MainWindow::verticalScrollBar2ValueChanged(int y)
{
	if (ZImg == nullptr || XImg == nullptr || YImg == nullptr) return;
	pDicomImg->GetYImage(y, YImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetZImage(ui->verticalScrollBar_1->value(), ZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_4->value(), XImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	drawCoordinatesLines();
	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));
	ui->label_2->setPixmap(QPixmap::fromImage(*YImg));
	ui->label_4->setPixmap(QPixmap::fromImage(*XImg));
}

void MainWindow::verticalScrollBar3ValueChanged(int)
{
}

void MainWindow::verticalScrollBar4ValueChanged(int x)
{
	if (ZImg == nullptr || XImg == nullptr || YImg == nullptr) return;
	pDicomImg->GetXImage(x, XImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetZImage(ui->verticalScrollBar_1->value(), ZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_2->value(), YImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	drawCoordinatesLines();
	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));
	ui->label_2->setPixmap(QPixmap::fromImage(*YImg));
	ui->label_4->setPixmap(QPixmap::fromImage(*XImg));
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
	
	pDicomImg->GetZImage(ui->verticalScrollBar_1->value(), ZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_2->value(), YImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_4->value(), XImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	
	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));
	ui->label_2->setPixmap(QPixmap::fromImage(*YImg));
	ui->label_4->setPixmap(QPixmap::fromImage(*XImg));

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

	pDicomImg->GetZImage(ui->verticalScrollBar_1->value(), ZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_2->value(), YImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_4->value(), XImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	
	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));
	ui->label_2->setPixmap(QPixmap::fromImage(*YImg));
	ui->label_4->setPixmap(QPixmap::fromImage(*XImg));

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

	pDicomImg->GetZImage(ui->verticalScrollBar_1->value(), ZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_2->value(), YImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_4->value(), XImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	
	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));
	ui->label_2->setPixmap(QPixmap::fromImage(*YImg));
	ui->label_4->setPixmap(QPixmap::fromImage(*XImg));

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

	pDicomImg->GetZImage(ui->verticalScrollBar_1->value(), ZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_2->value(), YImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_4->value(), XImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	
	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));
	ui->label_2->setPixmap(QPixmap::fromImage(*YImg));
	ui->label_4->setPixmap(QPixmap::fromImage(*XImg));

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
	pDicomImg->GetZImage(ui->verticalScrollBar_1->value(), ZImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());
	pDicomImg->GetYImage(ui->verticalScrollBar_2->value(), YImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());
	pDicomImg->GetXImage(ui->verticalScrollBar_4->value(), XImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());

	

	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));
	ui->label_2->setPixmap(QPixmap::fromImage(*YImg));
	ui->label_4->setPixmap(QPixmap::fromImage(*XImg));

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

// TODO:目前该函数有一个BUG，就是打开一个文件，再打开一个文件，并不会清除第一个文件的显示!!!
void MainWindow::fileOpen()
{
	QString directory = "";
	directory = QFileDialog::getExistingDirectory(this, tr("Please Select A Dicom Directory"), directory);
	if(directory == "")
	{
		return;
	}
	// pDicomImg = DicomImage(directory.toStdString().c_str());
	pDicomImg = make_shared<DicomImage>(directory.toStdString().c_str());
	auto iDimension = pDicomImg->GetDimensions();
	ui->verticalScrollBar_1->setMaximum(iDimension[2] - 1);
	ui->verticalScrollBar_1->setValue(iDimension[2] / 2);
	ui->verticalScrollBar_2->setMaximum(iDimension[1] - 1);
	ui->verticalScrollBar_2->setValue(iDimension[1] / 2);
	ui->verticalScrollBar_4->setMaximum(iDimension[0] - 1);
	ui->verticalScrollBar_4->setValue(iDimension[0] / 2);

	pDicomImg->GetZImage(ui->verticalScrollBar_1->value(), ZImg,pDicomImg->GetMinVal(),pDicomImg->GetMaxVal());
	pDicomImg->GetYImage(ui->verticalScrollBar_2->value(), YImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());
	pDicomImg->GetXImage(ui->verticalScrollBar_4->value(), XImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());

	

	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));
	ui->label_2->setPixmap(QPixmap::fromImage(*YImg));
	ui->label_4->setPixmap(QPixmap::fromImage(*XImg));

	ui->label_1->setAlignment(Qt::AlignCenter);
	ui->label_2->setAlignment(Qt::AlignCenter);
	ui->label_4->setAlignment(Qt::AlignCenter);

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

	myWidget_1 = new MyWidget(ZZImg,QString("ZZ.jpg"), ui->layoutWidget);
	myWidget_1->setObjectName(QString::fromUtf8("myWidget_1"));
	ui->gridLayout_new1->addWidget(myWidget_1, 0, 0, 1, 1);

	myWidget_2 = new MyWidget(YYImg, QString("YY.jpg"), ui->layoutWidget);
	myWidget_2->setObjectName(QString::fromUtf8("myWidget_2"));
	ui->gridLayout_new2->addWidget(myWidget_2, 0, 0, 1, 1);

	myWidget_3 = new MyWidget(XXImg, QString("XX.jpg"), ui->layoutWidget);
	myWidget_3->setObjectName(QString::fromUtf8("myWidget_3"));
	ui->gridLayout_new3->addWidget(myWidget_3, 0, 0, 1, 1);
}

void MainWindow::verticalScrollBarValueChangedNew1(int z)
{
	if (ZZImg == nullptr || XXImg == nullptr || YYImg == nullptr) return;
	pDicomImg->GetZImage(z, ZZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
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

void MainWindow::verticalScrollBarValueChangedNew2(int y)
{
	if (ZZImg == nullptr || XXImg == nullptr || YYImg == nullptr) return;
	pDicomImg->GetYImage(y, YYImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetXImage(ui->verticalScrollBar_new3->value(), XImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

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

void MainWindow::verticalScrollBarValueChangedNew3(int x)
{
	if (ZZImg == nullptr || XXImg == nullptr || YYImg == nullptr) return;
	pDicomImg->GetXImage(x, XXImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetZImage(ui->verticalScrollBar_new1->value(), ZImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	pDicomImg->GetYImage(ui->verticalScrollBar_new2->value(), YImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);

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



