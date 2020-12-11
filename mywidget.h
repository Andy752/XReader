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
	int action;          //����(�Ŵ�,��С,�ƶ�...)
	int pixW;            //ͼƬ��
	int pixH;            //ͼƬ��

	QRect Paint;         //�滭����
	QLabel label;

	float ratio;                //����
	QPoint offset;              //һ�ε�ͼƬƫ��ֵ
	QPoint Alloffset;           //��ƫ��

	QString picturePath;

	bool event(QEvent * event);
	void wheelEvent(QWheelEvent* e);     //��껬���¼�

private:
	Ui::MyWidget *ui;

private slots:
	void paintEvent(QPaintEvent *event);
};
