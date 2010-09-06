#ifndef _COLORWIDGET_H_
#define _COLORWIDGET_H_

#include <QWidget>
#include <QColor>

class ColorWidget : public QWidget
{
	Q_OBJECT

	private:
		QColor color;
		bool enabled;

	protected:
		void paintEvent(QPaintEvent* event);
		void mousePressEvent(QMouseEvent* event);

	public:
		ColorWidget(QWidget* parent = 0);
		~ColorWidget();

		inline const QColor& getColor() const { return color; }
		inline void setColor(const QColor& c) { color = c; update(); }
		inline bool getEnabled() const { return enabled; }
		inline void setEnabled(bool value) { enabled = value; update(); }

	signals:
		void colorChanged();
};

#endif