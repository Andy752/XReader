#include "MyWidget.h"
#include "ui_MyWidget.h"

#include <QWheelEvent>
#include <QPainter>

MyWidget::MyWidget(QImage* img,const QString picPath, QWidget *parent)
	: QWidget(parent), Alloffset(0, 0)
{
	ui = new Ui::MyWidget();
	ui->setupUi(this);
	ratio = 1.0;             //��ʼ��ͼƬ���ű���
	action = None;
	img->save(picPath); // Ȩ��֮�ƣ���ͼƬ���浽Ӳ���ٶ��롣��Ϊֱ�Ӱ�QImageתΪQPixmap�õ����ǿ�ͼ��
	picturePath = picPath;
	pix = new QPixmap;
	pix->load(picturePath);
	pixW = pix->width();
	pixH = pix->height();
	ui->label->setText("");
}

MyWidget::~MyWidget()
{
	delete ui;
}

bool MyWidget::event(QEvent * event)
{
	static bool press = false;
	static QPoint PreDot;

	if (event->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent *mouse = dynamic_cast<QMouseEvent*>(event);

		//�ж�����Ƿ����������,�����λ���Ƿ��ڻ滭����
		if (mouse->button() == Qt::LeftButton &&ui->label->geometry().contains(mouse->pos()))
		{
			press = true;
			QApplication::setOverrideCursor(Qt::OpenHandCursor); //���������ʽ

			PreDot = mouse->pos();
		}

	}
	else if (event->type() == QEvent::MouseButtonRelease)
	{
		QMouseEvent *mouse = dynamic_cast<QMouseEvent*>(event);

		//�ж�����Ƿ�������ͷ�,��֮ǰ���ڻ滭����
		if (mouse->button() == Qt::LeftButton && press)
		{
			QApplication::setOverrideCursor(Qt::ArrowCursor); //�Ļ������ʽ
			press = false;
		}
	}
	if (event->type() == QEvent::MouseMove)              //�ƶ�ͼƬ
	{
		if (press)
		{
			QMouseEvent *mouse = dynamic_cast<QMouseEvent*>(event);

			offset.setX(mouse->x() - PreDot.x());
			offset.setY(mouse->y() - PreDot.y());
			PreDot = mouse->pos();
			action = Move;
			this->update();
		}
	}
	return QWidget::event(event);
}

void MyWidget::wheelEvent(QWheelEvent * event)
{
	if (event->delta() > 0) {      //�ϻ�,��С

		action = Shrink;
		this->update();

	}
	else {                    //�»�,�Ŵ�
		action = Amplification;
		this->update();
	}

	event->accept();
}

void MyWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	int NowW = ratio * pixW;
	int NowH = ratio * pixH;

	if (action == MyWidget::Amplification)           //��С
	{
		ratio -= 0.1*ratio;
		if (ratio < 0.18)
			ratio = 0.1;


	}
	else  if (action == MyWidget::Shrink)           //�Ŵ�
	{

		ratio += 0.1*ratio;
		if (ratio > 4.5)
			ratio = 5.000;


	}
	if (action == MyWidget::Amplification || action == MyWidget::Shrink || action == MyWidget::JustUpdate)      //����ͼƬ
	{
		NowW = ratio * pixW;
		NowH = ratio * pixH;
		pix->load(picturePath);                 //����װ��,��Ϊ֮ǰ��ͼƬ�Ѿ������Ź�
		*pix = pix->scaled(NowW, NowH, Qt::KeepAspectRatio);
		action = MyWidget::None;

	}

	if (action == MyWidget::Move)                    //�ƶ�
	{
		int offsetx = Alloffset.x() + offset.x();
		Alloffset.setX(offsetx);

		int offsety = Alloffset.y() + offset.y();
		Alloffset.setY(offsety);
		action = MyWidget::None;
	}

	if (abs(Alloffset.x()) >= (ui->label->width() / 2 + NowW / 2 - 10))    //����Xƫ��ֵ
	{
		if (Alloffset.x() > 0)
			Alloffset.setX(ui->label->width() / 2 + NowW / 2 - 10);
		else
			Alloffset.setX(-ui->label->width() / 2 + -NowW / 2 + 10);
	}
	if (abs(Alloffset.y()) >= (ui->label->height() / 2 + NowH / 2 - 10))    //����Yƫ��ֵ
	{
		if (Alloffset.y() > 0)
			Alloffset.setY(ui->label->height() / 2 + NowH / 2 - 10);
		else
			Alloffset.setY(-ui->label->height() / 2 + -NowH / 2 + 10);

	}

	int x = ui->label->width() / 2 + Alloffset.x() - NowW / 2;
	if (x < 0)
		x = 0;


	int y = ui->label->height() / 2 + Alloffset.y() - NowH / 2;
	if (y < 0)
		y = 0;

	int  sx = NowW / 2 - ui->label->width() / 2 - Alloffset.x();
	if (sx < 0)
		sx = 0;

	int  sy = NowH / 2 - ui->label->height() / 2 - Alloffset.y();
	if (sy < 0)
		sy = 0;


	int w = (NowW - sx) > ui->label->width() ? ui->label->width() : (NowW - sx);
	if (w > (ui->label->width() - x))
		w = ui->label->width() - x;

	int h = (NowH - sy) > ui->label->height() ? ui->label->height() : (NowH - sy);
	if (h > (ui->label->height() - y))
		h = ui->label->height() - y;

	// painter.drawRect(ui->label->x() - 1, ui->label->y() - 1, ui->label->width() + 1, ui->label->height() + 1); //����
	painter.drawTiledPixmap(x + ui->label->x(), y + ui->label->y(), w, h, *pix, sx, sy);             //�滭ͼ��
}