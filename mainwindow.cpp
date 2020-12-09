#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qmessagebox.h"
#include <QWidget>
#include <QFileDialog>
#include <QContextMenuEvent>

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
	delete XImg;
	delete YImg;
	delete ZImg;
	delete fileMenu;
	delete openAct;
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

void MainWindow::verticalScrollBar1ValueChanged(int z)
{
	pDicomImg->GetZImage(z, ZImg,pDicomImg->newMinVal,pDicomImg->newMaxVal);
	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));//在label控件中显示图片
	// ui->label_1->setScaledContents(true);
	ui->label_1->setAlignment(Qt::AlignCenter);
}

void MainWindow::verticalScrollBar2ValueChanged(int y)
{
	pDicomImg->GetYImage(y, YImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	ui->label_2->setPixmap(QPixmap::fromImage(*YImg));//在label控件中显示图片
	// ui->label_2->setScaledContents(true);
	ui->label_2->setAlignment(Qt::AlignCenter);
}

void MainWindow::verticalScrollBar3ValueChanged(int)
{
}

void MainWindow::verticalScrollBar4ValueChanged(int x)
{
	pDicomImg->GetXImage(x, XImg, pDicomImg->newMinVal, pDicomImg->newMaxVal);
	ui->label_4->setPixmap(QPixmap::fromImage(*XImg));//在label控件中显示图片
	// ui->label_4->setScaledContents(true);
	ui->label_4->setAlignment(Qt::AlignCenter);
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
}

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
	ui->verticalScrollBar_2->setMaximum(iDimension[0] - 1);
	ui->verticalScrollBar_2->setValue(iDimension[0] / 2);
	ui->verticalScrollBar_4->setMaximum(iDimension[1] - 1);
	ui->verticalScrollBar_4->setValue(iDimension[1] / 2);

	pDicomImg->GetZImage(ui->verticalScrollBar_1->value(), ZImg,pDicomImg->GetMinVal(),pDicomImg->GetMaxVal());
	pDicomImg->GetYImage(ui->verticalScrollBar_2->value(), YImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());
	pDicomImg->GetXImage(ui->verticalScrollBar_4->value(), XImg, pDicomImg->GetMinVal(), pDicomImg->GetMaxVal());

	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));
	ui->label_2->setPixmap(QPixmap::fromImage(*YImg));
	ui->label_4->setPixmap(QPixmap::fromImage(*XImg));

	ui->lineEdit_1->setText(QString(to_string(pDicomImg->GetLevel()).c_str()));
	ui->lineEdit_2->setText(QString(to_string(pDicomImg->GetWindow()).c_str()));
	ui->lineEdit_3->setText(QString(to_string(pDicomImg->GetMinVal()).c_str()));
	ui->lineEdit_4->setText(QString(to_string(pDicomImg->GetMaxVal()).c_str()));
}



