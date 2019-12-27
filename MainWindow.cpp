#include "MainWindow.h"
#include <windows.h>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QMenu>
#include <QAction>

QString CurrentTimePeriod(unsigned short hour)
{
	if (6 <= hour && hour < 12) return QString::fromLocal8Bit("上午");
	else if (12 <= hour && hour < 18) return QString::fromLocal8Bit("下午");
	else return QString::fromLocal8Bit("晚上");
}

MainWindow::MainWindow(int screenWidth, int screenHeight, QWidget *parent)
	: QMainWindow(parent)
{
	this->resize(screenWidth/3, screenHeight/3);
	this->setWindowTitle(QString::fromLocal8Bit("Diet Buddy 食伴"));
	
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	this->timeLabel = new QLabel(this);
	this->timeLabel->setText(QString::fromLocal8Bit("您好，现在是北京时间%1年%2月%3日星期%4%5。").
		arg(QString::number(sysTime.wYear), QString::number(sysTime.wMonth), QString::number(sysTime.wDay),
			QString::number(sysTime.wDayOfWeek), CurrentTimePeriod(sysTime.wHour)));

	// Enable tray Icon and resuming and quiting from tray
	this->tray = new QSystemTrayIcon(QIcon(":/images/TrayIcon"), this);
	this->tray->show();
	QObject::connect(this->tray, &QSystemTrayIcon::activated, this, &MainWindow::mainWindowResume);
	this->trayMenu = new QMenu(this);
	this->resumeAction = new QAction(QString::fromLocal8Bit("主界面"), this);
	this->quitAction = new QAction(QString::fromLocal8Bit("退出"), this);
	QObject::connect(this->resumeAction, &QAction::triggered, [this]() {this->show(); this->hided = false; });
	QObject::connect(this->quitAction, &QAction::triggered, [this]() {this->closeAnyway=true; this->close(); });
	this->trayMenu->addAction(resumeAction);
	this->trayMenu->addAction(quitAction);
	this->tray->setContextMenu(trayMenu);

	this->newCentralWidget = new QWidget(this);
	this->MainLayout = new QVBoxLayout(this);

	//add time label
	this->MainLayout->addWidget(timeLabel);
	this->newCentralWidget->setLayout(MainLayout);
	this->setCentralWidget(newCentralWidget);

	this->hided = false;
	this->closeAnyway = false;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (!this->hided && !this->closeAnyway)
	{
		this->hide();
		this->hided = true;
		event->ignore();
	}
	else
	{
		event->accept();
	}
}

void MainWindow::mainWindowResume(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger)
	{
		this->show();
		this->hided = false;
	}
}
