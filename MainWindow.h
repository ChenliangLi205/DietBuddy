#pragma once

#include <QtWidgets/QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSharedPointer>
#include <QLabel>
#include <QEvent>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(int screenWidth, int screenHeight, QWidget *parent = Q_NULLPTR);

private:
	bool hided, closeAnyway; //close no matter hided or not
	QWidget* newCentralWidget;
	QVBoxLayout* MainLayout;
	QLabel* timeLabel;
	QSystemTrayIcon* tray;
	QAction* quitAction, * resumeAction;
	QMenu* trayMenu;

protected:
	void mainWindowResume(QSystemTrayIcon::ActivationReason reason);
	void closeEvent(QCloseEvent*);
};
