#pragma once

#include <QWidget>
namespace Ui { class ClickableWidgets; };

class ClickableWidgets : public QWidget
{
	Q_OBJECT

public:
	ClickableWidgets(QWidget *parent = Q_NULLPTR);
	~ClickableWidgets();

private:
	Ui::ClickableWidgets *ui;
};
