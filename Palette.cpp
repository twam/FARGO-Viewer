#include "Palette.h"
#include <stdio.h>

Palette::Palette()
{
	
}

Palette::~Palette()
{
	
}

QColor Palette::getColorNormalized(double value) const
{
	return getColor(getMinValue()+value*(getMaxValue()-getMinValue()));
}

QColor Palette::getColor(double value) const
{
	unsigned int pos = 0;
	QColor ret;

	while ((pos < values.size()) && (values.at(pos) < value)) {
		pos++;
	}

	if (pos == 0) {
		ret = *colors.at(0);
	} else if (pos == values.size()) {
		ret = *colors.at(pos-1);
	} else {
		double factor = (value-values.at(pos-1))/(values.at(pos)-values.at(pos-1));

		ret.setRedF((1-factor)*colors.at(pos-1)->redF()+(factor)*colors.at(pos)->redF());
		ret.setGreenF((1-factor)*colors.at(pos-1)->greenF()+(factor)*colors.at(pos)->greenF());
		ret.setBlueF((1-factor)*colors.at(pos-1)->blueF()+(factor)*colors.at(pos)->blueF());
		ret.setAlphaF((1-factor)*colors.at(pos-1)->alphaF()+(factor)*colors.at(pos)->alphaF());
	}

	return ret;
}

void Palette::addColor(double value, QColor color)
{
	unsigned int pos = 0;
	
	while ((pos < values.size()) && (values.at(pos) < value)) {
		pos++;
	}

	values.insert(values.begin()+pos,value);
	
	QColor *colorPointer = new QColor;
	*colorPointer = color;
	colors.insert(colors.begin()+pos,colorPointer);
}

void Palette::deleteColorById(unsigned int id)
{
	colors.erase(colors.begin()+id);
	values.erase(values.begin()+id);
}

QColor* Palette::getColorById(unsigned int id)
{
	return colors.at(id);
}

unsigned int Palette::getNumberOfColors() const
{
	return colors.size();
}

double* Palette::getValueById(unsigned int id)
{
	return &values.at(id);
}