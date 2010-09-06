#include "ColorWidget.h"
#include <QPainter>
#include <QColorDialog>
#include <QMouseEvent>

ColorWidget::ColorWidget(QWidget* parent)
: QWidget(parent)
{
	setFixedHeight(24);
}

ColorWidget::~ColorWidget()
{
	
}

void ColorWidget::paintEvent(QPaintEvent* /*event*/)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	
	painter.setPen(Qt::black);
	

	if (enabled) {
		QColor c = color;
		c.setAlpha(0xFF);
		painter.setBrush(c);
	} else {
		painter.setBrush(QBrush(Qt::NoBrush));
	}

	painter.drawRoundedRect(painter.viewport(), 4.0, 4.0);
}

void ColorWidget::mousePressEvent(QMouseEvent* event)
{
	if (enabled && (event->button() == Qt::LeftButton)) {
		color = QColorDialog::getColor(color, this, tr("Color"), QColorDialog::ShowAlphaChannel);
		emit colorChanged();
		update();
	}
}