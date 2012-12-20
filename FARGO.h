#ifndef _FARGO_H_
#define _FARGO_H_

#include "Simulation.h"

class FARGO : public Simulation
{
	Q_OBJECT

	public:
		enum Version {
			FARGO_ORIGINAL,
			FARGO_TWAM,
			N_VERSION
		};

		FARGO();
		~FARGO();
		int loadFromFile(const char* filename);

		// planet stuff
		unsigned int getNumberOfPlanets() const;
		const double* getPlanetPosition(unsigned int number) const;
		const double* getPlanetVelocity(unsigned int number) const;
		const double* getPlanetMass(unsigned int number) const;
		const double* getPlanetRadius(unsigned int number) const;

		// particle stuff
		unsigned int getNumberOfParticles() const;
		const double* getParticlePosition(unsigned int number) const;
		const double* getParticleVelocity(unsigned int number) const;
		const double* getParticleMass(unsigned int number) const;
		bool getHasParticles() const;

		int loadTimestep(unsigned int timestep);
		unsigned int getCurrentTimestep() const;
		unsigned int getLastTimeStep() const;
		unsigned int getNRadial() const;
		unsigned int getNAzimuthal() const;
		double getRMin() const;
		double getRMax() const;
		const double* getRadii() const;
		const double* getQuantity() const;
		void setQuantityType(Simulation::QuantityType type);

		double getMinimumValue(void) const;
		double getMaximumValue(void) const;

	private:
		Version version;

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

		// planests
		unsigned int NPlanets;
		double* planetPositions;
		double* planetVelocities;
		double* planetMasses;
		double* planetRadii;

		// particles
		bool HasParticles;
		unsigned int NParticles;
		double *particlePositions;
		double *particleVelocities;
		double *particleMasses;


		double* radii;
		double* quantity;

		int loadGrid(double* dest, const char* filename, bool scalar);

	signals:
		void dataUpdated();
};

#endif
