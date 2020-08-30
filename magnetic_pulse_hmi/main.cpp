// 2020 (c) Oleksandr Kolodkin <alexandr.kolodkin@gmail.com>
// All rights reserved.


#include <QApplication>
#include <QStyleFactory>
#include <QTranslator>
#include <QLibraryInfo>

#include <QDebug>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	Q_INIT_RESOURCE(resources);

	a.setApplicationName("Magnetic Pulse Test");
	a.setOrganizationName("Alexandr Kolodkin");
	a.setStyle(QStyleFactory::create("Fusion"));

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	a.installTranslator(&qtTranslator);

	QTranslator myTranslator;
	myTranslator.load("my_" + QLocale::system().name(), ":/");
	a.installTranslator(&myTranslator);


	MainWindow w;
	w.show();
	w.restoreSession();

	return a.exec();
}
