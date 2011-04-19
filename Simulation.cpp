#include "Simulation.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

Simulation::Simulation()
{
	configFilename = NULL;
	outputDirectory = NULL;
	planetConfigFilename = NULL;
	NPlanets = 0;
	readGhostCells = false;
	quantityType = DENSITY;
	quantity = NULL;
}

Simulation::~Simulation()
{
	delete [] configFilename;
	delete [] outputDirectory;
	delete [] planetConfigFilename;

	free(planetPositions);
	free(planetVelocities);
	free(planetMasses);

	delete [] radii;
	if (quantity != NULL) 
		delete [] quantity;
}

int Simulation::loadFromFile(const char* filename)
{
	char buffer[512];
	FILE *fd;
	
	configFilename = new char[strlen(filename)+1];
	strcpy(configFilename, filename);

	if (config::read_config_from_file(filename) == -1) {
		fprintf(stderr, "Cannot read config file '%s'!\n", filename);
		return -1;
	}

	rMin = config::value_as_double("Rmin");
	rMax = config::value_as_double("Rmax");
	totalTimestep = config::value_as_unsigned_int("Ntot");
	NRadial = config::value_as_unsigned_int("Nrad");
	NAzimuthal = config::value_as_unsigned_int("Nsec");
	currentTimestep = 0;
	
	if (!readGhostCells) {
		NRadial-=2;
	}

	char *temp = new char[strlen(filename)];
	strncpy(temp, filename, (strrchr(filename,'/')-filename-2));
	temp[(strrchr(filename,'/')-filename-2)] = 0;

	outputDirectory = new char[strlen(temp)+strlen(config::value_as_string("OutputDir"))+1];
	sprintf(outputDirectory,"%s%s",temp,config::value_as_string("OutputDir"));

	if ((config::key_exists("PLANETCONFIG")) && (strlen(config::value_as_string("PLANETCONFIG"))>0)) {
		planetConfigFilename = new char[strlen(temp)+strlen(config::value_as_string("PLANETCONFIG"))+1];
		sprintf(planetConfigFilename,"%s%s",temp, config::value_as_string("PLANETCONFIG"));
	} else {
		planetConfigFilename = NULL;
	}
	
	delete [] temp;

	config::clear_config();

	// load radii
	temp = new char[strlen(outputDirectory)+1+13];
	sprintf(temp, "%s/used_rad.dat", outputDirectory);

	fd = fopen(temp, "r");
	if (fd == NULL) {
		fprintf(stderr, "Cannot read radii file '%s'!\n", temp);
		return -1;
	}
	radii = new double[NRadial+1];

	if (!readGhostCells) {
		if (fgets(buffer, sizeof(buffer), fd) == NULL) {
			fprintf(stderr,"Not enough radii in radii file!\n");
			return -3;
		}
	}
	for (unsigned int i = 0; i <= NRadial; i++) {
		if (fgets(buffer, sizeof(buffer), fd) == NULL) {
			fprintf(stderr,"Not enough radii in radii file!\n");
			return -3;
		}
		sscanf(buffer, "%lf", &radii[i]);
	}

	quantity = new double[(NRadial + 1)*NAzimuthal];

	// load planets
	NPlanets = 1;

	planetPositions = (double*)malloc(3*sizeof(double));
	planetVelocities = (double*)malloc(3*sizeof(double));
	planetMasses = (double*)malloc(1*sizeof(double));
	// planet 0 is sun, positon (0,0,0)
	planetPositions[0] = 0.0;
	planetPositions[1] = 0.0;
	planetPositions[2] = 0.0;
	planetVelocities[0] = 0.0;
	planetVelocities[1] = 0.0;
	planetVelocities[2] = 0.0;
	planetMasses[0] = 1.0;

	if (planetConfigFilename != NULL) {
		fd = fopen(planetConfigFilename, "r");

		if (fd == NULL) {
			fprintf(stderr, "Error : can't find '%s'.\n", planetConfigFilename);
			return -1;
		}
		// read line by line
		while (fgets(buffer, sizeof(buffer), fd) != NULL) {
			char name[80], feeldisk[5], feelother[5];
			double semi_major_axis, mass, acc, eccentricity;

			// try to cut line into pieces
			sscanf(buffer, "%80s %lf %lf %lf %5s %5s %lf", name, &semi_major_axis, &mass, &acc, feeldisk, feelother, &eccentricity);

			// check if this line is a comment
			if (name[0] == '#')
				continue;

			NPlanets++;

			planetPositions = (double*)realloc(planetPositions, NPlanets*3*sizeof(double));
			planetVelocities = (double*)realloc(planetVelocities, NPlanets*3*sizeof(double));
			planetMasses = (double*)realloc(planetMasses, NPlanets*1*sizeof(double));

			// this is not really needed (just load timestep 0);
			planetPositions[(NPlanets-1)*3+0] = semi_major_axis*(1.0+eccentricity);
			planetPositions[(NPlanets-1)*3+1] = 0;
			planetPositions[(NPlanets-1)*3+2] = 0;
			planetVelocities[(NPlanets-1)*3+0] = 0;
			planetVelocities[(NPlanets-1)*3+1] = sqrt(1*(1.0+mass)/semi_major_axis)*sqrt( (1.0-eccentricity)/(1.0+eccentricity));
			planetVelocities[(NPlanets-1)*3+2] = 0;
			planetMasses[(NPlanets-1)] = mass;
		}

		fclose(fd);
	}

	// check for last timestep
	temp = new char[strlen(outputDirectory)+1+15];
	sprintf(temp, "%s/Quantities.dat", outputDirectory);
	fd = fopen(temp, "r");
	if (fd == NULL) {
		fprintf(stderr, "Cannot read file '%s'!\n", temp);
		return -1;
	}

	unsigned int line = 0;
	while (fgets(buffer, sizeof(buffer), fd) != NULL) {
		line++;
	}
	totalTimestep = line-2;

	delete[] temp;
	fclose(fd);

	loadTimestep(0);

	emit dataUpdated();

	return 0;
}

