// 2020 (c) Oleksandr Kolodkin <alexandr.kolodkin@gmail.com>
// All rights reserved.


#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->outputFault->setColor(QLed::red);

	mClient        = new QModbusTcpClient(this);
	mMagneticPulse = new MagneticPulse(mClient, this);

	mCmdLeds
	    << ui->outputCommand1
	    << ui->outputCommand2
	    << ui->outputCommand3
	    << ui->outputCommand4
	    << ui->outputCommand5
	    << ui->outputCommand6
	    << ui->outputCommand7
	    << ui->outputCommand8;

	mSelectedLeds
	    << ui->outputSelected1
	    << ui->outputSelected2
	    << ui->outputSelected3
	    << ui->outputSelected4
	    << ui->outputSelected5
	    << ui->outputSelected6
	    << ui->outputSelected7
	    << ui->outputSelected8;

	mPulseLeds
	    << ui->outputPulse1
	    << ui->outputPulse2
	    << ui->outputPulse3
	    << ui->outputPulse4
	    << ui->outputPulse5
	    << ui->outputPulse6
	    << ui->outputPulse7
	    << ui->outputPulse8;

	mFaultLeds
	    << ui->outputFault1
	    << ui->outputFault2
	    << ui->outputFault3
	    << ui->outputFault4
	    << ui->outputFault5
	    << ui->outputFault6
	    << ui->outputFault7
	    << ui->outputFault8;

	foreach (auto led, mFaultLeds) {
		led->setColor(QLed::Color::red);
	}

	mButtons[ui->inputStart1]  = 0;
	mButtons[ui->inputStart2]  = 1;
	mButtons[ui->inputStart3]  = 2;
	mButtons[ui->inputStart4]  = 3;
	mButtons[ui->inputStart5]  = 4;
	mButtons[ui->inputStart6]  = 5;
	mButtons[ui->inputStart7]  = 6;
	mButtons[ui->inputStart8]  = 7;

	foreach (auto *button, mButtons.keys()) {
		connect(button, &QPushButton::clicked, this, [this](){
			QPushButton *button = qobject_cast<QPushButton*>(sender());
			if (mButtons.contains(button)) mMagneticPulse->start(mButtons.value(button));
		});
	}

	connect(mMagneticPulse, &MagneticPulse::setCharging, this, [this](bool state) {
		ui->outputCharging->setState(state);
	});

	connect(mMagneticPulse, &MagneticPulse::setCharged, this, [this](bool state) {
		ui->outputCharged->setState(state);
	});

	connect(mMagneticPulse, &MagneticPulse::setFault, this, [this](bool state) {
		ui->outputFault->setState(state);
	});

	connect(mMagneticPulse, &MagneticPulse::setReady, this, [this](bool state) {
		ui->outputReady->setState(state);
	});

	connect(mMagneticPulse, &MagneticPulse::setLocal, this, [this](bool state) {
		ui->outputLocal->setState(state);
	});

	connect(mMagneticPulse, &MagneticPulse::setRemote, this, [this](bool state) {
		ui->outputRemote->setState(state);
	});

	connect(mMagneticPulse, &MagneticPulse::setCmd, this, [this](int n, bool state){
		if ((n >= 0) && (n < mCmdLeds.count())) {
			mCmdLeds.value(n)->setState(state);
		}
	});

	connect(mMagneticPulse, &MagneticPulse::setSelected, this, [this](int n, bool state){
		if ((n >= 0) && (n < mSelectedLeds.count())) {
			mSelectedLeds.value(n)->setState(state);
		}
	});

	connect(mMagneticPulse, &MagneticPulse::setPulse, this, [this](int n, bool state){
		if ((n >= 0) && (n < mPulseLeds.count())) {
			mPulseLeds.value(n)->setState(state);
		}
	});

	connect(mMagneticPulse, &MagneticPulse::setFaultN, this, [this](int n, bool state){
		if ((n >= 0) && (n < mFaultLeds.count())) {
			mFaultLeds.value(n)->setState(state);
		}
	});

	connect(ui->inputStartTest, &QPushButton::clicked, mMagneticPulse, &MagneticPulse::connectDivice);
	connect(ui->inputStopTest,  &QPushButton::clicked, mMagneticPulse, &MagneticPulse::diconnectDivice);
	connect(ui->inputReset,     &QPushButton::clicked, mMagneticPulse, &MagneticPulse::reset);
	connect(ui->inputStartAll,  &QPushButton::clicked, mMagneticPulse, &MagneticPulse::startAll);
}

MainWindow::~MainWindow()
{
	delete ui;
	delete mClient;
}

void MainWindow::saveSession()
{
	QSettings settings;

	settings.setValue("geometry"     , saveGeometry());
	settings.setValue("state"        , saveState());
}

void MainWindow::restoreSession()
{
	QSettings settings;

	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	saveSession();
	event->accept();
}
