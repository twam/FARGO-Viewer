#ifndef _OPENGLWIDGET_H_
#define _OPENGLWIDGET_H_

#include "OpenGLNavigationWidget.h"
#include "Simulation.h"
#include "Palette.h"
#include "RocheLobe.h"
#include "Vector.h"
#include "Matrix.h"

class OpenGLWidget : public OpenGLNavigationWidget
{
	Q_OBJECT

	public:
		OpenGLWidget(QWidget *parent);
		~OpenGLWidget();
		void setSimulation(Simulation *simulation);
		
		inline Palette* getPalette() { return palette; }
		
		void setLogarithmic(bool value);
		inline bool getLogarithmic() const { return logarithmicScale; }
		void setMinimumValue(double value);
		void setMaximumValue(double value);
		inline double getMinimumValue() const { return minimumValue; }
		inline double getMaximumValue() const { return maximumValue; }

	public slots:
		void updateShowDisk(bool value);
		void updateShowGrid(bool value);
		void updateShowDiskBorder(bool value);
		void updateShowPlanets(bool value);
		void updateShowOrbits(bool value);
		void updateShowRocheLobe(bool value);
		void updateShowSky(bool value);
		void updateShowText(bool value);
		void updateShowKey(bool value);
		void updateUseMultisampling(bool value);
		void updateSaveScreenshots(bool value);
		void updateFromGrid();

	protected:
		void initializeGL();
		void resizeGL(int width, int height);
		void paintGL();

	private:
		bool gridChanged;

		// disk
		bool showDisk;
		GLfloat* diskVertices;
		GLfloat* diskNormals;
		GLfloat* diskColors;
		GLuint* diskIndices;
		void initDisk();
		void renderDisk();
		void diskColor(GLfloat* color, double value, double minValue, double maxValue, bool logarithmic);

		// grid
		bool showGrid;
		void initGrid();
		void renderGrid();

		// disk border
		bool showDiskBorder;
		GLfloat* diskBorderVertices;
		unsigned int diskBorderDetailLevel;
		void initDiskBorder();
		void renderDiskBorder();

		// planets
		bool showPlanets;
		void renderPlanets();

		// orbits
		bool showOrbits;
		GLfloat* orbitsVertices;
		unsigned int orbitsDetailLevel;
		void initOrbits();
		void renderOrbits();

		// roche lobe
		bool showRocheLobe;
		GLfloat* rocheLobeVertices;
		unsigned int rocheLobeDetailLevel;
		void initRocheLobe();
		void renderRocheLobe();
		
		// sky
		bool showSky;
		double skyDistance;
		unsigned int skyNumberOfObjects;
		GLfloat* skyVertices;
		void initSky();
		void renderSky();

		// text
		bool showText;

		// key
		bool showKey;
		void renderKey();

		bool saveScreenshots;
		bool supportMultisampling;
		bool useMultisampling;
		Simulation* simulation;

		Palette* palette;
		double minimumValue;
		double maximumValue;
		bool logarithmicScale;
};

#endif
