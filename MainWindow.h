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
#include <QThread>
#include <QMutex>
#include <QPushButton>
#include <QSpinBox>
#include <QSlider>

class timeRecorder : public QThread
{
	Q_OBJECT
public:
	bool shouldStop;
	int minutesFromLastDrink, interval;
	QMutex shouldStopLock, intervalLock;
	timeRecorder(QObject* parent=Q_NULLPTR);
	void run();
	void stop();
private:
	void sendDrinkMessage(const QString& curTime);
signals:
	void drinkSignal(const QString& str);
};

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(int screenWidth, int screenHeight, QWidget *parent = Q_NULLPTR);

private:
	bool hided, keepHided, closeAnyway; //close no matter hided or not
	QWidget* newCentralWidget;
	QVBoxLayout* mainLayout;
	QHBoxLayout* drinkLayout, * timeLayout, * setIntervalLayout, * intervalTextLayout;
	QPushButton* drinkButton;
	QLabel *timeLabel, *drinkLabel, *intervalLabel;
	QSystemTrayIcon* tray;
	QAction* quitAction, * resumeAction;
	QMenu* trayMenu;
	timeRecorder* timeRecorderThread;
	QSpinBox* intervalSpinBox;
	QSlider* intervalSlider;

protected:
	void mainWindowResume(QSystemTrayIcon::ActivationReason reason);
	void closeEvent(QCloseEvent*);
	void onReceiveDrinkMessage(const QString& str);
	void onPushDrinkButton();
	void onIntervalSpinBoxChanged();
};
