// 2020 (c) Oleksandr Kolodkin <alexandr.kolodkin@gmail.com>
// All rights reserved.


#include "magneticpulse.h"

MagneticPulse::MagneticPulse(QModbusClient *client, QObject *parent)
    : QObject(parent)
    , mClient(client)
    , mIsConnected(PULSES_COUNT, true)
    , mCharging(false)
    , mChargingFaultSimulation(false)
{
	mChargingTimer.setSingleShot(true);
	connect(&mChargingTimer, &QTimer::timeout, this, [this](){
		if (!mChargingFaultSimulation) sendWriteRequest(CHARGED_ADDR, true);
	});

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
			this->setFault(data.value(FAULT_ADDR));
			this->setCharging(data.value(CHARGE_ADDR));
			this->setCharged(data.value(CHARGED_ADDR));

			// Simulate charging
			if (mCharging != data.value(CHARGE_ADDR)) {
				mCharging = data.value(CHARGE_ADDR);
				if (mCharging && !mChargingFaultSimulation) mChargingTimer.start(5000);
			}

			for (int i = 0; i < PULSES_COUNT; i++) {

				// Simulate discharging
				if (mIsConnected[i] && data.value(PULSE_BASE_ADDR + i) && data.value(CHARGED_ADDR)) {
					sendWriteRequest(CHARGED_ADDR, false);
				}

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
