#include "FARGO.h"
#include "config.h"
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <sys/stat.h>

FARGO::FARGO()
{
	version = FARGO_TWAM;
	configFilename = NULL;
	outputDirectory = NULL;
	planetConfigFilename = NULL;
	NPlanets = 0;
	readGhostCells = true;
	quantityType = DENSITY;
	quantity = NULL;
	radii = NULL;

	planetPositions = NULL;
	planetVelocities = NULL;
	planetMasses = NULL;
	planetRadii = NULL;

	particlePositions = NULL;
	particleVelocities = NULL;
	particleMasses = NULL;

	if (version == FARGO_TWAM) {
		readGhostCells = false;
	}
}

FARGO::~FARGO()
{
	free(configFilename);
	free(outputDirectory);
	free(planetConfigFilename);

	free(planetPositions);
	free(planetVelocities);
	free(planetMasses);
	free(planetRadii);

	free(particlePositions);
	free(particleVelocities);
	free(particleMasses);

	delete [] radii;

	if (quantity != NULL)
		delete [] quantity;
}

int FARGO::loadFromFile(const char* filename)
{
	char buffer[512];
	FILE *fd;

	free(configFilename);
	configFilename = (char*)malloc(1+strlen(filename));
	strcpy(configFilename, filename);

	if (config::read_config_from_file(configFilename) == -1) {
		fprintf(stderr, "Cannot read config file '%s'!\n", configFilename);
		return -1;
	}

	rMin = config::value_as_double("Rmin");
	rMax = config::value_as_double("Rmax");
	totalTimestep = config::value_as_unsigned_int("Ntot");
	NRadial = config::value_as_unsigned_int("Nrad");
	NAzimuthal = config::value_as_unsigned_int("Nsec");
	currentTimestep = 0;

	if (!readGhostCells) {
		NRadial -= 2;
	}

	// create a copy of filename as dirname may modify it
	char *configFilenameCopy = (char*)malloc(1+strlen(configFilename));
    strcpy(configFilenameCopy, configFilename);

    char *temp = dirname(configFilenameCopy);

    char *configDirname = (char*)malloc(1+strlen(temp));
    strcpy(configDirname, temp);

    // filename_copy is not needed anymore. releasing it may invalidate temp
    free(configFilenameCopy);
    temp = NULL;

    // create temporary output directory
    if (asprintf(&temp, "%s/%s", configDirname, config::value_as_string("OutputDir"))<0) {
    	fprintf(stderr, "Not enough memory.");
		exit(-1);
    }

    // create realpath of output directory
    outputDirectory = realpath(temp, NULL);

    // invalidate temp
    free(temp);

    // output directory could not be found!
    if (outputDirectory == NULL) {
    	// try again with ..
	    if (asprintf(&temp, "%s/../%s", configDirname, config::value_as_string("OutputDir"))<0) {
	    	fprintf(stderr, "Not enough memory.");
			exit(-1);
	    }

	    // create realpath of output directory
	    outputDirectory = realpath(temp, NULL);

	    // invalidate temp
	    free(temp);

	    if (outputDirectory == NULL) {
    		fprintf(stderr, "Output directory does not exist!");
    		exit(-1);
    	}
    }

	if ((config::key_exists("PLANETCONFIG")) && (strlen(config::value_as_string("PLANETCONFIG"))>0)) {
	    // create temporary planet config
	    if (asprintf(&temp, "%s/%s", configDirname, config::value_as_string("PLANETCONFIG"))<0) {
	    	fprintf(stderr, "Not enough memory.");
			exit(-1);
	    }

	    // create realpath of planet config file
	    planetConfigFilename = realpath(temp, NULL);

	    // invalidate temp
	    free(temp);

	    // output directory could not be found!
	    if (planetConfigFilename == NULL) {
	    	// try again with ..
		    if (asprintf(&temp, "%s/../%s", configDirname, config::value_as_string("PLANETCONFIG"))<0) {
		    	fprintf(stderr, "Not enough memory.");
				exit(-1);
		    }

		    // create realpath of output directory
		    planetConfigFilename = realpath(temp, NULL);

		    // invalidate temp
		    free(temp);
	    }
	} else {
		planetConfigFilename = NULL;
	}

	// load radii
	temp = new char[strlen(outputDirectory)+1+13];
	sprintf(temp, "%s/used_rad.dat", outputDirectory);

	fd = fopen(temp, "r");
	if (fd == NULL) {
		fprintf(stderr, "Cannot read radii file '%s'!\n", temp);
		delete [] temp;
		return -1;
	}
	delete [] temp;

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
	planetRadii = (double*)malloc(1*sizeof(double));
	// planet 0 is sun, positon (0,0,0)
	planetPositions[0] = 0.0;
	planetPositions[1] = 0.0;
	planetPositions[2] = 0.0;
	planetVelocities[0] = 0.0;
	planetVelocities[1] = 0.0;
	planetVelocities[2] = 0.0;
	planetMasses[0] = 1.0;
//	planetRadii[0] = config::value_as_double_default("StarRadius", 0.009304813);
	planetRadii[0] = config::value_as_double_default("StarRadius", 0.009304813*10);


	if (planetConfigFilename != NULL) {
		fd = fopen(planetConfigFilename, "r");

		if (fd == NULL) {
			fprintf(stderr, "Error : can't find '%s'.\n", planetConfigFilename);
			return -1;
		}
		// read line by line
		while (fgets(buffer, sizeof(buffer), fd) != NULL) {
			char name[80], feeldisk[5], feelother[5], irradiate[5];
			double semi_major_axis, mass, acc, eccentricity, radius, temperature, phi;

			// try to cut line into pieces
			int num_args = sscanf(buffer, "%80s %lf %lf %lf %5s %5s %lf %lf %lf %5s %lf", name, &semi_major_axis, &mass, &acc, feeldisk, feelother, &eccentricity, &radius, &temperature, irradiate, &phi);

			if (num_args < 8) {
				radius = 0.009304813*10;
			}

			// check if this line is a comment
			if (name[0] == '#')
				continue;

			NPlanets++;

			planetPositions = (double*)realloc(planetPositions, NPlanets*3*sizeof(double));
			planetVelocities = (double*)realloc(planetVelocities, NPlanets*3*sizeof(double));
			planetMasses = (double*)realloc(planetMasses, NPlanets*1*sizeof(double));
			planetRadii = (double*)realloc(planetRadii, NPlanets*1*sizeof(double));

			// this is not really needed (just load timestep 0);
			planetPositions[(NPlanets-1)*3+0] = semi_major_axis*(1.0+eccentricity);
			planetPositions[(NPlanets-1)*3+1] = 0.0;
			planetPositions[(NPlanets-1)*3+2] = 0.0;
			planetVelocities[(NPlanets-1)*3+0] = 0;
			planetVelocities[(NPlanets-1)*3+1] = sqrt(1*(1.0+mass)/semi_major_axis)*sqrt( (1.0-eccentricity)/(1.0+eccentricity));
			planetVelocities[(NPlanets-1)*3+2] = 0;
			planetMasses[(NPlanets-1)] = mass;
			planetRadii[(NPlanets-1)] = radius;
		}

		fclose(fd);
	}

	// load particles
	HasParticles = config::value_as_bool_default("IntegrateParticles", false);
	NParticles = config::value_as_unsigned_int_default("NumberOfParticles", 0);

	particlePositions = (double*)malloc(2*NParticles*sizeof(double));
	particleVelocities = (double*)malloc(2*NParticles*sizeof(double));
	particleMasses = (double*)malloc(NParticles*sizeof(double));

	// check for last timestep
	if (version == FARGO_TWAM) {
		temp = new char[strlen(outputDirectory)+1+12+(unsigned int)(log(totalTimestep)/log(10)+1)];

		struct stat buffer;
		unsigned int lastTimestep;
		for (unsigned int timestep = 1; timestep <= getLastTimeStep(); timestep++) {
			sprintf(temp, "%s/gasdens%u.dat", outputDirectory, timestep);
			if (stat(temp, &buffer) == 0) {
				lastTimestep = timestep;
			}
			else {
				break;
			}
		}

		totalTimestep = lastTimestep;
	}

	config::clear_config();

	loadTimestep(0);

	emit dataUpdated();

	return 0;
}

