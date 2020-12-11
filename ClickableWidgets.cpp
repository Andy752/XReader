#include "ClickableWidgets.h"
#include "ui_ClickableWidgets.h"

ClickableWidgets::ClickableWidgets(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::ClickableWidgets();
	ui->setupUi(this);
}

ClickableWidgets::~ClickableWidgets()
{
	delete ui;
}
