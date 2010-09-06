#include "PaletteWidget.h"

#include <QStyle>
#include <QIntValidator>
#include <QDesktopWidget>

PaletteWidget::PaletteWidget(Palette* palette, QWidget* parent)
: QWidget(parent)
{
	this->palette = palette;

	// set window title
	setWindowTitle(tr("Palette"));

	// center on screen
	QDesktopWidget desktopWidget;
	QRect desktopRect(desktopWidget.availableGeometry(desktopWidget.primaryScreen()));
	QRect widgetRect(rect());
	move(desktopRect.center() - widgetRect.center());

	listWidget = new QListWidget;
	connect(listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(listClicked(QListWidgetItem *)));

	colorLabel = new QLabel;
	colorLabel->setText("Color:");

	colorWidget = new ColorWidget;
	colorWidget->setEnabled(false);

	valueLabel = new QLabel;
	valueLabel->setText("Value:");

	valueLineEdit = new QLineEdit;
	valueLineEdit->setValidator(new QIntValidator);
	valueLineEdit->setEnabled(false);

	okButton = new QPushButton;
	okButton->setText(tr("&OK"));
	okButton->setEnabled(false);
	connect(okButton, SIGNAL(clicked()), this, SLOT(clickedOK()));
	
	deleteButton = new QPushButton;
	deleteButton->setText(tr("&Delete"));
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(clickedDelete()));

	newButton = new QPushButton;
	newButton->setText(tr("&New"));
	connect(newButton, SIGNAL(clicked()), this, SLOT(clickedNew()));

	mainLayout = new QGridLayout;
	mainLayout->addWidget(listWidget,0,0,3,2);
	mainLayout->addWidget(valueLabel,0,2);
	mainLayout->addWidget(valueLineEdit,0,3);
	mainLayout->addWidget(colorLabel,1,2);
	mainLayout->addWidget(colorWidget,1,3);
	mainLayout->addWidget(okButton,2,2,1,2);
	mainLayout->addWidget(deleteButton,3,1);
	mainLayout->addWidget(newButton,3,0);

	setLayout(mainLayout);
	updateList();
}

PaletteWidget::~PaletteWidget()
{

}

void PaletteWidget::updateList()
{
	listWidget->clear();

	unsigned int count = palette->getNumberOfColors();
	unsigned int value;
	value = palette->getFirstValue();
	while (count > 0) {
		new QListWidgetItem(QString("%1: 0x%2%3%4 %5").arg(value).arg(palette->getColorByValue(value).red(),2,16,QLatin1Char('0')).arg(palette->getColorByValue(value).blue(),2,16,QLatin1Char('0')).arg(palette->getColorByValue(value).green(),2,16,QLatin1Char('0')).arg(palette->getColorByValue(value).alphaF()).toUpper(), listWidget, value);
		count--;
		value = palette->getNextValue(value);
	}
}

void PaletteWidget::clickedOK()
{
	unsigned int value = listWidget->currentItem()->type();	
	palette->deleteColorByValue(value);
	palette->addColor(valueLineEdit->text().toInt(), colorWidget->getColor());
	valueLineEdit->setEnabled(false);
	valueLineEdit->setText(QString(""));
	colorWidget->setEnabled(false);
	okButton->setEnabled(false);
	updateList();
	emit paletteUpdated();
}

void PaletteWidget::clickedDelete()
{
	unsigned int value = listWidget->currentItem()->type();
	palette->deleteColorByValue(value);
	valueLineEdit->setEnabled(false);
	valueLineEdit->setText(QString(""));
	colorWidget->setEnabled(false);
	updateList();
	emit paletteUpdated();
}

void PaletteWidget::clickedNew()
{	
	palette->addColor(0,QColor(0xFF,0xFF,0xFF,0xFF));
	updateList();
	emit paletteUpdated();
}

void PaletteWidget::listClicked(QListWidgetItem* item)
{
	unsigned int value = item->type();

	valueLineEdit->setText(QString("%1").arg(value));
	colorWidget->setColor(palette->getColorByValue(value));
	okButton->setEnabled(true);
	valueLineEdit->setEnabled(true);
	colorWidget->setEnabled(true);
}