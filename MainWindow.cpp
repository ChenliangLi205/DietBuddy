#include "MainWindow.h"
#include <windows.h>
#include <QDebug>
#include <QObject>
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
#include <QThread>
#include <QMutex>
#include <QPushButton>
#include <QSpinBox>
#include <QSlider>

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
	this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinimizeButtonHint);
	this->setWindowTitle(QString::fromLocal8Bit("Diet Buddy 食伴"));
	
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	this->timeLabel = new QLabel(this);
	this->timeLabel->setText(QString::fromLocal8Bit("您好，现在是北京时间%1年%2月%3日星期%4%5。").
		arg(QString::number(sysTime.wYear), QString::number(sysTime.wMonth), QString::number(sysTime.wDay),
			QString::number(sysTime.wDayOfWeek), CurrentTimePeriod(sysTime.wHour)));
	
	//Setup the drink notify area
	this->drinkLabel = new QLabel(this);
	this->drinkLabel->setText(QString::fromLocal8Bit("您现在还不需要喝水"));
	this->drinkButton = new QPushButton(QString::fromLocal8Bit("劳资喝过了"), this);
	this->drinkButton->hide();
	QObject::connect(this->drinkButton, &QPushButton::clicked, this, &MainWindow::onPushDrinkButton);

	//Setup the interval setting area
	this->intervalLabel = new QLabel(QString::fromLocal8Bit("提醒喝水的时间间隔（分钟）:"), this);
	this->intervalSpinBox = new QSpinBox(this);
	this->intervalSpinBox->setRange(1, 360);
	this->intervalSpinBox->setValue(60);
	this->intervalSlider = new QSlider(Qt::Horizontal, this);
	this->intervalSlider->setRange(1, 360);
	this->intervalSlider->setValue(60);
	void (QSpinBox:: * valueChangedInt)(int) = &QSpinBox::valueChanged;
	QObject::connect(this->intervalSpinBox, valueChangedInt, this->intervalSlider, &QSlider::setValue);
	QObject::connect(this->intervalSlider, &QSlider::valueChanged, this->intervalSpinBox, &QSpinBox::setValue);
	QObject::connect(this->intervalSpinBox, valueChangedInt, this, &MainWindow::onIntervalSpinBoxChanged);
	
	// Enable tray Icon and resuming and quiting from tray
	this->tray = new QSystemTrayIcon(QIcon(":/images/TrayIcon"), this);
	this->tray->show();
	QObject::connect(this->tray, &QSystemTrayIcon::activated, this, &MainWindow::mainWindowResume);
	this->trayMenu = new QMenu(this);
	this->resumeAction = new QAction(QString::fromLocal8Bit("主界面"), this);
	this->quitAction = new QAction(QString::fromLocal8Bit("退出"), this);
	QObject::connect(this->resumeAction, &QAction::triggered, [this]() {this->show(); this->hided = false; keepHided = false; });
	QObject::connect(this->quitAction, &QAction::triggered, [this]() {this->closeAnyway=true; this->close(); });
	this->trayMenu->addAction(resumeAction);
	this->trayMenu->addAction(quitAction);
	this->tray->setContextMenu(trayMenu);

	this->newCentralWidget = new QWidget(this);
	this->mainLayout = new QVBoxLayout(this);
	this->timeLayout = new QHBoxLayout(this);
	this->drinkLayout = new QHBoxLayout(this);
	this->setIntervalLayout = new QHBoxLayout(this);
	this->intervalTextLayout = new QHBoxLayout(this);

	//add labels for text information
	this->timeLayout->addWidget(timeLabel);
	this->drinkLayout->addWidget(drinkLabel);
	this->drinkLayout->addWidget(drinkButton);
	this->intervalTextLayout->addWidget(intervalLabel);
	this->setIntervalLayout->addWidget(intervalSpinBox);
	this->setIntervalLayout->addWidget(intervalSlider);
	this->mainLayout->addLayout(timeLayout);
	this->mainLayout->addLayout(intervalTextLayout);
	this->mainLayout->addLayout(setIntervalLayout);
	this->mainLayout->addLayout(drinkLayout);
	this->newCentralWidget->setLayout(mainLayout);
	this->setCentralWidget(newCentralWidget);

	this->hided = false;
	this->keepHided = false;
	this->closeAnyway = false;

	this->timeRecorderThread = new timeRecorder(this);
	QObject::connect(this->timeRecorderThread, &timeRecorder::drinkSignal, this, &MainWindow::onReceiveDrinkMessage);
	this->timeRecorderThread->start();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (!this->hided && !this->closeAnyway)
	{
		this->hide();
		this->hided = true;
		this->keepHided = true;
		event->ignore();
	}
	else
	{
		this->timeRecorderThread->stop();
		this->timeRecorderThread->wait();
		event->accept();
	}
}

void MainWindow::mainWindowResume(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger)
	{
		this->show();
		this->hided = false;
		this->keepHided = false;
	}
}

void MainWindow::onReceiveDrinkMessage(const QString& str)
{
	this->timeRecorderThread->stop();
	this->timeRecorderThread->wait();
	this->drinkLabel->setText(str);
	this->drinkButton->show();
	if (this->hided)
	{
		this->show(); 
		this->hided = false;
	}
}

void MainWindow::onPushDrinkButton()
{
	this->drinkLabel->setText(QString::fromLocal8Bit("您现在还不需要喝水"));
	this->drinkButton->hide();
	if (this->keepHided)
	{
		this->hide();
		this->hided = true;
	}
	this->timeRecorderThread->start();
}

void MainWindow::onIntervalSpinBoxChanged()
{
	this->timeRecorderThread->intervalLock.lock();
	this->timeRecorderThread->interval = this->intervalSpinBox->value();
	this->timeRecorderThread->intervalLock.unlock();
}

timeRecorder::timeRecorder(QObject* parent) : QThread(parent)
{
	this->shouldStop = false;
	this->minutesFromLastDrink = 0;
}

void timeRecorder::stop()
{
	QMutexLocker locker(&this->shouldStopLock);
	this->shouldStop = true;
}

void timeRecorder::run()
{
	qDebug() << "time recorder starts running";
	SYSTEMTIME curTime, lastTime;
	GetLocalTime(&lastTime);
	while (true)
	{
		{
			QMutexLocker locker(&this->shouldStopLock);
			if (shouldStop)
			{
				this->shouldStop = false;
				return;
			}
		}
		GetLocalTime(&curTime);
		if (curTime.wMinute != lastTime.wMinute || curTime.wHour != lastTime.wHour)
		{
			if (curTime.wMinute < lastTime.wMinute)
				minutesFromLastDrink += curTime.wMinute + 60 - lastTime.wMinute;
			else
				minutesFromLastDrink += curTime.wMinute - lastTime.wMinute;
			lastTime = curTime;
			//If the time without drinking reaches interval
			if (this->intervalLock.tryLock())
			{
				if (this->minutesFromLastDrink >= this->interval)
				{
					this->sendDrinkMessage(QString());
					this->minutesFromLastDrink = 0;
				}
				intervalLock.unlock();
			}	
		}
		sleep(60);
	}
}

void timeRecorder::sendDrinkMessage(const QString& curTime)
{
	emit drinkSignal(curTime+QString::fromLocal8Bit(" 您该喝水辣！"));
}
