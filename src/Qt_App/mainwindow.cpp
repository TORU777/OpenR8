#include "mainwindow.h"
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
	setWindowTitle(tr("Qt_App"));

	btn1 = new QPushButton(tr("Open"), this);
	btn1->setGeometry(30, 30, 120, 50);
	//btn1->setText(tr("Open"));

	btn2 = new QPushButton(tr("Run"), this);
	btn2->setGeometry(30, 100, 120, 50);
	//btn2->setText(tr("Run"));

	btn3 = new QPushButton(tr("Close"), this);
	btn3->setGeometry(30, 170, 120, 50);
	//btn3->setText(tr("Close"));

	//openAction = new QAction(this);
	//connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
	QObject::connect(btn1, SIGNAL(clicked()), this, SLOT(open()));
	QObject::connect(btn3, SIGNAL(clicked()), this, SLOT(close()));


	//QMenu *file = menuBar()->addMenu(tr("&File"));
	//file->addAction(openAction);

}

void MainWindow::open()
{
	QMessageBox::information(this, tr("information dialog"), tr("open file"));
}