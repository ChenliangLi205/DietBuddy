#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QRect>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QDesktopWidget* desktopWidget = QApplication::desktop();
	if (desktopWidget != Q_NULLPTR)
	{
		QRect rect = desktopWidget->screenGeometry();
		MainWindow w(rect.width(), rect.height());
		w.show();
		return a.exec();
	}
	else
		return 0;
}
