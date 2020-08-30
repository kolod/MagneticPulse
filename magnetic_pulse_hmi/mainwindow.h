// 2020 (c) Oleksandr Kolodkin <alexandr.kolodkin@gmail.com>
// All rights reserved.


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QCloseEvent>
#include <QModbusTcpClient>
#include <QVector>
#include <QMap>
#include <QPushButton>
#include <QCheckBox>

#include "qled.h"
#include "magneticpulse.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
	void saveSession();
	void restoreSession();

private:
	Ui::MainWindow *ui;

	QModbusClient *mClient;
	MagneticPulse *mMagneticPulse;

	QVector<QLed*> mCmdLeds;
	QVector<QLed*> mPulseLeds;
	QVector<QLed*> mSelectedLeds;
	QVector<QLed*> mFaultLeds;
	QMap<QPushButton*, int> mButtons;

	void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
