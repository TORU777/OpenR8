// Reference: http://www.saoe.net/blog/715/


#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtCore/QString>


int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QLabel* label = new QLabel("Hello, world!");
	label->show();
	return app.exec();
}
