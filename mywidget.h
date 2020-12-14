#pragma once

#include <QWidget>
#include <QMouseEvent>
#include <QLabel>
#include <QString>
#include <QComboBox>
namespace Ui { class MyWidget; };

class MyWidget : public QWidget
{
	Q_OBJECT

public:
	MyWidget(QWidget *parent = Q_NULLPTR);
	~MyWidget();

	enum  Type 
	{
		None = 0,
		Amplification,
		Shrink,
		Lift,
		Right,
		Up,
		Down,
		Move,
		JustUpdate,
		SelectPoint
	};

	QPixmap  *pix;
	int action;          //动作(放大,缩小,移动...)
	int pixW;            //图片宽
	int pixH;            //图片高

	QRect Paint;         //绘画区域
	QLabel label;

	float ratio;                //比例
	QPoint offset;              //一次的图片偏移值
	QPoint Alloffset;           //总偏移
	QPoint targetPoint;			//鼠标选择的目标点

	QString picturePath;

	int drawCoordinateX;
	int drawCoordinateY;

	bool event(QEvent * event);
	void wheelEvent(QWheelEvent* e);     //鼠标滑轮事件
	bool saveAndLoadPicture(QImage* img, const QString picPath);
	void setDrawCoordinateXY(int x, int y);

signals:
	void emitSelectedX(int x);
	void emitSelectedY(int y);
	void emitSelectedXY(int x, int y);

private:
	Ui::MyWidget *ui;
	bool  imagesLoaded;

	void drawCoordinatesLines(QImage & img);

private slots:
	void paintEvent(QPaintEvent *event);
};
