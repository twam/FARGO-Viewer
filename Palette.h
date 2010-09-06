#ifndef _PALETTE_H_
#define _PALETTE_H_

#include <vector>
#include <QColor>
#include <QMap>

class Palette
{
	public:
		Palette();
		~Palette();

		inline unsigned int getMinValue() const { return colorMap.isEmpty() ? 0 : colorMap.keys().first(); }
		inline unsigned int getMaxValue() const { return colorMap.isEmpty() ? 0 : colorMap.keys().last(); }
		void addColor(unsigned int value, const QColor& color);
		QColor getColorNormalized(double value) const;
		QColor getColor(double value) const;

		void deleteColorByValue(unsigned int value);
		const QColor& getColorByValue(unsigned int value) const;
		unsigned int getNumberOfColors() const;
		void clear();
		unsigned int getFirstValue();
		unsigned int getNextValue(unsigned int prevValue);

	private:
		QMap<unsigned int, QColor> colorMap;
};

#endif