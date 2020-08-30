// 2020 (c) Oleksandr Kolodkin <alexandr.kolodkin@gmail.com>
// All rights reserved.


#ifndef MAGNETICPULSE_H
#define MAGNETICPULSE_H

#include <QObject>
#include <QModbusClient>
#include <QQueue>
#include <QTimer>

#define MODBUS_TIMEOUT        10

#define READY_ADDR            82
#define FAULT_ADDR            83
#define CHARGED_ADDR          87
#define LOCAL_ADDR            88
#define REMOTE_ADDR           89
#define CHARGE_ADDR           91

#define PULSE_BASE_ADDR       96
#define CMD_BASE_ADDR         64
#define SELECTED_BASE_ADDR   112
#define FAULT_BASE_ADDR      160

#define MODBUS_BASE_ADDR     128
#define MODBUS_ADDR          144
#define RESET_ADDR           146

typedef struct {
	int addr;
	bool value;
} WriteTask;

class MagneticPulse : public QObject
{
	Q_OBJECT
public:
	explicit MagneticPulse(QModbusClient *client, QObject *parent = nullptr);

public slots:
	void setPulsesCount(int count) {if ((count > 0) && (count <= 16)) mPulsesCount = count;}
	void connectDivice();
	void diconnectDivice();
	void reset() {sendWriteRequest(RESET_ADDR, true);}
	void start(int i) {
		if ((i >= 0) && (i < mPulsesCount)) {
			sendWriteRequest(MODBUS_BASE_ADDR + i, true);
		}
	}
	void setSimulateChargingFault(bool state) {
		mChargingFaultSimulation = state;
	}
	void startAll() {
		for (int i = 0; i < mPulsesCount; i++) {
			sendWriteRequest(MODBUS_BASE_ADDR + i, true);
		}
	}

signals:
	void log(QString msg);
	void setCmd(int n, bool state);
	void setPulse(int n, bool state);
	void setSelected(int n, bool state);
	void setFaultN(int n, bool state);
	void setFault(bool state);
	void setCharging(bool state);
	void setCharged(bool state);
	void setLocal(bool state);
	void setRemote(bool state);
	void setReady(bool state);

private:
	int mPulsesCount;
	QModbusClient *mClient;
	bool mCharging;
	bool mChargingFaultSimulation;
	QQueue<WriteTask> mQueue;
	QTimer mTimer;

	void sendWriteRequest(int reg, bool value) {
		mQueue.enqueue({reg, value});
	};

private slots:
	void finished(QModbusReply *reply);
	void timeout() {finished(nullptr);}
};

#endif // MAGNETICPULSE_H
