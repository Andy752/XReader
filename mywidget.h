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
		SelectPoint,
		ChangeSliderValue,
		AddGrowPoint
	};

	QPixmap  *pix;
	int action;          //����(�Ŵ�,��С,�ƶ�...)
	int pixW;            //ͼƬ��
	int pixH;            //ͼƬ��

	QRect Paint;         //�滭����
	QLabel label;

	float ratio;                //����
	QPoint offset;              //һ�ε�ͼƬƫ��ֵ
	QPoint Alloffset;           //��ƫ��
	QPoint targetPoint;			//���ѡ���Ŀ���

	QString picturePath;

	int drawCoordinateX;
	int drawCoordinateY;

	int wheelDelta;

	bool event(QEvent * event);
	void wheelEvent(QWheelEvent* e);     //��껬���¼�
	bool loadPicture(const QString picPath);
	void setDrawCoordinateXY(int x, int y);

signals:
	void emitSelectedX(int x);
	void emitSelectedY(int y);
	void emitWheelDelta(int d);

private:
	Ui::MyWidget *ui;
	bool  imagesLoaded;

	void drawCoordinatesLines(QImage & img);

private slots:
	void paintEvent(QPaintEvent *event);
};
