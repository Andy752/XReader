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

		//�ж�����Ƿ����Ҽ�����,�����λ���Ƿ��ڻ滭����
		if (mouse->button() == Qt::RightButton &&ui->label->geometry().contains(mouse->pos()))
		{
			press = true;
			QApplication::setOverrideCursor(Qt::OpenHandCursor); //���������ʽ

			PreDot = mouse->pos();
		}
		// �ж�����Ƿ����������,�����λ���Ƿ��ڻ滭�����ǵĻ��ͼ�¼Ŀ���Ȼ������������
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

		//�ж�����Ƿ����Ҽ��ͷ�,��֮ǰ���ڻ滭����
		if (mouse->button() == Qt::RightButton && press)
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
	if(event->modifiers() == Qt::ControlModifier) // ��ס��Ctrl��
	{
		if(event->delta() > 0)  //�ϻ�,��С
		{
			action = Shrink;
			this->update();
		}
		else   //�»�,�Ŵ�
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
	// img->save(picPath); // Ȩ��֮�ƣ���ͼƬ���浽Ӳ���ٶ��롣��Ϊֱ�Ӱ�QImageתΪQPixmap�õ����ǿ�ͼ��


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

	// ��Ӧ�����ָı们����
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

	// ����������
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

	// painter.drawRect(ui->label->x() - 1, ui->label->y() - 1, ui->label->width() + 1, ui->label->height() + 1); //����
	// painter.drawTiledPixmap(x + ui->label->x(), y + ui->label->y(), w, h, *pix, sx, sy);             //�滭ͼ��
	painter.drawTiledPixmap(x + ui->label->x(), y + ui->label->y(), w, h, m, sx, sy);             //�滭ͼ��
}