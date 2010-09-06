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
	QMap<unsigned int, QColor>::const_iterator pos = colorMap.begin();
	QColor ret;

	while ((pos != colorMap.end()) && (pos.key() < value)) {
		pos++;
	}

	if ((colorMap.size() == 1) || (pos == colorMap.begin())) {
		ret = pos.value();
	} else if (pos == colorMap.end()) {
		ret = pos.value();
	} else {
		double factor = (value-(pos-1).key())/((pos.key())-((pos-1).key()));

		ret.setRedF((1-factor)*(pos-1).value().redF()+(factor)*pos.value().redF());
		ret.setGreenF((1-factor)*(pos-1).value().greenF()+(factor)*pos.value().greenF());
		ret.setBlueF((1-factor)*(pos-1).value().blueF()+(factor)*pos.value().blueF());
		ret.setAlphaF((1-factor)*(pos-1).value().alphaF()+(factor)*pos.value().alphaF());
	}

	return ret;
}

void Palette::addColor(unsigned int value, const QColor& color)
{
	colorMap.insert(value, color);
}

void Palette::deleteColorByValue(unsigned int value)
{
	colorMap.erase(colorMap.find(value));
}

const QColor& Palette::getColorByValue(unsigned int value) const
{
	return colorMap.find(value).value();
}

unsigned int Palette::getNumberOfColors() const
{
	return colorMap.size();
}

unsigned int Palette::getFirstValue()
{
	if (colorMap.isEmpty()) {
		return 0;
	} else {
		return colorMap.find(colorMap.keys().first()).key();
	}
}

unsigned int Palette::getNextValue(unsigned int prevValue)
{
	if ((colorMap.isEmpty()) || (prevValue == colorMap.find(colorMap.keys().last()).key())) {
		return 0;
	} else {
		return colorMap.keys().at(colorMap.keys().indexOf(prevValue)+1);
	}
}

void Palette::clear()
{
	colorMap.clear();
}