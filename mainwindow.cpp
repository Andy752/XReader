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
	pDicomImg->GetZImage(z, ZImg);
	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));//在label控件中显示图片
	// ui->label_1->setScaledContents(true);
	ui->label_1->setAlignment(Qt::AlignCenter);
}

void MainWindow::verticalScrollBar2ValueChanged(int x)
{
	pDicomImg->GetXImage(x, XImg);
	ui->label_2->setPixmap(QPixmap::fromImage(*XImg));//在label控件中显示图片
	// ui->label_2->setScaledContents(true);
	ui->label_2->setAlignment(Qt::AlignCenter);
}

void MainWindow::verticalScrollBar3ValueChanged(int)
{
}

void MainWindow::verticalScrollBar4ValueChanged(int y)
{
	pDicomImg->GetYImage(y, YImg);
	ui->label_4->setPixmap(QPixmap::fromImage(*YImg));//在label控件中显示图片
	// ui->label_4->setScaledContents(true);
	ui->label_4->setAlignment(Qt::AlignCenter);
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

	pDicomImg->GetZImage(ui->verticalScrollBar_1->value(), ZImg);
	pDicomImg->GetXImage(ui->verticalScrollBar_2->value(), XImg);
	pDicomImg->GetYImage(ui->verticalScrollBar_4->value(), YImg);

	ui->label_1->setPixmap(QPixmap::fromImage(*ZImg));//在label控件中显示图片
	// ui->label_1->setScaledContents(true);

	ui->label_2->setPixmap(QPixmap::fromImage(*XImg));//在label控件中显示图片
	// ui->label_2->setScaledContents(true);

	ui->label_4->setPixmap(QPixmap::fromImage(*YImg));//在label控件中显示图片
	// ui->label_4->setScaledContents(true);
}



