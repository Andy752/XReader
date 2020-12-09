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
 
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
#ifndef QT_NO_CONTEXTMENU
	void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU

private:
    Ui::MainWindow *ui;

	QImage *ZImg = nullptr;
	QImage *XImg = nullptr;
	QImage *YImg = nullptr;
	
	// DicomImage dicomImg;
	std::shared_ptr<DicomImage> pDicomImg = nullptr;

	void createActions();
	void createMenus();
	QMenu *fileMenu;
	QAction *openAct;

private slots:
	void verticalScrollBar1ValueChanged(int);
	void verticalScrollBar2ValueChanged(int);
	void verticalScrollBar3ValueChanged(int);
	void verticalScrollBar4ValueChanged(int);
	void levelChanged(QString);
	void windowChanged(QString);
	void minimumChanged(QString);
	void maximumChanged(QString);

	void fileOpen();

};

#endif // MAINWINDOW_H
