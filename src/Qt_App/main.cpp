// Reference: http://www.saoe.net/blog/715/

#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>


int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	//QLabel* label = new QLabel("Hello, world!");
	//label->show();
	MainWindow w;
	w.resize(500, 500);
	w.show();

	//openAction = new QAction(this);
	//connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
	//QObject::connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

	//QPushButton *btn = new QPushButton("Open");
	//btn->resize(250, 50);
	//QObject::connect(btn, SIGNAL(clicked()), &app, SLOT(quit()));
	//btn->show();

	return app.exec();
}
