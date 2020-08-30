// 2020 (c) Oleksandr Kolodkin <alexandr.kolodkin@gmail.com>
// All rights reserved.


#include "magneticpulse.h"

MagneticPulse::MagneticPulse(QModbusClient *client, QObject *parent)
    : QObject(parent)
    , mPulsesCount(8)
    , mClient(client)
    , mCharging(false)
    , mChargingFaultSimulation(false)
{
	if (client != nullptr) {

		mTimer.setInterval(MODBUS_TIMEOUT);
		connect(&mTimer, &QTimer::timeout, this, &MagneticPulse::timeout);

		client->setTimeout(100);

		connect(client, &QModbusClient::errorOccurred, this, [this](){
			emit this->log(this->mClient->errorString());
		});

		connect(client, &QModbusClient::stateChanged, this, [this](QModbusDevice::State state) {
			if (state == QModbusDevice::State::ConnectedState) {
				this->log("Connected.");
				mTimer.start();
			}
		});
	}
}

void MagneticPulse::connectDivice() {
	if ((mClient != nullptr) && (mClient->state() != QModbusDevice::State::ConnectedState)) {
		mCharging = false;
		mClient->connectDevice();
	}
}

void MagneticPulse::diconnectDivice() {
	if ((mClient != nullptr) && (mClient->state() != QModbusDevice::State::UnconnectedState))
		mClient->disconnectDevice();
}

void MagneticPulse::finished(QModbusReply *reply) {

	mTimer.stop();

	//
	if ((reply != nullptr) && (reply->result().registerType() == QModbusDataUnit::DiscreteInputs)) {
		QModbusDataUnit data = reply->result();
		if (data.isValid()) {

			// Set MODBUS bit
			if (!data.value(MODBUS_ADDR)) sendWriteRequest(MODBUS_ADDR, true);

			// Clear reset request
			if (data.value(RESET_ADDR)) sendWriteRequest(RESET_ADDR, false);

			// Update LED's
			emit setFault    (data.value(   FAULT_ADDR));
			emit setCharging (data.value(  CHARGE_ADDR));
			emit setCharged  (data.value( CHARGED_ADDR));
			emit setLocal    (data.value(   LOCAL_ADDR));
			emit setRemote   (data.value(  REMOTE_ADDR));
			emit setReady    (data.value(   READY_ADDR));

			for (int i = 0; i < mPulsesCount; i++) {

				// Send start request
				if (data.value(MODBUS_BASE_ADDR + i)) sendWriteRequest(MODBUS_BASE_ADDR + i, false);

				// Update LED's
				this->setCmd(i, data.value(CMD_BASE_ADDR + i));
				this->setPulse(i, data.value(PULSE_BASE_ADDR + i));
				this->setSelected(i, data.value(SELECTED_BASE_ADDR + i));
				this->setFaultN(i, data.value(FAULT_BASE_ADDR + i));
			}
		}
		reply->deleteLater();
	}

	// Skip if not connected
	if (mClient == nullptr) return;
	if (mClient->state() != QModbusClient::State::ConnectedState) return;

	//
	if (mQueue.isEmpty()) {
		QModbusDataUnit data(QModbusDataUnit::DiscreteInputs, 0, 172);
		QModbusReply *reply = mClient->sendReadRequest(data, 255);
		if (reply != nullptr) {
			connect(reply, &QModbusReply::finished, this, [this](){
				this->finished(qobject_cast<QModbusReply*>(sender()));
			});
			return;
		}
	} else {
		auto task = mQueue.dequeue();
		this->log(tr("Sending write request: %1 = %2").arg(task.addr).arg(task.value));

		QVector<quint16> v;
		v.append(task.value ? 0xFF00 : 0);

		QModbusDataUnit data(QModbusDataUnit::Coils, task.addr, v);
		QModbusReply *reply = mClient->sendWriteRequest(data, 255);
		if (reply != nullptr) {
			connect(reply, &QModbusReply::finished, this, [this](){
				this->finished(qobject_cast<QModbusReply*>(sender()));
			});
			return;
		}
	}

	//
	mTimer.start();
}