int Simulation::loadTimestep(unsigned int timestep)
{
	// TODO: On errors, memory leakage occurs!
	int ret = 0;

	FILE *fd;
	char *filename;

	double* newPlanetPositions = (double*)malloc(NPlanets*3*sizeof(double));
	double* newPlanetVelocities = (double*)malloc(NPlanets*3*sizeof(double));

	for (unsigned int i = 1; i < NPlanets; ++i) {
		filename = new char[strlen(outputDirectory)+1+10+(unsigned int)(log(NPlanets)/log(10)+1)];
		sprintf(filename, "%splanet%u.dat", outputDirectory, i);


		fd = fopen(filename,"r");
		if (fd == NULL) {
			fprintf(stderr, "Could not open '%s'.\n", filename);
			return -1;
		}
		
		char buffer[1024];
		for (unsigned int l = 0; l <= timestep; ++l) {
			if (fgets(buffer, 1024, fd) == NULL) {
				return -1;
			}
		}
		
		unsigned int timestepFile;
		sscanf(buffer, "%u %lf %lf %lf %lf", &timestepFile, &newPlanetPositions[i*3+0], &newPlanetPositions[i*3+1], &newPlanetVelocities[i*3+0], &newPlanetVelocities[i*3+1]);

		if (timestepFile != timestep) {
			fprintf(stderr, "Timestep does not matched with file: %u != %u\n", timestep, timestepFile);
			fprintf(stderr, "Line was '%s'\n",buffer);
			return -3;
		}

		delete [] filename;
		fclose(fd);
	}

	// read grid
	switch (quantityType) {
		case DENSITY:
			filename = new char[strlen(outputDirectory)+1+12+(unsigned int)(log(timestep)/log(10)+1)];
			sprintf(filename, "%sgasdens%u.dat", outputDirectory, timestep);
			ret = loadGrid(quantity, filename, true);
			break;

		case TEMPERATURE:
			filename = new char[strlen(outputDirectory)+1+19+(unsigned int)(log(timestep)/log(10)+1)];
			sprintf(filename, "%sgasTemperature%u.dat", outputDirectory, timestep);
			ret = loadGrid(quantity, filename, true);
			break;

		case V_RADIAL:
			filename = new char[strlen(outputDirectory)+1+19+(unsigned int)(log(timestep)/log(10)+1)];
			sprintf(filename, "%sgasvrad%u.dat", outputDirectory, timestep);
			ret = loadGrid(quantity, filename, false);
			break;

		case V_AZIMUTHAL:
			filename = new char[strlen(outputDirectory)+1+19+(unsigned int)(log(timestep)/log(10)+1)];
			sprintf(filename, "%sgasvtheta%u.dat", outputDirectory, timestep);
			ret = loadGrid(quantity, filename, false);
			break;

		default:
			break;
	}

	if (ret == 0) {
		double* temp;

		// copy planet positions
		temp = planetPositions;
		planetPositions = newPlanetPositions;
		free(temp);

		temp = planetVelocities;
		planetVelocities = newPlanetVelocities;
		free(temp);
	
		currentTimestep = timestep;

		emit dataUpdated();
	}

	return ret;
}

