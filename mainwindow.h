#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QContextMenuEvent>
#include <QString>
#include <QAction>
#include <QImage>

#include <memory>

#include "DicomImage.h"
#include "mywidget.h"
 
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

	QImage *ZZImg = nullptr;
	QImage *YYImg = nullptr;
	QImage *XXImg = nullptr;
	
	// DicomImage dicomImg;
	std::shared_ptr<DicomImage> pDicomImg = nullptr;

	void createActions();
	void createMenus();
	QMenu *fileMenu;
	QAction *openAct;

	MyWidget* myWidget_1 = nullptr;
	MyWidget* myWidget_2 = nullptr;
	MyWidget* myWidget_3 = nullptr;

private slots:
	void levelChanged(QString);
	void windowChanged(QString);
	void minimumChanged(QString);
	void maximumChanged(QString);
	void resetClicked();
	void autoClicked();

	void fileOpen();

	void verticalScrollBarValueChangedNew1(int);
	void verticalScrollBarValueChangedNew2(int);
	void verticalScrollBarValueChangedNew3(int);

	void setVerticalScrollBar1Value(int);
	void setVerticalScrollBar2Value(int);
	void setVerticalScrollBar3Value(int);
};

#endif // MAINWINDOW_H
