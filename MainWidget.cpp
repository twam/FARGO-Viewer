#include "MainWidget.h"

#include <QStyle>
#include <QIntValidator>
#include <QFileDialog>
#include <QInputDialog>
#include <float.h>
#include "Simulation.h"
#include "util.h"

MainWidget::MainWidget(QWidget *parent)
: QWidget(parent), simulation(NULL)
{
	fps = 10.0;

	openGLWidget = new OpenGLWidget(this);
	paletteWidget = new PaletteWidget(openGLWidget->getPalette(),0);
	connect(paletteWidget, SIGNAL(paletteUpdated()), openGLWidget, SLOT(update()));

	createMenu();
	createButtons();

	// setup layout
	mainLayout = new QVBoxLayout;
	mainLayout->setMenuBar(menuBar);
	mainLayout->addWidget(openGLWidget);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);
	
	// set window title
	setWindowTitle(tr("FARGO Viewer"));

	setMinimumSize(640,480);

	timer = new QTimer;
	timer->setInterval(1000.0/fps);

	connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
	
	setSimulation(NULL);
}

MainWidget::~MainWidget()
{
	delete openGLWidget;
	delete paletteWidget;
}

void MainWidget::closeEvent(QCloseEvent* /*event*/)
{
	paletteWidget->close();
}

void MainWidget::createMenu()
{
	menuBar = new QMenuBar;

	// file
	fileMenu = new QMenu(tr("&File"), this);

	openAction = fileMenu->addAction(tr("&Open"));
	connect(openAction, SIGNAL(triggered()), this, SLOT(triggeredOpen()));

	exitAction = fileMenu->addAction(tr("E&xit"));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(triggeredExit()));

	menuBar->addMenu(fileMenu);

	// options
	optionsMenu = new QMenu(tr("&Options"), this);

	saveScreenshotsAction = optionsMenu->addAction(tr("Save &Screenshots"));
	saveScreenshotsAction->setCheckable(true);
	saveScreenshotsAction->setChecked(false);
	openGLWidget->updateSaveScreenshots(false);
	connect(saveScreenshotsAction, SIGNAL(toggled(bool)), openGLWidget, SLOT(updateSaveScreenshots(bool)));

	setWindowSizeAction = optionsMenu->addAction(tr("Set &Window Size"));
	connect(setWindowSizeAction, SIGNAL(triggered()), this, SLOT(triggeredSetWindowSize()));
	
	editPaletteAction = optionsMenu->addAction(tr("Edit &Palette"));
	connect(editPaletteAction, SIGNAL(triggered()), this, SLOT(triggeredEditPalette()));

	setLogarithmicAction = optionsMenu->addAction(tr("Set &Logarithmic"));
	setLogarithmicAction->setCheckable(true);
	setLogarithmicAction->setChecked(false);
	this->toogledSetLogarithmic(false);
 	connect(setLogarithmicAction, SIGNAL(toggled(bool)), this, SLOT(toogledSetLogarithmic(bool)));

	setMinimumValueAction = optionsMenu->addAction(tr("Set &Minimum Value"));
	connect(setMinimumValueAction, SIGNAL(triggered()), this, SLOT(triggeredSetMinimumValue()));
	
	setMaximumValueAction = optionsMenu->addAction(tr("Set Ma&ximum Value"));
	connect(setMaximumValueAction, SIGNAL(triggered()), this, SLOT(triggeredSetMaximumValue()));

	menuBar->addMenu(optionsMenu);

	// quantity
	quantityMenu = new QMenu(tr("Quantity"), this);

	quantityActionGroup = new QActionGroup(this);
	quantityActionGroup->setExclusive(true);

	quantityDensityAction = quantityActionGroup->addAction(tr("&Density"));
	quantityDensityAction->setCheckable(true);
	quantityDensityAction->setChecked(true);
	connect(quantityDensityAction, SIGNAL(toggled(bool)), this, SLOT(toggledQuantityDensity(bool)));

	quantityTemperatureAction = quantityActionGroup->addAction(tr("&Temperature"));
	quantityTemperatureAction->setCheckable(true);
	connect(quantityTemperatureAction, SIGNAL(toggled(bool)), this, SLOT(toggledQuantityTemperature(bool)));

	quantityMenu->addActions(quantityActionGroup->actions());

	// view
	viewMenu = new QMenu(tr("&View"), this);
	viewMenu->addMenu(quantityMenu);

	showDiskAction = viewMenu->addAction(tr("Show &Disk"));
	showDiskAction->setCheckable(true);
	showDiskAction->setChecked(true);
	openGLWidget->updateShowDisk(true);
	connect(showDiskAction, SIGNAL(toggled(bool)), openGLWidget, SLOT(updateShowDisk(bool)));
	
	showDiskBorderAction = viewMenu->addAction(tr("Show disk &border"));
	showDiskBorderAction->setCheckable(true);
	showDiskBorderAction->setChecked(false);
	openGLWidget->updateShowDiskBorder(false);
	connect(showDiskBorderAction, SIGNAL(toggled(bool)), openGLWidget, SLOT(updateShowDiskBorder(bool)));

	showPlanetsAction = viewMenu -> addAction(tr("Show &Planets"));
	showPlanetsAction->setCheckable(true);
	showPlanetsAction->setChecked(true);
	openGLWidget->updateShowPlanets(true);
	connect(showPlanetsAction, SIGNAL(toggled(bool)), openGLWidget, SLOT(updateShowPlanets(bool)));

	showOrbitsAction = viewMenu->addAction(tr("Show &Orbits"));
	showOrbitsAction->setCheckable(true);
	showOrbitsAction->setChecked(true);
	openGLWidget->updateShowOrbits(true);
	connect(showOrbitsAction, SIGNAL(toggled(bool)), openGLWidget, SLOT(updateShowOrbits(bool)));
	
	showSkyAction = viewMenu->addAction(tr("Show &Sky"));
	showSkyAction->setCheckable(true);
	showSkyAction->setChecked(true);
	openGLWidget->updateShowSky(true);
	connect(showSkyAction, SIGNAL(toggled(bool)), openGLWidget, SLOT(updateShowSky(bool)));

	showTextAction = viewMenu->addAction(tr("Show &Text"));
	showTextAction->setCheckable(true);
	showTextAction->setChecked(true);
	openGLWidget->updateShowText(true);
	connect(showTextAction, SIGNAL(toggled(bool)), openGLWidget, SLOT(updateShowText(bool)));
	
	showKeyAction = viewMenu->addAction(tr("Show &Key"));
	showKeyAction->setCheckable(true);
	showKeyAction->setChecked(true);
	openGLWidget->updateShowKey(true);
	connect(showKeyAction, SIGNAL(toggled(bool)), openGLWidget, SLOT(updateShowKey(bool)));
	
	useMultisampling = viewMenu->addAction(tr("Use &Multisampling"));
	useMultisampling->setCheckable(true);
	useMultisampling->setChecked(true);
	openGLWidget->updateUseMultisampling(true);
	connect(useMultisampling, SIGNAL(toggled(bool)), openGLWidget, SLOT(updateUseMultisampling(bool)));

	menuBar->addMenu(viewMenu);	
}

