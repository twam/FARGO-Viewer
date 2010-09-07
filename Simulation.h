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
			N_QUANTITY_TYPES
		};

		Simulation();
		~Simulation();
		int loadFromFile(const char* filename);
		inline unsigned int getNumberOfPlanets() { return NPlanets; }
		inline double* getPlanetPosition(unsigned int number) { return &planetPositions[number*3]; }
		inline double* getPlanetVelocity(unsigned int number) { return &planetVelocities[number*3]; }
		inline double* getPlanetMass(unsigned int number) { return &planetMasses[number]; }
		int loadTimestep(unsigned int timestep);
		inline unsigned int getCurrentTimestep() { return currentTimestep; }
		inline unsigned int getLastTimeStep() { return totalTimestep; }
		inline unsigned int getNRadial() { return NRadial; }
		inline unsigned int getNAzimuthal() { return NAzimuthal; }
		inline double getRMin() { return rMin; }
		inline double getRMax() { return rMax; }
		inline double* getRadii() { return radii; }
		inline const double* getQuantity() const { return quantity; }
		void setQuantityType(QuantityType type);

	private:
		QuantityType quantityType;
		bool readGhostCells;
		char *configFilename;
		char *planetConfigFilename;
		char *outputDirectory;
		unsigned int NRadial;
		unsigned int NAzimuthal;
		unsigned int totalTimestep;
		unsigned int currentTimestep;
		double rMin;
		double rMax;
		unsigned int NPlanets;
		double* planetPositions;
		double* planetVelocities;
		double* planetMasses;
		double* radii;
		double* quantity;

		void loadGrid(double* dest, const char* filename, bool scalar);

	signals:
		void dataUpdated();
};

#endif