int FARGO::loadTimestep(unsigned int timestep)
{
	// TODO: On errors, memory leakage occurs!
	int ret = 0;

	FILE *fd;
	char *filename;
	bool foundTimestep = false;

	double* newPlanetPositions = (double*)malloc(NPlanets*3*sizeof(double));
	double* newPlanetVelocities = (double*)malloc(NPlanets*3*sizeof(double));

	newPlanetPositions[0] = 0.0;
	newPlanetPositions[1] = 0.0;
	newPlanetPositions[2] = 0.0;

	newPlanetVelocities[0] = 0.0;
	newPlanetVelocities[1] = 0.0;
	newPlanetVelocities[1] = 0.0;

	for (unsigned int i = 1; i < NPlanets; ++i) {
		filename = new char[strlen(outputDirectory)+1+10+(unsigned int)(log(NPlanets)/log(10)+1)];

		if (version == FARGO_TWAM) {
			sprintf(filename, "%s/planet%u.dat", outputDirectory, i);
		} else {
			sprintf(filename, "%s/planet%u.dat", outputDirectory, i-1);
		}

		fd = fopen(filename,"r");
		if (fd == NULL) {
			fprintf(stderr, "Could not open '%s'.\n", filename);
			delete [] filename;
			return -1;
		}
		delete [] filename;
		filename = NULL;

		while (!feof(fd)) {
			unsigned int timestepFile;
			if (fscanf(fd, "%u %lf %lf %lf %lf %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f",
					&timestepFile,
					&newPlanetPositions[i*3+0],
					&newPlanetPositions[i*3+1],
					&newPlanetVelocities[i*3+0],
					&newPlanetVelocities[i*3+1]
					)== EOF) {
				fprintf(stderr, "File ended unexpected\n");
				break;
			}

			if (timestepFile == timestep) {
				foundTimestep = true;
				break;
			}
		}
		fclose(fd);

		if (!foundTimestep) {
			fprintf(stderr, "Timestep %u was not in file!\n", timestep);
			return -3;
		}
	}

	// read particles
	if (HasParticles) {
		char *filename;
		if (asprintf(&filename, "%s/particles%u.dat", outputDirectory, timestep)<0) {
			fprintf(stderr, "Not enough memory!\n");
			exit(EXIT_FAILURE);
		}

		// get filesize
		struct stat filestatus;
  		stat(filename, &filestatus);

 		// update number of particles and array sizes
  		NParticles = filestatus.st_size/(9*8);
		particlePositions = (double*)realloc(particlePositions, 2*NParticles*sizeof(double));
		particleVelocities = (double*)realloc(particleVelocities, 2*NParticles*sizeof(double));
		particleMasses = (double*)realloc(particleMasses, NParticles*sizeof(double));

		FILE *fd = fopen(filename, "r");

		if (fd == NULL) {
			fprintf(stderr, "Could not open '%s'.\n", filename);
			return -1;
		}

		for (unsigned int i = 0; i < NParticles; ++i) {
			size_t count = 0;

			fseek(fd, 8, SEEK_CUR);
			count += fread(&particlePositions[i*2+0], sizeof(double), 1, fd);
			count += fread(&particlePositions[i*2+1], sizeof(double), 1, fd);
			count += fread(&particleVelocities[i*2+0], sizeof(double), 1, fd);
			count += fread(&particleVelocities[i*2+1], sizeof(double), 1, fd);
			count += fread(&particleMasses[i], sizeof(double), 1, fd);
			fseek(fd, 16+8, SEEK_CUR);

			if (count < 5) {
				fprintf(stderr, "File '%s' ended to early!\n", filename);
				exit(EXIT_FAILURE);
			}
		}

		free(filename);
		fclose(fd);
	}

	// read grid
	switch (quantityType) {
		case DENSITY:
			filename = new char[strlen(outputDirectory)+1+12+(unsigned int)(log(timestep)/log(10)+1)];
			sprintf(filename, "%s/gasdens%u.dat", outputDirectory, timestep);
			ret = loadGrid(quantity, filename, true);
			break;

		case TEMPERATURE:
			filename = new char[strlen(outputDirectory)+1+19+(unsigned int)(log(timestep)/log(10)+1)];
			sprintf(filename, "%s/gasTemperature%u.dat", outputDirectory, timestep);
			ret = loadGrid(quantity, filename, true);
			break;

		case V_RADIAL:
			filename = new char[strlen(outputDirectory)+1+19+(unsigned int)(log(timestep)/log(10)+1)];
			sprintf(filename, "%s/gasvrad%u.dat", outputDirectory, timestep);
			ret = loadGrid(quantity, filename, false);
			break;

		case V_AZIMUTHAL:
			filename = new char[strlen(outputDirectory)+1+19+(unsigned int)(log(timestep)/log(10)+1)];
			sprintf(filename, "%s/gasvtheta%u.dat", outputDirectory, timestep);
			ret = loadGrid(quantity, filename, false);
			break;

		default:
			break;
	}

	if (filename != NULL)
		delete [] filename;

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
int FARGO::loadGrid(double* dest, const char* filename, bool scalar)
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
		switch (version) {
			case FARGO_TWAM:
				if (fread(dest, sizeof(double), (NRadial+1)*NAzimuthal, fd)<(NRadial+1)*NAzimuthal) {
					fprintf(stderr, "Error while reading '%s' (%lu bytes).\n", filename,(NRadial+1)*NAzimuthal*sizeof(double));
					goto loadGrid_cleanUp;
				}
				break;

			default:
				if (fread(dest, sizeof(double), (NRadial)*NAzimuthal, fd)<(NRadial)*NAzimuthal) {
					fprintf(stderr, "Error while reading '%s' (%lu bytes).\n", filename,(NRadial)*NAzimuthal*sizeof(double));
					goto loadGrid_cleanUp;
				}
				break;
		}

	}