void MainWidget::createButtons()
{
	QSize iconSize(16,16);

	timelineSlider = new QSlider(Qt::Horizontal);
	timelineSlider->setMinimumWidth(200);
	connect(timelineSlider, SIGNAL(valueChanged(int)), this, SLOT(changedTimeline(int)));

	playPauseButton = new QToolButton;
	playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	playPauseButton->setIconSize(iconSize);
	playPauseButton->setToolTip(tr("Play"));
	connect(playPauseButton, SIGNAL(clicked(bool)), this, SLOT(clickedPlayPause(bool)));

	stopButton = new QToolButton;
	stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
	stopButton->setIconSize(iconSize);
	stopButton->setToolTip(tr("Stop"));
	connect(stopButton, SIGNAL(clicked()), this, SLOT(clickedStop()));

	backwardButton = new QToolButton;
	backwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
	backwardButton->setIconSize(iconSize);
	backwardButton->setToolTip(tr("Jump Backward"));
	connect(backwardButton, SIGNAL(clicked()), this, SLOT(clickedBackward()));
	
	forwardButton = new QToolButton;
	forwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
	forwardButton->setIconSize(iconSize);
	forwardButton->setToolTip(tr("Jump Forward"));
	connect(forwardButton, SIGNAL(clicked()), this, SLOT(clickedForward()));
	
	beginningButton = new QToolButton;
	beginningButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	beginningButton->setIconSize(iconSize);
	beginningButton->setToolTip(tr("Jump to Beginning"));
	connect(beginningButton, SIGNAL(clicked()), this, SLOT(clickedBeginning()));
	
	endButton = new QToolButton;
	endButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	endButton->setIconSize(iconSize);
	endButton->setToolTip(tr("Jump to End"));
	connect(endButton, SIGNAL(clicked()), this, SLOT(clickedEnd()));
	
	timestepLineEdit = new QLineEdit;
	QIntValidator* validator1 = new QIntValidator;
	validator1->setBottom(0);
	timestepLineEdit->setValidator(validator1);
	timestepLineEdit->setMaximumWidth(timestepLineEdit->sizeHint().width()/3.0);
	connect(timestepLineEdit, SIGNAL(returnPressed()), this, SLOT(timestepUpdate()));

	fpsLineEdit = new QLineEdit;
	QDoubleValidator* validator2 = new QDoubleValidator(this);
	validator2->setBottom(0);
	fpsLineEdit->setValidator(validator2);
	fpsLineEdit->setText(QString("%1").arg(fps));
	fpsLineEdit->setMaximumWidth(fpsLineEdit->sizeHint().width()/3.0);
	connect(fpsLineEdit, SIGNAL(returnPressed()), this, SLOT(fpsUpdate()));

	increaseFpsButton = new QToolButton;
	increaseFpsButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
	increaseFpsButton->setIconSize(iconSize);
	increaseFpsButton->setToolTip(tr("Increase FPS"));
	connect(increaseFpsButton, SIGNAL(clicked()), this, SLOT(clickedIncreaseFps()));

	decreaseFpsButton = new QToolButton;
	decreaseFpsButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
	decreaseFpsButton->setIconSize(iconSize);
	decreaseFpsButton->setToolTip(tr("Decrease FPS"));
	connect(decreaseFpsButton, SIGNAL(clicked()), this, SLOT(clickedDecreaseFps()));
	
	buttonsLayout = new QHBoxLayout;
	buttonsLayout->addWidget(beginningButton);
	buttonsLayout->addWidget(backwardButton);
	buttonsLayout->addWidget(timestepLineEdit);
	buttonsLayout->addWidget(stopButton);
	buttonsLayout->addWidget(playPauseButton);
	buttonsLayout->addWidget(forwardButton);
	buttonsLayout->addWidget(endButton);
	buttonsLayout->addSpacing(16);
	buttonsLayout->addWidget(timelineSlider);
	buttonsLayout->addSpacing(16);
	buttonsLayout->addWidget(decreaseFpsButton);
	buttonsLayout->addWidget(fpsLineEdit);
	buttonsLayout->addWidget(increaseFpsButton);
}

