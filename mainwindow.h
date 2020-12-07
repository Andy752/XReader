#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private slots:
	void verticalScrollBar1ValueChanged(int);
	void verticalScrollBar2ValueChanged(int);
	void verticalScrollBar3ValueChanged(int);
	void verticalScrollBar4ValueChanged(int);
};

#endif // MAINWINDOW_H
