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
	MyWidget(const QString picPath,QWidget *parent = Q_NULLPTR);
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
		Move
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

	QString picturePath;

	bool event(QEvent * event);
	void wheelEvent(QWheelEvent* e);     //鼠标滑轮事件

private:
	Ui::MyWidget *ui;

private slots:
	void paintEvent(QPaintEvent *event);
};