void MainWidget::timerUpdate()
{
	unsigned int currentTimestep = simulation->getCurrentTimestep();
	
	if ((currentTimestep == simulation->getLastTimeStep()) || (simulation->loadTimestep(currentTimestep+1)<0)) {
		clickedStop();
	}
}

void MainWidget::updateFromSimulation()
{
	if (simulation != NULL) {
		timestepLineEdit->setText(QString("%1").arg(simulation->getCurrentTimestep()));
		timelineSlider->setValue(simulation->getCurrentTimestep());
	}
}

void MainWidget::clickedPlayPause(bool value)
{
	if (playPauseButton->isCheckable() == true) {
		// button is in pause mode
		if (value) {
			timer->stop();
			timestepLineEdit->setReadOnly(false);
		} else {
			timer->start();
			timestepLineEdit->setReadOnly(true);
		}
	} else {
		// button is in play mode
		playPauseButton->setCheckable(true);
		playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
		playPauseButton->setToolTip(tr("Pause"));
		timestepLineEdit->setReadOnly(true);
		timer->start();
	}
}

void MainWidget::clickedStop()
{
	timer->stop();
	
	playPauseButton->setCheckable(false);
	playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	playPauseButton->setToolTip(tr("Play"));
	timestepLineEdit->setReadOnly(false);
}

void MainWidget::clickedForward()
{
	unsigned int currentTimestep = simulation->getCurrentTimestep();

	if (simulation->loadTimestep(min(currentTimestep+100,simulation->getLastTimeStep()))<0) {
		clickedStop();
	}
}

void MainWidget::clickedBackward()
{
	unsigned int currentTimestep = simulation->getCurrentTimestep();

	if (simulation->loadTimestep(max(0,(int)currentTimestep-100))<0) {
		clickedStop();
	}
	
}

void MainWidget::clickedBeginning()
{
	simulation->loadTimestep(0);
	clickedStop();
}

