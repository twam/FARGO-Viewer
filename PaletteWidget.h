#ifndef _PALETTEWIDGET_H_
#define _PALETTEWIDGET_H_

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include "Palette.h"
#include "ColorWidget.h"

class PaletteWidget : public QWidget
{
	Q_OBJECT

	public:
		PaletteWidget(Palette* palette, QWidget* parent = 0);
		~PaletteWidget();

	public slots:
		void clickedOK();
		void clickedDelete();
		void clickedNew();
		void listClicked(QListWidgetItem *);

	private:
		Palette* palette;

		QGridLayout* mainLayout;

		QPushButton* okButton;
		QPushButton* deleteButton;
		QPushButton* newButton;

		QLabel* valueLabel;
		QLabel* colorLabel;
		QLineEdit* valueLineEdit;
		ColorWidget* colorWidget;

		QListWidget* listWidget;

		void updateList();

	signals:
		void paletteUpdated();
};

#endif