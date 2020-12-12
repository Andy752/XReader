#include "MyWidget.h"
#include "ui_MyWidget.h"

#include <QWheelEvent>
#include <QPainter>

MyWidget::MyWidget(QImage* img,const QString picPath, QWidget *parent)
	: QWidget(parent), Alloffset(0, 0)
{
	ui = new Ui::MyWidget();
	ui->setupUi(this);
	ratio = 1.0;             //初始化图片缩放比例
	action = None;
	img->save(picPath); // 权宜之计，把图片保存到硬盘再读入。因为直接把QImage转为QPixmap得到的是空图像。
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

		//判断鼠标是否是左键按下,且鼠标位置是否在绘画区域
		if (mouse->button() == Qt::LeftButton &&ui->label->geometry().contains(mouse->pos()))
		{
			press = true;
			QApplication::setOverrideCursor(Qt::OpenHandCursor); //设置鼠标样式

			PreDot = mouse->pos();
		}

	}
	else if (event->type() == QEvent::MouseButtonRelease)
	{
		QMouseEvent *mouse = dynamic_cast<QMouseEvent*>(event);

		//判断鼠标是否是左键释放,且之前是在绘画区域
		if (mouse->button() == Qt::LeftButton && press)
		{
			QApplication::setOverrideCursor(Qt::ArrowCursor); //改回鼠标样式
			press = false;
		}
	}
	if (event->type() == QEvent::MouseMove)              //移动图片
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
	if (event->delta() > 0) {      //上滑,缩小

		action = Shrink;
		this->update();

	}
	else {                    //下滑,放大
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

	if (action == MyWidget::Amplification)           //缩小
	{
		ratio -= 0.1*ratio;
		if (ratio < 0.18)
			ratio = 0.1;


	}
	else  if (action == MyWidget::Shrink)           //放大
	{

		ratio += 0.1*ratio;
		if (ratio > 4.5)
			ratio = 5.000;


	}
	if (action == MyWidget::Amplification || action == MyWidget::Shrink || action == MyWidget::JustUpdate)      //更新图片
	{
		NowW = ratio * pixW;
		NowH = ratio * pixH;
		pix->load(picturePath);                 //重新装载,因为之前的图片已经被缩放过
		*pix = pix->scaled(NowW, NowH, Qt::KeepAspectRatio);
		action = MyWidget::None;

	}

	if (action == MyWidget::Move)                    //移动
	{
		int offsetx = Alloffset.x() + offset.x();
		Alloffset.setX(offsetx);

		int offsety = Alloffset.y() + offset.y();
		Alloffset.setY(offsety);
		action = MyWidget::None;
	}

	if (abs(Alloffset.x()) >= (ui->label->width() / 2 + NowW / 2 - 10))    //限制X偏移值
	{
		if (Alloffset.x() > 0)
			Alloffset.setX(ui->label->width() / 2 + NowW / 2 - 10);
		else
			Alloffset.setX(-ui->label->width() / 2 + -NowW / 2 + 10);
	}
	if (abs(Alloffset.y()) >= (ui->label->height() / 2 + NowH / 2 - 10))    //限制Y偏移值
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

	// painter.drawRect(ui->label->x() - 1, ui->label->y() - 1, ui->label->width() + 1, ui->label->height() + 1); //画框
	painter.drawTiledPixmap(x + ui->label->x(), y + ui->label->y(), w, h, *pix, sx, sy);             //绘画图形
}