void MainWidget::clickedEnd()
{
	simulation->loadTimestep(simulation->getLastTimeStep());
	clickedStop();
}

void MainWidget::clickedIncreaseFps()
{
	fps += 1.0;

	timer->setInterval(1000.0/fps);
	fpsLineEdit->setText(QString("%1").arg(fps));
}

void MainWidget::clickedDecreaseFps()
{
	if (fps>1)
		fps -= 1.0;

	timer->setInterval(1000.0/fps);
	fpsLineEdit->setText(QString("%1").arg(fps));
}

void MainWidget::fpsUpdate()
{
	fps = fpsLineEdit->text().toDouble();
	timer->setInterval(1000.0/fps);
}

void MainWidget::timestepUpdate()
{
	simulation->loadTimestep(timestepLineEdit->text().toDouble());
}

void MainWidget::setSimulation(Simulation *simulation)
{
	this->simulation = simulation;

	if (simulation == NULL) { 
		timelineSlider->setEnabled(false);
		playPauseButton->setEnabled(false);
		stopButton->setEnabled(false);
		backwardButton->setEnabled(false);
		forwardButton->setEnabled(false);
		beginningButton->setEnabled(false);
		endButton->setEnabled(false);
		timestepLineEdit->setEnabled(false);
		quantityActionGroup->setEnabled(false);
	} else {
		connect(simulation, SIGNAL(dataUpdated()), this, SLOT(updateFromSimulation()));
		connect(simulation, SIGNAL(dataUpdated()), openGLWidget, SLOT(update()));

		timelineSlider->setEnabled(true);
		playPauseButton->setEnabled(true);
		stopButton->setEnabled(true);
		backwardButton->setEnabled(true);
		forwardButton->setEnabled(true);
		beginningButton->setEnabled(true);
		endButton->setEnabled(true);
		timestepLineEdit->setEnabled(true);
		quantityActionGroup->setEnabled(true);

		timelineSlider->setMinimum(0);
		timelineSlider->setMaximum(simulation->getLastTimeStep());

		openGLWidget->setSimulation(simulation);

		updateFromSimulation();
	}
}

void MainWidget::triggeredExit()
{
	close();
}

void MainWidget::triggeredOpen()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Simulation"));

	if (filename.isNull()) {
		delete simulation;
		simulation = NULL;
	} else {
		delete simulation;
		setSimulation(NULL);
		simulation = new Simulation;
		simulation->loadFromFile(filename.toAscii().data());
		setSimulation(simulation);
	}
}

void MainWidget::changedTimeline(int value)
{
	simulation->loadTimestep(value);
}

void MainWidget::triggeredSetWindowSize()
{
	unsigned int width = 0, height = 0;
	bool ok;

	width =  QInputDialog::getInt(this, tr("Set Window Size"), tr("Window width:"), 0, 0, 1920, 1, &ok);

	if (ok)
		height = QInputDialog::getInt(this, tr("Set Window Size"), tr("Window height:"), 0, 0, 1600, 1, &ok);

	if (ok)
		openGLWidget->setFixedSize(QSize(width,height));

	setMinimumSize(QSize(width+30,height+80));
}

void MainWidget::triggeredEditPalette()
{
	paletteWidget->show();
	paletteWidget->activateWindow();
}

void MainWidget::toggledQuantityTemperature(bool value)
{
	if (value) {
		simulation->setQuantityType(Simulation::TEMPERATURE);
		openGLWidget->update();
	}
}

void MainWidget::toggledQuantityDensity(bool value)
{
	if (value) {
		simulation->setQuantityType(Simulation::DENSITY);
		openGLWidget->update();	
	}
}

void MainWidget::toogledSetLogarithmic(bool value)
{
	openGLWidget->setLogarithmic(value);
}

void MainWidget::triggeredSetMinimumValue()
{
	bool ok;
	double value = QInputDialog::getDouble(this, tr("Minimum Value"), tr("Minimum Value:"), openGLWidget->getMinimumValue(), -DBL_MAX, DBL_MAX, 10, &ok);
	if (ok) {
		openGLWidget->setMinimumValue(value);
	}
}

void MainWidget::triggeredSetMaximumValue()
{
	bool ok;
	double value = QInputDialog::getDouble(this, tr("Maximum Value"), tr("Maximum Value:"), openGLWidget->getMaximumValue(), -DBL_MAX, DBL_MAX, 10, &ok);

	if (ok) {
		openGLWidget->setMaximumValue(value);
	}
}