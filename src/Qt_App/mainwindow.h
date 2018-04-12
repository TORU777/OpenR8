#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QAction>
#include <QtWidgets/QPushButton>
#include <QtCore/QObject>

class MainWindow : public QMainWindow
{
	//Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
private:
	//QAction *openAction;
	QPushButton *btn1;
	QPushButton *btn2;
	QPushButton *btn3;

	private slots:
	void open();
};

#endif 


