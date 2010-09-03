#ifndef _PALATTE_H_
#define _PALETTE_H_

#include <vector>
#include <QColor>

class Palette
{
	public:
		Palette();
		~Palette();

		inline double getMinValue() const { return values.at(0); }
		inline double getMaxValue() const { return values.at(values.size()-1); }
		void addColor(double value, QColor color);
		QColor getColorNormalized(double value) const;
		QColor getColor(double value) const;

		void deleteColorById(unsigned int id);
		QColor* getColorById(unsigned int id);
		unsigned int getNumberOfColors() const;
		double* getValueById(unsigned int id);

	private:
		std::vector<QColor*> colors;
		std::vector<double> values;
};

#endif