loadGrid_cleanUp:
	delete [] buffer;
	fclose(fd);

	return 0;
}

void FARGO::setQuantityType(QuantityType type) {
	if (quantityType != type) {
		quantityType = type;
		loadTimestep(currentTimestep);
	}
}


double FARGO::getMinimumValue(void) const {
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

double FARGO::getMaximumValue(void) const {
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

unsigned int FARGO::getNumberOfPlanets() const
{
	return NPlanets;
}

const double* FARGO::getPlanetPosition(unsigned int number) const
{
	return &planetPositions[number*3];
}

const double* FARGO::getPlanetVelocity(unsigned int number) const
{
	return &planetVelocities[number*3];
}

const double* FARGO::getPlanetMass(unsigned int number) const
{
	return &planetMasses[number];
}

const double* FARGO::getPlanetRadius(unsigned int number) const
{
	return &planetRadii[number];
}

unsigned int FARGO::getCurrentTimestep() const
{
	return currentTimestep;
}

unsigned int FARGO::getLastTimeStep() const
{
	return totalTimestep;
}

unsigned int FARGO::getNRadial() const {
	return NRadial;
}

unsigned int FARGO::getNAzimuthal() const {
	return NAzimuthal;
}

double FARGO::getRMin() const {
	return rMin;
}

double FARGO::getRMax() const {
	return rMax;
}

const double* FARGO::getRadii() const {
	return radii;
}

const double* FARGO::getQuantity() const {
	return quantity;
}

unsigned int FARGO::getNumberOfParticles() const {
	return NParticles;
}

const double* FARGO::getParticlePosition(unsigned int number) const {
	return &particlePositions[number*2];
}

const double* FARGO::getParticleVelocity(unsigned int number) const {
	return &particleVelocities[number*2];
}

const double* FARGO::getParticleMass(unsigned int number) const {
	return &particleMasses[number];
}

bool FARGO::getHasParticles() const {
	return HasParticles;
}
