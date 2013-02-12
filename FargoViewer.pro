version.target = version.h
version.commands = echo "\\$${LITERAL_HASH}ifndef _VERSION_H_\\\\nconst char git_version[] = \\\"`git describe --tags --always --dirty`\\\"\\;\\\\n\\$${LITERAL_HASH}endif" > version.h
version.depends = .git

QMAKE_EXTRA_TARGETS += version

PRE_TARGETDEPS += version.h

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
QT+=opengl
CONFIG += debug
#CONFIG += release
LIBS += -lGLEW -lGLU
QMAKE_CXXFLAGS_DEBUG = -march=native -O2 -pipe -g
QMAKE_CXXFLAGS_RELEASE = -march=native -O2 -pipe -DNDEBUG

debug {
	TARGET = FargoViewer.debug
}

release {
	TARGET = FargoViewer
}

#QMAKE_CFLAGS+=-pg
#QMAKE_CXXFLAGS+=-pg
#QMAKE_LFLAGS+=-pg

# Input
HEADERS += MainWidget.h OpenGLWidget.h Simulation.h config.h Palette.h PaletteWidget.h ColorWidget.h RocheLobe.h Vector.h Matrix.h OpenGLNavigationWidget.h FARGO.h
SOURCES += main.cpp MainWidget.cpp OpenGLWidget.cpp Simulation.cpp config.cpp Palette.cpp PaletteWidget.cpp ColorWidget.cpp RocheLobe.cpp OpenGLNavigationWidget.cpp FARGO.cpp
