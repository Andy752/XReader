#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QContextMenuEvent>
#include <QString>
#include <QAction>
#include <QImage>

#include <memory>
#include <vector>
#include <queue>
#include <string>

#include "DicomImage.h"
#include "mywidget.h"
 
namespace Ui {
class MainWindow;
}

struct GrowPoint
{
	int x;
	int y;
	int z;
};

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
	void showHistogram(int numOfGroups);

	QMenu *fileMenu;
	QAction *openAct;

	MyWidget* myWidget_1 = nullptr;
	MyWidget* myWidget_2 = nullptr;
	MyWidget* myWidget_3 = nullptr;

	std::vector<unsigned int> histogramVec;

	short autoImageMinVal;
	short autoImageMaxVal;
	int rangeOfGroup;
	std::queue<GrowPoint> growPointQueue;

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

	void changeVerticalScrollBar1Value(int);
	void changeVerticalScrollBar2Value(int);
	void changeVerticalScrollBar3Value(int);

	void horizontalSlider1ValueChanged(int);
	void horizontalSlider2ValueChanged(int);

	void useThreshold(bool);
	void thresholdChanged(QString);
	void horizontalSliderValueChanged(int);

	void addGrowPoint();
	void startGrow();
	void removeGrowResult();
};

#endif // MAINWINDOW_H
