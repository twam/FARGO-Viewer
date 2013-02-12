#ifndef _MAINWIDGET_H_
#define _MAINWIDGET_H_

#include <QWidget>
#include <QAction>
#include <QActionGroup>
#include <QSettings>
#include <QMenu>
#include <QMenuBar>
#include <QCheckBox>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QSlider>
#include <QTimer>
#include <QString>

#include "OpenGLWidget.h"
#include "PaletteWidget.h"
#include "Simulation.h"

class MainWidget : public QWidget
{
	Q_OBJECT

	public:
		MainWidget(QWidget* parent = 0);
		~MainWidget();
		void setSimulation(Simulation* simulation);
		void loadSimulation(QString filename);

		OpenGLWidget* openGLWidget;
		PaletteWidget* paletteWidget;

	protected:
		void closeEvent(QCloseEvent *event);

	public slots:
		void timerUpdate();
		void updateFromSimulation();
		void clickedPlayPause(bool value);
		void clickedStop();
		void clickedForward();
		void clickedBackward();
		void clickedBeginning();
		void clickedEnd();
		void clickedIncreaseFps();
		void clickedDecreaseFps();
		void triggeredOpen();
		void triggeredExit();
		void triggeredAbout();
		void triggeredEditPalette();
		void triggeredSetWindowSize();
		void changedTimeline(int value);
		void fpsUpdate();
		void skipUpdate();
		void timestepUpdate();
		void toggledQuantityTemperature(bool value);
		void toggledQuantityDensity(bool value);
		void toggledQuantityVRadial(bool value);
		void toggledQuantityVAzimuthal(bool value);
		void toogledSetLogarithmic(bool value);
		void triggeredSetMinimumValue();
		void triggeredSetMaximumValue();
		void triggeredAutoscale();
		void triggeredResetCamera();

	private:
		void createMenu();
		void createButtons();

		QMenuBar* menuBar;

		QMenu* fileMenu;
		QMenu* quantityMenu;
		QMenu* viewMenu;
		QMenu* optionsMenu;
		QMenu* helpMenu;

		QActionGroup* quantityActionGroup;
		QAction* exitAction;
		QAction* openAction;
		QAction* aboutAction;
		QAction* resetCameraAction;
		QAction* showDiskAction;
		QAction* showGridAction;
		QAction* showParticlesAction;
		QAction* showPlanetsAction;
		QAction* showOrbitsAction;
		QAction* showRocheLobeAction;
		QAction* showSkyAction;
		QAction* showTextAction;
		QAction* showDiskBorderAction;
		QAction* showKeyAction;
		QAction* useMultisampling;
		QAction* saveScreenshotsAction;
		QAction* setWindowSizeAction;
		QAction* setLogarithmicAction;
		QAction* setMinimumValueAction;
		QAction* setMaximumValueAction;
		QAction* autoscaleAction;
		QAction* editPaletteAction;
		QAction* quantityDensityAction;
		QAction* quantityTemperatureAction;
		QAction* quantityVRadialAction;
		QAction* quantityVAzimuthalAction;

		QToolButton* playPauseButton;
		QToolButton* stopButton;
		QToolButton* forwardButton;
		QToolButton* backwardButton;
		QToolButton* beginningButton;
		QToolButton* endButton;
		QToolButton* increaseFpsButton;
		QToolButton* decreaseFpsButton;

		QLineEdit* timestepLineEdit;
		QLineEdit* fpsLineEdit;
		QLineEdit* skipLineEdit;

		QSlider* timelineSlider;

		QHBoxLayout* buttonsLayout;
		QVBoxLayout* mainLayout;

		QTimer* timer;

		QSettings* settings;

		Simulation* simulation;
		double fps;
		unsigned int skip;

	protected:

};

#endif
