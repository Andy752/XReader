#include "MyWidget.h"
#include "ui_MyWidget.h"

#include <QWheelEvent>
#include <QPainter>
#include <QMessageBox>

MyWidget::MyWidget(QWidget *parent)
	: QWidget(parent), Alloffset(0, 0),ratio(1.0),action(None),picturePath(""),
	pix(nullptr),pixW(0),pixH(0),imagesLoaded(false),drawCoordinateX(0),drawCoordinateY(0),targetPoint(0,0),wheelDelta(0)
{
	ui = new Ui::MyWidget();
	ui->setupUi(this);
	ui->label->setText("");
	ui->label->setStyleSheet("QLabel{border:1px solid rgb(0, 0, 0);}");
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

		//判断鼠标是否是右键按下,且鼠标位置是否在绘画区域
		if (mouse->button() == Qt::RightButton &&ui->label->geometry().contains(mouse->pos()))
		{
			press = true;
			QApplication::setOverrideCursor(Qt::OpenHandCursor); //设置鼠标样式

			PreDot = mouse->pos();
		}
		// 判断鼠标是否是左键按下,且鼠标位置是否在绘画区域，是的话就记录目标点然后设置坐标线
		 else if(mouse->button() == Qt::LeftButton &&ui->label->geometry().contains(mouse->pos()))
		 {
			
		 	targetPoint = mouse->pos();
		 	action = SelectPoint;
		 	this->update();
		 }
		 else if(mouse->button() == Qt::MiddleButton && ui->label->geometry().contains(mouse->pos()))
		 {
			targetPoint = mouse->pos();
			action = AddGrowPoint;
			this->update();
		 }
	}
	else if (event->type() == QEvent::MouseButtonRelease)
	{
		QMouseEvent *mouse = dynamic_cast<QMouseEvent*>(event);

		//判断鼠标是否是右键释放,且之前是在绘画区域
		if (mouse->button() == Qt::RightButton && press)
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
	if(event->modifiers() == Qt::ControlModifier) // 按住了Ctrl键
	{
		if(event->delta() > 0)  //上滑,缩小
		{
			action = Shrink;
			this->update();
		}
		else   //下滑,放大
		{
			action = Amplification;
			this->update();
		}
	}
	else
	{
		wheelDelta = event->delta();
		action = ChangeSliderValue;
		this->update();
	}

	event->accept();
}

bool MyWidget::loadPicture(const QString picPath)
{
	// img->save(picPath); // 权宜之计，把图片保存到硬盘再读入。因为直接把QImage转为QPixmap得到的是空图像。


	picturePath = picPath;
	pix = new QPixmap;
	if(pix->load(picturePath))
	{
		pixW = pix->width();
		pixH = pix->height();
		imagesLoaded = true;
		return true;
	}
	else
	{
		return false;
	}
}

void MyWidget::setDrawCoordinateXY(int x, int y)
{
	drawCoordinateX = x;
	drawCoordinateY = y;
}

void MyWidget::drawCoordinatesLines(QImage & img)
{
	int width = img.width();
	int height = img.height();
	QColor c(0, 0, 255);
	int currentDrawCoordinateX = drawCoordinateX * ratio;
	int currentDrawCoordinateY = drawCoordinateY * ratio;
	for (int i = 0; i < height; i = i + 3)
	{
		img.setPixelColor(currentDrawCoordinateX, i, c);
	}
	for (int i = 0; i < width; i = i + 3)
	{
		img.setPixelColor(i, currentDrawCoordinateY, c);
	}
}

void MyWidget::paintEvent(QPaintEvent *event)
{
	if (!imagesLoaded) return;
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
	int dx = 0;
	if (x < 0)
	{
		dx = x;
		x = 0;
	}

	int y = ui->label->height() / 2 + Alloffset.y() - NowH / 2;
	int dy = 0;
	if (y < 0)
	{
		dy = y;
		y = 0;
	}
		

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

	// 响应鼠标滚轮改变滑动条
	if(action == ChangeSliderValue)
	{
		emit emitWheelDelta(wheelDelta);
	}

	if(action == AddGrowPoint)
	{
		int x = targetPoint.rx();
		int y = targetPoint.ry();
		// std::string text = std::to_string(x) + " " + std::to_string(y);
		// QMessageBox::information(NULL, "Title", text.c_str(), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	}

	// 绘制坐标线
	QRect picArea(QPoint(x + ui->label->x(), y + ui->label->y()), QSize(w, h));
	if(action == SelectPoint && picArea.contains(targetPoint))
	{
		emit emitSelectedX((targetPoint.rx() - (x + ui->label->x()) - dx) / ratio);
		emit emitSelectedY((targetPoint.ry() - (y + ui->label->y()) - dy) / ratio);
	}
		

	// QImage image(pix->toImage().convertToFormat(QImage::Format_Grayscale8));
	QImage image(pix->toImage());
	drawCoordinatesLines(image);
	pix->fromImage(image);
	QPixmap m = QPixmap::fromImage(image);

	// painter.drawRect(ui->label->x() - 1, ui->label->y() - 1, ui->label->width() + 1, ui->label->height() + 1); //画框
	// painter.drawTiledPixmap(x + ui->label->x(), y + ui->label->y(), w, h, *pix, sx, sy);             //绘画图形
	painter.drawTiledPixmap(x + ui->label->x(), y + ui->label->y(), w, h, m, sx, sy);             //绘画图形
}