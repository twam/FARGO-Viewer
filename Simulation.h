#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include <QObject>

class Simulation : public QObject
{
	Q_OBJECT

	public:

		enum QuantityType {
			DENSITY,
			TEMPERATURE,
			V_RADIAL,
			V_AZIMUTHAL,
			N_QUANTITY_TYPES
		};

		Simulation();
		virtual ~Simulation();

		virtual int loadFromFile(const char* filename) = 0;
		virtual unsigned int getNumberOfPlanets() const = 0;
		virtual const double* getPlanetPosition(unsigned int number) const = 0;
		virtual const double* getPlanetVelocity(unsigned int number) const = 0;
		virtual const double* getPlanetMass(unsigned int number) const = 0;
		virtual const double* getPlanetRadius(unsigned int number) const = 0;
		virtual int loadTimestep(unsigned int timestep) = 0;
		virtual unsigned int getCurrentTimestep() const = 0;
		virtual unsigned int getLastTimeStep() const = 0;
		virtual unsigned int getNRadial() const = 0;
		virtual unsigned int getNAzimuthal() const = 0;
		virtual double getRMin() const = 0;
		virtual double getRMax() const = 0;
		virtual const double* getRadii() const = 0;
		virtual const double* getQuantity() const = 0;
		virtual void setQuantityType(QuantityType type) = 0;

		virtual double getMinimumValue(void) const = 0;
		virtual double getMaximumValue(void) const = 0;

	private:

	signals:
		void dataUpdated();
};

#endif