/**
	reads a two-dimensional FARGO polargrid

	\param dest destination
	\param filename filename to read
	\param scalar is this a scalar or vector grid
*/
int Simulation::loadGrid(double* dest, const char* filename, bool scalar)
{
	FILE *fd = fopen(filename, "rb");
	if (fd == NULL) {
		fprintf(stderr, "Could not open '%s'!\n", filename);
		return -1;
	}

	double* buffer = new double[(scalar ? NRadial : NRadial + 1)*NAzimuthal];

	// if we don't want to read ghost cells, skip the first NAzimuthal datapoints in file
	if (!readGhostCells) {
		if (fread(buffer, sizeof(double), NAzimuthal, fd)<NAzimuthal) {
			fprintf(stderr, "Error while reading '%s' (%lu bytes).\n", filename,NAzimuthal*sizeof(double));
			goto loadGrid_cleanUp;
		}
	}

	if (scalar) {
		if (fread(buffer, sizeof(double), NRadial*NAzimuthal, fd)<NRadial*NAzimuthal) {
			fprintf(stderr, "Error while reading '%s' (%lu bytes).\n", filename,NRadial*NAzimuthal*sizeof(double));
			goto loadGrid_cleanUp;
		}

		// interpolate
		for (unsigned int nRadial = 0; nRadial <= NRadial; ++nRadial) {
			for (unsigned int nAzimuthal = 0; nAzimuthal < NAzimuthal; ++nAzimuthal) {
				unsigned int index = nRadial*NAzimuthal + nAzimuthal;

				if (nRadial == 0) {
					dest[index] = 0.5*(buffer[index]+buffer[nAzimuthal == 0 ? index + NAzimuthal -1: index-1]);
				} else if (nRadial == NRadial) {
					dest[index] = 0.5*(buffer[index-NAzimuthal]+buffer[nAzimuthal == 0 ? index-NAzimuthal+NAzimuthal-1 : index-NAzimuthal-1]);
				} else {
					dest[index] = 0.25*(buffer[index]+buffer[nAzimuthal == 0 ? index + NAzimuthal -1: index-1]+buffer[index-NAzimuthal]+buffer[nAzimuthal == 0 ? index-NAzimuthal+NAzimuthal-1 : index-NAzimuthal-1]);
				}
			}
		}
	} else {
		if (fread(dest, sizeof(double), (NRadial+1)*NAzimuthal, fd)<(NRadial+1)*NAzimuthal) {
			fprintf(stderr, "Error while reading '%s' (%lu bytes).\n", filename,(NRadial+1)*NAzimuthal*sizeof(double));
			goto loadGrid_cleanUp;
		}		
	}

loadGrid_cleanUp:
	delete [] buffer;
	fclose(fd);

	return 0;
}

void Simulation::setQuantityType(QuantityType type) {
	if (quantityType != type) {
		quantityType = type;
		loadTimestep(currentTimestep);
	}
}


double Simulation::getMinimumValue(void) {
	double minimum = DBL_MAX;

	if (quantity != NULL) {
		for (unsigned int i = 0; i < (NRadial + 1)*NAzimuthal; ++i) {
			if (quantity[i] < minimum) {
				minimum = quantity[i];
			}
		}
	}

	return minimum;
}

double Simulation::getMaximumValue(void) {
	double maximum = -DBL_MAX;

	if (quantity != NULL) {
		for (unsigned int i = 0; i < (NRadial + 1)*NAzimuthal; ++i) {
			if (quantity[i] > maximum) {
				maximum = quantity[i];
			}
		}
	}
	
	return maximum;
}
