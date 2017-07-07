/*
 * TLSensor.h - Capacitive sensing library based on TL method for Arduino
 * https://github.com/AdmarSchoonen/TLSensor
 * Copyright (c) 2016 Admar Schoonen
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TLSensor_h
#define TLSensor_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#include "pins_arduino.h"
#include "WConstants.h"
#endif
#include <avr/io.h>
#include <math.h>

#ifdef EEPROM_h
#include <avr/eeprom.h>
#endif

#include <TLSampleMethodCustom.h>
#include <TLSampleMethodCVD.h>
#include <TLSampleMethodResistive.h>
#include <TLSampleMethodTouchRead.h>

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
class TLSensors;

struct TLStruct {
	/* enum definitions */
	enum ButtonState {
		/*
		 * Button states.
		 * buttonStatePreCalibrating, buttonStateCalibrating,
		 * buttonStateNoisePowerMeasurement, buttonStateReleased,
		 * buttonStateReleasedToApproached, buttonStateApproached,
		 * buttonStateApproachedToReleased and
		 * buttonStateApproachedToPressed can be regarded as "released"
		 * or "not touched". 
		 *
		 * buttonStatePressed and buttonStatePressedToApproached can be
		 * regarded as "pressed" or "touched".
		 *
		 * In application code, this can be simplified by considering a
		 * button as "pressed" or "touched" if the button state is equal
		 * to or larger than buttonStatePressed.
		 *
		 * If desired, application can consider a button state larger
		 * than or equal to buttonStateApproached and smaller than or
		 * equal to buttonStateApproachedToReleased as "approached".
		 *
		 * Additionally, button state smaller than or equal to
		 * buttonStateNoisePowerMeasurement can be considered as
		 * "calibrating".
		 */
		buttonStatePreCalibrating = 0,
		buttonStateCalibrating = 1,
		buttonStateNoisePowerMeasurement = 2,
		buttonStateReleased = 3,
		buttonStateReleasedToApproached = 4,
		buttonStateApproached = 5,
		buttonStateApproachedToPressed = 6,
		buttonStateApproachedToReleased = 7,
		buttonStatePressed = 8,
		buttonStatePressedToApproached = 9,
		buttonStateMax = 10
	};

	enum Direction {
		/*
		 * directionPositive means the capacitance is increased when a
		 * user touches the button (this is the default behaviour).
		 * directionNegative means the capacitance is decreased when a
		 * user touches the button (not very common).
		 */
		directionNegative,
		directionPositive
	};

	enum SampleType {
		/*
		 * SampleType specifies the way samples are taken.
		 * sampleTypeNormal means the sensor is first discharged and
		 * reference capacitor is charged. sampleTypeInverted means the
		 * sensor is first charged and the reference capacitor is
		 * discharged. sampleTypeDifferential means that both normal and
		 * inverted samples must be taken and the difference between
		 * those must be computed.
		 *
		 * sampleTypeDifferential is slower but more robust against
		 * interference. sampleTypeDifferential is default.
		 *
		 * Note: do not change values associated with these types!
		 *
 		 * sampleTypeNormal must be binary 01 and sampletypeInverted
 		 * must be binary 10 so that sampleTypeDifferential is equal to
 		 * sampleTypeNormal | sampleTypeInverted.
		 */
		sampleTypeNormal = 1,
		sampleTypeInverted = 2,
		sampleTypeDifferential = 3
	};

	/*
	 * These members are set to defaults upon initialization but can be
	 * overruled by the user.
	 */
	int pin;
	int sampleMethodResistive_gndPin;
	bool sampleMethodResistive_useInternalPullup;
	enum Direction direction;
	enum SampleType sampleType;
	float releasedToApproachedThreshold; /* stored in EEPROM */
	float approachedToReleasedThreshold; /* stored in EEPROM */
	float approachedToPressedThreshold; /* stored in EEPROM */
	float pressedToApproachedThreshold; /* stored in EEPROM */
	uint32_t releasedToApproachedTime;
	uint32_t approachedToReleasedTime;
	uint32_t approachedToPressedTime;
	uint32_t pressedToApproachedTime;
	bool enableSlewrateLimiter; /* stored in EEPROM as global */
	unsigned long preCalibrationTime;
	unsigned long calibrationTime;
	unsigned long approachedTimeout;
	unsigned long pressedTimeout;
	uint16_t filterCoeff;
	uint32_t forceCalibrationWhenReleasingFromApproached;
	uint32_t forceCalibrationWhenApproachingFromReleased;
	uint32_t forceCalibrationWhenApproachingFromPressed;
	uint32_t forceCalibrationWhenPressing;
	bool setParallelCapacitanceManually;
	bool disableUpdateIfAnyButtonIsApproached;
	bool disableUpdateIfAnyButtonIsPressed;
	float referenceCapacitance; /* in pico Farad (pF) */
	float parallelCapacitance; /* in pico Farad (pF) */
	float capacitanceScaleFactor;
	float distanceOffset;
	float distanceScaleFactor;
	float relativePermittivity;
	float area; /* in mm^2 */
	uint32_t nChargesMin;
	uint32_t nChargesMax;
	bool useNChargesPadding;
	unsigned int chargeDelaySensor; /* delay to charge sensor in microseconds (us) */
	unsigned int chargeDelayADC; /* delay to charge ADC in microseconds (us) */

	/* 
	 * sampleMethod can be set to:
	 * - TLSampleMethodTL
	 * - TLSampleMethodResistive
	 * - TLSampleMethodTouchRead (Teensy 3.x only)
	 * - custom method
	 *
	 * For custom method: the inv parameter indicates if an inverted
	 * measurement is requested. This is used in pseudo differential
	 * measurements. If inverted measurements are not supported, just check
	 * return 0 when inv == true.
	 */
	int (*sampleMethod)(struct TLStruct * d, uint8_t nSensors, uint8_t ch, bool inv);

	/*
	 * Set enableTouchStateMachine to false to only use a sensor for
	 * capacitive sensing or during tuning. After startup, sensor will be in
	 * state buttonStatePreCalibrating followed by buttonStateCalibrating
	 * and buttonStateNoisePowerMeasurement. After noise measurement the state
	 * will switch to buttonStateReleased and stay there.
	 */
	bool enableTouchStateMachine;

	/*
	 * Set enableNoisePowerMeasurement to true to measure noise power.
	 * This is useful during tuning or debugging but adds processing time.
	 */
	bool enableNoisePowerMeasurement;

	/* These members will be set by the init / sample methods. */
	uint8_t nSensors;
	uint8_t nMeasurementsPerSensor;
	int32_t raw;
	/* Total capacitance (including parallelCapacitance) in pico Farad (pF) */
	float capacitance;
	float distance;
	float avg;
	float delta;
	float noisePower;
	enum ButtonState buttonState;
	const char * buttonStateLabel; /* human readable label */
	bool buttonIsCalibrating; /* use this to see if button is calibrating */
	bool buttonIsReleased; /* use this to see if button is released */
	bool buttonIsApproached; /* use this to see if button is approached */
	bool buttonIsPressed; /* use this to see if button is pressed */
	uint32_t counter;
	uint32_t recalCounter;
	uint32_t nCharges;
	uint32_t nChargesNext;
	unsigned long lastSampledAtTime;
	unsigned long stateChangedAtTime;
	bool slewrateFirstSample;
	bool stateIsBeingChanged;
};

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
class TLSensors
{
	public:
		struct TLStruct data[N_SENSORS];
		uint8_t nSensors;
		bool enableReadSettingsFromEeprom;
		int eepromOffset;

		/*
		 * Ideally scanOrder would be a static const uint8_t array the
		 * size of nSensors * nMeasurementPerSensor with a pseudo random
		 * sequence generated by the C preprocessor. I just don't know
		 * how to do this (or if it is possible at all). For now, we'll
		 * just use RAM and initialize it at start up, wasting some
		 * precious RAM space.
		 */
		uint8_t scanOrder[N_SENSORS * N_MEASUREMENTS_PER_SENSOR];
		uint8_t	nMeasurementsPerSensor;
		int8_t error;

		void writeSettingsToEeprom(void);
		int8_t setDefaults(void);
		int8_t sample(void);
		void printScanOrder(void);
		bool setForceCalibratingStates(int ch, uint32_t mask,
			enum TLStruct::ButtonState * newState);
		float getRaw(int n);
		float getCapacitance(int n);
		float getDelta(int n);
		float getAvg(int n);
		const char * getStateLabel(int n);
		enum TLStruct::ButtonState getState(int n);
		void setState(int n, enum TLStruct::ButtonState newState);
		TLSensors(void);
		~TLSensors(void);

		/* call backs: */
		void (*buttonStateChangeCallback)(int ch,
			enum TLStruct::ButtonState oldState,
			enum TLStruct::ButtonState newState);

	private:
		bool useCustomScanOrder;
		bool anyButtonIsApproached;
		bool anyButtonIsPressed;

		uint16_t crcUpdate(uint16_t crc, unsigned char c);

		/* 
		 * Older versions of EEPROM library don't have length(). Add
		 * method here for compatibility.
		 */
		uint16_t EEPROM_length();

		/* 
		 * Older versions of EEPROM library don't have update(). Add
		 * method here for compatibility.
		 */
		void EEPROM_update(int addr, uint8_t b);

		uint16_t eepromSizeRequired(void);
		float readFloatFromEeprom(int * addr, uint16_t * crc);
		void writeFloatToEeprom(float f, int * addr, uint16_t * crc);
		void readSensorSettingFromEeprom(int n, int * addr, 
			uint16_t * crc, bool applySettings);
		void writeSensorSettingToEeprom(int n, int * addr, 
			uint16_t * crc);
		void readSettingsFromEeprom(void);
		int8_t addChannel(uint8_t ch);
		void addSample(uint8_t ch, int32_t sample);
		bool isPressed(TLStruct * d);
		bool isApproached(TLStruct * d);
		bool isReleased(TLStruct * d);
		void updateAvg(TLStruct * d);
		void processStatePreCalibrating(uint8_t ch);
		void processStateCalibrating(uint8_t ch);
		void processStateNoisePowerMeasurement(uint8_t ch);
		void processStateReleased(uint8_t ch);
		void processStateReleasedToApproached(uint8_t ch);
		void processStateApproached(uint8_t ch);
		void processStateApproachedToPressed(uint8_t ch);
		void processStatePressed(uint8_t ch);
		void processStatePressedToApproached(uint8_t ch);
		void processStateApproachedToReleased(uint8_t ch);
		void correctSample(uint8_t ch);
		void processSample(uint8_t ch);
		void updateNCharges(uint8_t ch);
		void resetButtonStateSummaries(uint8_t ch);
		void initScanOrder(void);

		/* These strings are for human readability */
		const char * const buttonStateLabels[TLStruct::buttonStateMax +
				1] = {
			"PreCalibrating", "Calibrating",
			"NoisePowerMeasurement", "Released",
			"ReleasedToApproached", "Approached",
			"ApproachedToPressed", "ApproachedToReleased",
			"Pressed", "PressedToApproached", "Invalid"
		};
};

/* Actual implementation */
#define TL_RELEASED_TO_APPROACHED_THRESHOLD_DEFAULT		50.0
#define TL_APPROACHED_TO_RELEASED_THRESHOLD_DEFAULT		40.0
#define TL_APPROACHED_TO_PRESSED_THRESHOLD_DEFAULT		150.0
#define TL_PRESSED_TO_APPROACHED_THRESHOLD_DEFAULT		120.0
#define TL_RELEASED_TO_APPROACHED_TIME_DEFAULT			10
#define TL_APPROACHED_TO_RELEASED_TIME_DEFAULT			10
#define TL_APPROACHED_TO_PRESSED_TIME_DEFAULT			10
#define TL_PRESSED_TO_APPROACHED_TIME_DEFAULT			10
#define TL_ENABLE_SLEWRATE_LIMITER_DEFAULT			false
#define TL_PRE_CALIBRATION_TIME_DEFAULT			100
#define TL_CALIBRATION_TIME_DEFAULT				500
#define TL_FILTER_COEFF_DEFAULT				128
#define TL_APPROACHED_TIMEOUT_DEFAULT				300000
#define TL_PRESSED_TIMEOUT_DEFAULT				TL_APPROACHED_TIMEOUT_DEFAULT
#define TL_FORCE_CALIBRATION_WHEN_RELEASING_FROM_APPROACHED_DEFAULT	0
#define TL_FORCE_CALIBRATION_WHEN_APPROACHING_FROM_RELEASED_DEFAULT	0
#define TL_FORCE_CALIBRATION_WHEN_APPROACHING_FROM_PRESSED_DEFAULT	0
#define TL_FORCE_CALIBRATION_WHEN_PRESSING_DEFAULT		0
#define TL_USE_CUSTOM_SCAN_ORDER_DEFAULT			false
#define TL_ADC_RESOLUTION_BIT					10
#define TL_ADC_MAX						((1 << TL_ADC_RESOLUTION_BIT) - 1)
#define TL_N_CHARGES_MIN_DEFAULT				1
#define TL_N_CHARGES_MAX_DEFAULT				1
#define TL_USE_N_CHARGES_PADDING_DEFAULT			true
#define TL_CHARGE_DELAY_SENSOR_DEFAULT				0
#define TL_CHARGE_DELAY_ADC_DEFAULT				0
#define TL_ENABLE_TOUCH_STATE_MACHINE_DEFAULT			true
#define TL_ENABLE_NOISE_POWER_MEASUREMENT_DEFAULT		false

#define TL_REFERENCE_CAPACITANCE_DEFAULT			((float) 15) /* 15 pF */
#define TL_CAPACITANCE_SCALE_FACTOR_DEFAULT			((float) 1)
#define TL_PARALLEL_CAPACITANCE_DEFAULT			((float) 1000) /* pF */

#define TL_DISTANCE_SCALE_FACTOR_DEFAULT			((float) 1)
#define TL_DISTANCE_OFFSET_DEFAULT				((float) 0)

#define TL_PERMITTIVITY_VACUUM					((float) (8.85E-12*1E12/1E3))
#define TL_RELATIVE_PERMITTIVITY_DEFAULT			((float) 1)
#define TL_AREA_DEFAULT					(10*10) /* 100 mm^2 */

#define TL_SET_PARALLEL_CAPACITANCE_MANUALLY_DEFAULT		false
#define TL_DISABLE_UPDATE_IF_ANY_BUTTON_IS_APPROACHED_DEFAULT	true
#define TL_DISABLE_UPDATE_IF_ANY_BUTTON_IS_PRESSED_DEFAULT	true
#ifdef EEPROM_h
#define TL_ENABLE_READ_SETTINGS_FROM_EEPROM_DEFAULT		true
#else
#define TL_ENABLE_READ_SETTINGS_FROM_EEPROM_DEFAULT		false
#endif
#define TL_EEPROM_OFFSET_DEFAULT				0
#define TL_EEPROM_KEY						0xC7
#define TL_EEPROM_FORMAT_VERSION				0
#define TL_EEPROM_FORMAT_MASK					0x7
#define TL_EEPROM_FORMAT_SHIFT					5
#define TL_EEPROM_N_SENSORS_MASK				0x1F
#define TL_EEPROM_N_SENSORS_SHIFT				0
#define TL_EEPROM_CONFIG_ENABLE_SLEWRATE_LIMITER		0x80
#define TL_SAMPLE_METHOD_RESISTIVE_GND_PIN			2
#define TL_SAMPLE_METHOD_RESISTIVE_USE_INTERNAL_PULLUP		true

#define TL_SAMPLE_METHOD_DEFAULT				(&TLSampleMethodTL)

/*
 * EEPROM overhead:
 * 1 byte key
 * 1 byte description (EEPROM format version + nSensors)
 * 1 byte config
 * 2 byte CRC
 */
#define TL_EEPROM_N_BYTES_OVERHEAD				(1+1+1+2)

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
int8_t TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::addChannel(uint8_t ch)
{
	long r;
	uint16_t n, pos, length;
	int8_t err = -1;

	length = ((uint16_t) nSensors) * ((uint16_t) nMeasurementsPerSensor);
	
	r = random(0, length);

	for (n = 0; n < length; n++) {
		pos = (n + r) % length;
		if (scanOrder[pos] == 255) {
			scanOrder[pos] = ch;
			err = 0;
			break;
		}
	}

	error = err;
	return err;
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::initScanOrder(void)
{
	uint16_t pos, length;
	uint8_t n, k;

	length = ((uint16_t) nSensors) * ((uint16_t) nMeasurementsPerSensor);

	for (pos = 0; pos < length; pos++) {
		scanOrder[pos] = 255;
	}
	
	/*
	 * Use a fixed seed so that scan order is pseudo random but always the
	 * same.
	 */
	randomSeed(nMeasurementsPerSensor);

	for (k = 0; k < nMeasurementsPerSensor; k++) {
		for (n = 0; n < nSensors; n++) {
			addChannel(n);
		}
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
int8_t TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::setDefaults(void)
{
	uint8_t n;
	
	error = 0;

	if (nSensors < 2) {
		error = -1;
	}

	if (error == 0) {
		this->anyButtonIsApproached = false;
		this->anyButtonIsPressed = false;
	}

	if (error == 0) {
		this->useCustomScanOrder = TL_USE_CUSTOM_SCAN_ORDER_DEFAULT;
	
		if (useCustomScanOrder == false) {
			initScanOrder();
		}
	}

	if (error == 0) {
		this->enableReadSettingsFromEeprom =
			TL_ENABLE_READ_SETTINGS_FROM_EEPROM_DEFAULT;
		this->eepromOffset = TL_EEPROM_OFFSET_DEFAULT;
		buttonStateChangeCallback = NULL;
	}

	if (error == 0) {
		for (n = 0; n < nSensors; n++) {
			data[n].pin = A0 + n;
			data[n].direction = TLStruct::directionPositive;
			data[n].sampleType = TLStruct::sampleTypeDifferential;
			data[n].releasedToApproachedThreshold =
				TL_RELEASED_TO_APPROACHED_THRESHOLD_DEFAULT;
			data[n].approachedToReleasedThreshold =
				TL_APPROACHED_TO_RELEASED_THRESHOLD_DEFAULT;
			data[n].approachedToPressedThreshold =
				TL_APPROACHED_TO_PRESSED_THRESHOLD_DEFAULT;
			data[n].pressedToApproachedThreshold =
				TL_PRESSED_TO_APPROACHED_THRESHOLD_DEFAULT;
			data[n].releasedToApproachedTime =
				TL_RELEASED_TO_APPROACHED_TIME_DEFAULT;
			data[n].approachedToReleasedTime =
				TL_APPROACHED_TO_RELEASED_TIME_DEFAULT;
			data[n].approachedToPressedTime =
				TL_APPROACHED_TO_PRESSED_TIME_DEFAULT;
			data[n].pressedToApproachedTime =
				TL_PRESSED_TO_APPROACHED_TIME_DEFAULT;
			data[n].enableSlewrateLimiter = 
				TL_ENABLE_SLEWRATE_LIMITER_DEFAULT;
			data[n].preCalibrationTime =
				TL_PRE_CALIBRATION_TIME_DEFAULT;
			data[n].calibrationTime =
				TL_CALIBRATION_TIME_DEFAULT;
			data[n].filterCoeff =
				TL_FILTER_COEFF_DEFAULT;
			data[n].approachedTimeout =
				TL_APPROACHED_TIMEOUT_DEFAULT;
			data[n].pressedTimeout =
				TL_PRESSED_TIMEOUT_DEFAULT;
			data[n].forceCalibrationWhenReleasingFromApproached =
				TL_FORCE_CALIBRATION_WHEN_RELEASING_FROM_APPROACHED_DEFAULT;
			data[n].forceCalibrationWhenApproachingFromReleased =
				TL_FORCE_CALIBRATION_WHEN_APPROACHING_FROM_RELEASED_DEFAULT;
			data[n].forceCalibrationWhenApproachingFromPressed =
				TL_FORCE_CALIBRATION_WHEN_APPROACHING_FROM_PRESSED_DEFAULT;
			data[n].forceCalibrationWhenPressing =
				TL_FORCE_CALIBRATION_WHEN_PRESSING_DEFAULT;
			data[n].enableTouchStateMachine = 
				TL_ENABLE_TOUCH_STATE_MACHINE_DEFAULT;
			data[n].enableNoisePowerMeasurement =
				TL_ENABLE_NOISE_POWER_MEASUREMENT_DEFAULT;
			data[n].parallelCapacitance =
				TL_PARALLEL_CAPACITANCE_DEFAULT;
			data[n].referenceCapacitance =
				TL_REFERENCE_CAPACITANCE_DEFAULT;
			data[n].capacitanceScaleFactor =
				TL_CAPACITANCE_SCALE_FACTOR_DEFAULT;
			data[n].distanceScaleFactor =
				TL_DISTANCE_SCALE_FACTOR_DEFAULT;
			data[n].relativePermittivity =
				TL_RELATIVE_PERMITTIVITY_DEFAULT;
			data[n].distanceOffset = TL_DISTANCE_OFFSET_DEFAULT;
			data[n].area = TL_AREA_DEFAULT;
			data[n].setParallelCapacitanceManually =
				TL_SET_PARALLEL_CAPACITANCE_MANUALLY_DEFAULT;
			data[n].disableUpdateIfAnyButtonIsApproached =
				TL_DISABLE_UPDATE_IF_ANY_BUTTON_IS_APPROACHED_DEFAULT;
			data[n].disableUpdateIfAnyButtonIsPressed =
				TL_DISABLE_UPDATE_IF_ANY_BUTTON_IS_PRESSED_DEFAULT;
			data[n].stateIsBeingChanged = false;
			data[n].sampleMethod = TL_SAMPLE_METHOD_DEFAULT;
			data[n].sampleMethodResistive_gndPin =
				TL_SAMPLE_METHOD_RESISTIVE_GND_PIN + n;
			data[n].sampleMethodResistive_useInternalPullup =
				TL_SAMPLE_METHOD_RESISTIVE_USE_INTERNAL_PULLUP;
			if (!data[n].setParallelCapacitanceManually) {
				/*
				 * Set parallelCapacitance to 0; will be updated
				 * after calibration.
				 */
				data[n].parallelCapacitance = 0;
			}
		}
	}

	return error;
}

/**
 * \file
 * Functions and types for CRC checks.
 *
 * Generated on Sun Jun 25 21:01:32 2017
 * by pycrc v0.9, https://pycrc.org
 * using the configuration:
 *  - Width         = 16
 *  - Poly          = 0x1021
 *  - XorIn         = 0x1d0f
 *  - ReflectIn     = False
 *  - XorOut        = 0x0000
 *  - ReflectOut    = False
 *  - Algorithm     = bit-by-bit-fast
 */

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
uint16_t TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::crcUpdate(uint16_t
		crc, unsigned char c)
{
	unsigned int i;
	bool bit;

	for (i = 0x80; i > 0; i >>= 1) {
		bit = crc & 0x8000;
		if (c & i) {
			bit = !bit;
		}
		crc <<= 1;
		if (bit) {
			crc ^= 0x1021;
		}
	}
	crc &= 0xffff;
	return crc & 0xffff;
}

/* 
 * Older versions of EEPROM library don't have length(). Add function here for
 * compatibility.
 */
template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
uint16_t TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::EEPROM_length(void)
{
	#ifdef EEPROM_h
	return E2END + 1;
	#else
	return 0;
	#endif
}

/* 
 * Older versions of EEPROM library don't have update(). Add function here for
 * compatibility.
 */
template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::EEPROM_update(int
		addr, uint8_t b)
{
	#ifdef EEPROM_h
	if (EEPROM.read(addr) != b) {
		EEPROM.write(addr, b);
	}
	#endif
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
float TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::readFloatFromEeprom(int
		* addr, uint16_t * crc)
{
	#ifdef EEPROM_h
	float f;
	uint32_t i = 0;
	int k;
	uint8_t tmp;

	for (k = sizeof(float); k > 0; k--) {
		tmp = EEPROM.read(*addr);
		*crc = crcUpdate(*crc, tmp);
		*addr = *addr + 1;
		i |= (((uint32_t) tmp) << ((k - 1) << 3));
	}

	memcpy(&f, &i, sizeof(float));

	return f;
	#else
	return 0;
	#endif
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::writeFloatToEeprom(float f,
		int * addr, uint16_t * crc)
{
	#ifdef EEPROM_h
	int k;
	uint32_t i;
	uint8_t tmp;

	memcpy(&i, &f, sizeof(float));

	for (k = sizeof(float); k > 0; k--) {
		tmp = (i >> ((k - 1) << 3)) & 0xFF;
		*crc = crcUpdate(*crc, tmp);
		EEPROM_update(*addr, tmp);
		*addr = *addr + 1;
	}
	#endif
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::readSensorSettingFromEeprom(int n, 
		int * addr, uint16_t * crc, bool applySettings)
{
	#ifdef EEPROM_h
	if (applySettings) {
		data[n].releasedToApproachedThreshold =
			readFloatFromEeprom(addr, crc);

		data[n].approachedToReleasedThreshold =
			readFloatFromEeprom(addr, crc);

		data[n].approachedToPressedThreshold =
			readFloatFromEeprom(addr, crc);

		data[n].pressedToApproachedThreshold =
			readFloatFromEeprom(addr, crc);
	} else {
		readFloatFromEeprom(addr, crc);
		readFloatFromEeprom(addr, crc);
		readFloatFromEeprom(addr, crc);
		readFloatFromEeprom(addr, crc);
	}
	#endif
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::writeSensorSettingToEeprom(int n, 
		int * addr, uint16_t * crc)
{
	#ifdef EEPROM_h
	float f;

	f = data[n].releasedToApproachedThreshold;
	writeFloatToEeprom(f, addr, crc);
	
	f = data[n].approachedToReleasedThreshold;
	writeFloatToEeprom(f, addr, crc);
	
	f = data[n].approachedToPressedThreshold;
	writeFloatToEeprom(f, addr, crc);
	
	f = data[n].pressedToApproachedThreshold;
	writeFloatToEeprom(f, addr, crc);
	#endif
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
uint16_t TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::eepromSizeRequired(void)
{
	return nSensors * 4 * sizeof(float) + TL_EEPROM_N_BYTES_OVERHEAD;
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::writeSettingsToEeprom(void)
{
	#ifdef EEPROM_h
	int addr = eepromOffset;
	int n;
	uint16_t crc = 0;
	uint8_t tmp;

	if (((nSensors - 1) & TL_EEPROM_N_SENSORS_MASK) != (nSensors - 1)) {
		error = -28; /* not enough space; return ENOSPC */
	}

	if (eepromOffset + eepromSizeRequired() > EEPROM_length()) {
		error = -28; /* not enough space; return ENOSPC */
	}

	tmp = EEPROM.read(addr);
	if ((error == 0) && (tmp != TL_EEPROM_KEY) && (tmp != 0xFF)) {
		error = -5; /* key not found and not empty; return EIO */
	}

	if (error == 0) {
		tmp = TL_EEPROM_KEY;
		EEPROM_update(addr++, tmp);
		crc = crcUpdate(crc, tmp);

		tmp = (TL_EEPROM_FORMAT_VERSION << TL_EEPROM_FORMAT_SHIFT) |
			(((nSensors - 1) & TL_EEPROM_N_SENSORS_MASK) << 
			TL_EEPROM_N_SENSORS_SHIFT);
		EEPROM_update(addr++, tmp);
		crc = crcUpdate(crc, tmp);

		for (n = 0; n < nSensors; n++) {
			writeSensorSettingToEeprom(n, &addr, &crc);
		}

		EEPROM_update(addr++, (crc >> 8) & 0xFF);
		EEPROM_update(addr++, crc & 0xFF);
	}
	#endif
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::readSettingsFromEeprom(void)
{
	#ifdef EEPROM_h
	int addr = eepromOffset;
	int tmpAddr = eepromOffset + 2;
	int n;
	uint16_t crc = 0, crcEeprom = 0;
	uint8_t tmp;
	uint8_t formatVersion;
	uint8_t nSensorsEeprom;
	uint8_t config = 0;
	bool b;

	if (((nSensors - 1) & TL_EEPROM_N_SENSORS_MASK) != (nSensors - 1)) {
		error = -28; /* not enough space; return ENOSPC */
	}

	if (eepromOffset + eepromSizeRequired() > EEPROM_length()) {
		error = -28; /* not enough space; return ENOSPC */
	}

	tmp = EEPROM.read(addr++);
	crc = crcUpdate(crc, tmp);
	if ((error == 0) && (tmp != TL_EEPROM_KEY)) {
		error = -5; /* key not found; return EIO */
	}

	if (error == 0) {
		tmp = EEPROM.read(addr++);
		crc = crcUpdate(crc, tmp);
		formatVersion = ((tmp >> TL_EEPROM_FORMAT_SHIFT) &
			TL_EEPROM_FORMAT_MASK);
		nSensorsEeprom = ((tmp >> TL_EEPROM_N_SENSORS_SHIFT) &
			TL_EEPROM_N_SENSORS_MASK) + 1;

		config = EEPROM.read(addr++);
		crc = crcUpdate(crc, tmp);

		if (formatVersion != TL_EEPROM_FORMAT_VERSION) {
			error = -5; /* incorrect version; return EIO */
		}

		if (nSensorsEeprom != nSensors) {
			error = -5; /* incorrect EEPROM setting; return EIO */
		}
	}

	if (error == 0) {
		tmpAddr = addr;

		/* First do a dummy read since we haven't verified CRC yet */
		for (n = 0; n < nSensors; n++) {
			readSensorSettingFromEeprom(n, &addr, &crc, false);
		}

		tmp = EEPROM.read(addr++);
		crcEeprom = (((uint16_t) tmp) << 8);
		tmp = EEPROM.read(addr++);
		crcEeprom |= (uint16_t) tmp;

		if (crc != crcEeprom) {
			error = -5; /* CRC error; return EIO */
		}
	}

	if (error == 0) {
		addr = tmpAddr;

		/* CRC is valid; read again but now do apply settings */
		for (n = 0; n < nSensors; n++) {
			readSensorSettingFromEeprom(n, &addr, &crc, true);
		}

		/* Apply settings from config */
		if (config & TL_EEPROM_CONFIG_ENABLE_SLEWRATE_LIMITER) {
			b = true;
		} else {
			b = false;
		}
		for (n = 0; n < nSensors; n++) {
			data[n].enableSlewrateLimiter = b;
		}
	}
	#endif
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::~TLSensors(void)
{
	/* Nothing to destroy */
}

#warning overload constructor with uint8_t customScanOrder[]
template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::TLSensors(void)
{
	uint8_t n;
	unsigned long now;
	
	error = 0;

	if (N_SENSORS < 2) {
		error = -1;
	} else {
		nSensors = N_SENSORS;
	}

	if ((error == 0) && (N_MEASUREMENTS_PER_SENSOR >= 1)) {
		nMeasurementsPerSensor = N_MEASUREMENTS_PER_SENSOR;
	} else {
		error = -1;
	}

	if (error == 0) {
		initScanOrder();
	}

	if (error == 0) {
		setDefaults();
	}

	if (error == 0) {
		now = millis();

		for (n = 0; n < nSensors; n++) {
			resetButtonStateSummaries(n);
			setState(n, TLStruct::buttonStatePreCalibrating);
			data[n].buttonStateLabel =
				this->buttonStateLabels[data[n].buttonState];
			data[n].counter = 0;
			data[n].raw = 0;
			data[n].capacitance = 0;
			data[n].avg = 0;
			data[n].noisePower = 0;
			data[n].delta = 0;
			data[n].nCharges = TL_N_CHARGES_MIN_DEFAULT;
			data[n].nChargesNext = TL_N_CHARGES_MIN_DEFAULT;
			data[n].nChargesMin = TL_N_CHARGES_MIN_DEFAULT;
			data[n].nChargesMax = TL_N_CHARGES_MAX_DEFAULT;
			data[n].useNChargesPadding =
				TL_USE_N_CHARGES_PADDING_DEFAULT;
			data[n].chargeDelaySensor = TL_CHARGE_DELAY_SENSOR_DEFAULT;
			data[n].chargeDelayADC = TL_CHARGE_DELAY_ADC_DEFAULT;
			data[n].stateChangedAtTime = now;
			data[n].lastSampledAtTime = 0;
		}
	}

	if ((error == 0) && (this->enableReadSettingsFromEeprom)) {
		readSettingsFromEeprom();
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::addSample(uint8_t ch, int32_t sample)
{
	if (data[ch].enableSlewrateLimiter) {
		if (data[ch].slewrateFirstSample) {
			data[ch].raw = sample;
			data[ch].slewrateFirstSample = false;
		} else {
			if (sample > data[ch].raw) {
				data[ch].raw++;
			} 
			if (sample < data[ch].raw) {
				data[ch].raw--;
			}
		}
	} else {
		data[ch].raw += sample;
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
bool TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::isReleased(TLStruct * d)
{
	bool isReleased = false;

	if ((d->direction == TLStruct::directionPositive) &&
			(d->delta <= d->approachedToReleasedThreshold)) {
		isReleased = true;
	}
	if ((d->direction == TLStruct::directionNegative) &&
			(-d->delta <= d->approachedToReleasedThreshold)) {
		isReleased = true;
	}

	return isReleased;
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
bool TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::isApproached(TLStruct * d)
{
	bool isApproached = false;

	if ((d->direction == TLStruct::directionPositive) &&
			(d->delta >= d->releasedToApproachedThreshold)) {
		isApproached = true;
	}
	if ((d->direction == TLStruct::directionNegative) &&
			(-d->delta >= d->releasedToApproachedThreshold)) {
		isApproached = true;
	}

	return isApproached;
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
bool TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::isPressed(TLStruct * d)
{
	bool isPressed = false;

	if ((d->direction == TLStruct::directionPositive) &&
			(d->delta >= d->approachedToPressedThreshold)) {
		isPressed = true;
	}
	if ((d->direction == TLStruct::directionNegative) &&
			(-d->delta >= d->approachedToPressedThreshold)) {
		isPressed = true;
	}

	return isPressed;
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::updateAvg(TLStruct * d)
{
	float s;

	if (d->disableUpdateIfAnyButtonIsApproached &&
			this->anyButtonIsApproached) {
		return;
	}
	if (d->disableUpdateIfAnyButtonIsPressed &&
			this->anyButtonIsPressed) {
		return;
	}

	d->avg = (d->counter * d->avg + d->capacitance) / (d->counter + 1);

	/* Only perform noise measurement when not calibrating any more */
	if ((d->enableNoisePowerMeasurement) && (d->buttonState >
			TLStruct::buttonStateCalibrating)) {
		s = d->delta * d->delta;
		d->noisePower = (d->counter * d->noisePower + s) / 
			(d->counter + 1);
	}

	if (d->counter < d->filterCoeff - 1) {
		d->counter++;
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
bool TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::setForceCalibratingStates(
		int ch, uint32_t mask, enum TLStruct::ButtonState * newState)
{
	int n;
	bool chStateChanged = false;

	for (n = 0; n < N_SENSORS; n++) {
		if (mask & (1 << n)) {
			if (n == ch) {
				chStateChanged = true;
				*newState = TLStruct::buttonStatePreCalibrating;
			} else {
				setState(n, TLStruct::buttonStatePreCalibrating);
			}
		}
	}

	return chStateChanged;
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
float TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::getRaw(int ch)
{
	TLStruct * d;

	d = &(data[ch]);

	return d->raw; 
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
float TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::getCapacitance(int ch)
{
	TLStruct * d;

	d = &(data[ch]);

	return d->capacitance; 
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
float TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::getDelta(int ch)
{
	TLStruct * d;

	d = &(data[ch]);

	return d->delta; 
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
float TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::getAvg(int ch)
{
	TLStruct * d;

	d = &(data[ch]);

	return d->avg; 
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
const char * TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::getStateLabel(int
		ch)
{
	TLStruct * d;

	d = &(data[ch]);

	return d->buttonStateLabel; 
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
enum TLStruct::ButtonState TLSensors<N_SENSORS,
		N_MEASUREMENTS_PER_SENSOR>::getState(int ch)
{
	TLStruct * d;

	d = &(data[ch]);

	return d->buttonState; 
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::setState(int ch,
		enum TLStruct::ButtonState newState)
{
	bool setStateChangedAtTime = true;
	uint32_t mask = 0;
	enum TLStruct::ButtonState oldState;
	TLStruct * d;

	d = &(data[ch]);

	if (d->stateIsBeingChanged) {
		/*
		 * This button is already being changed; break circular
		 * reference.
		 */
		return;
	}

	/* 
	 * When switching from approachedToReleased back to approached or from
	 * pressedToApproached back to pressed, do not update
	 * stateChangedAtTime. If it would be updated, button could be
	 * erroneously in approached or pressed state and would never trigger a
	 * recalibration.
	 */
	if (((d->buttonState == TLStruct::buttonStateApproachedToReleased) &&
			newState == TLStruct::buttonStateApproached) ||
			((d->buttonState == TLStruct::buttonStatePressedToApproached) &&
			newState == TLStruct::buttonStatePressed)) {
		setStateChangedAtTime = false;
	}

	if (d->buttonState != newState) {
		d->stateIsBeingChanged = true;
		switch(newState) {
		case TLStruct::buttonStatePreCalibrating:
			break;
		case TLStruct::buttonStateCalibrating:
			d->counter = 0;
			d->avg = 0;
			d->noisePower = 0;
	
			if (!d->setParallelCapacitanceManually) {
				/*
				 * Set parallelCapacitance to 0; will be updated
				 * after calibration.
				 */
				d->parallelCapacitance = 0;
			}
			break;
		case TLStruct::buttonStateNoisePowerMeasurement:
			break;
		case TLStruct::buttonStateReleased:
			if (d->buttonState == 
					TLStruct::buttonStateApproachedToReleased) {
				mask = d->forceCalibrationWhenReleasingFromApproached;
			}
			break;
		case TLStruct::buttonStateReleasedToApproached:
			break;
		case TLStruct::buttonStateApproached:
			if (d->buttonState ==
					TLStruct::buttonStateReleasedToApproached) {
				mask = d->forceCalibrationWhenApproachingFromReleased;
			}
			if (d->buttonState ==
					TLStruct::buttonStatePressedToApproached) {
				mask = d->forceCalibrationWhenApproachingFromPressed;
			}
			break;
		case TLStruct::buttonStateApproachedToPressed:
			break;
		case TLStruct::buttonStateApproachedToReleased:
			break;
		case TLStruct::buttonStatePressed:
			mask = d->forceCalibrationWhenPressing;
			break;
		case TLStruct::buttonStatePressedToApproached:
			break;
		default:
			/* Error: illegal state */
			newState = TLStruct::buttonStatePreCalibrating;
		}

		if (mask) {
			setStateChangedAtTime |= setForceCalibratingStates(ch,
				mask, &newState);
		}

		if (setStateChangedAtTime) {
			d->stateChangedAtTime = d->lastSampledAtTime;
		}

		oldState = d->buttonState;
		d->buttonState = newState; 

		if (buttonStateChangeCallback != NULL) {
			(*buttonStateChangeCallback)(ch, oldState, newState);
		}
		d->stateIsBeingChanged = false;
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::processStatePreCalibrating(uint8_t ch)
{
	TLStruct * d;

	d = &(data[ch]);

	if (d->lastSampledAtTime - d->stateChangedAtTime >= d->preCalibrationTime) {
		setState(ch, TLStruct::buttonStateCalibrating);
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::processStateCalibrating(uint8_t ch)
{
	unsigned long t, t_max;
	TLStruct * d;

	d = &(data[ch]);

	t = d->lastSampledAtTime - d->stateChangedAtTime;
	t_max = d->calibrationTime;

	if (t < t_max) {
		updateAvg(d);
	} else {
		setState(ch, TLStruct::buttonStateNoisePowerMeasurement);
	
		if (!d->setParallelCapacitanceManually) {
			d->parallelCapacitance = d->avg;
		}
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::processStateNoisePowerMeasurement(uint8_t ch)
{
	unsigned long t, t_max;
	TLStruct * d;

	d = &(data[ch]);

	t = d->lastSampledAtTime - d->stateChangedAtTime;
	t_max = d->calibrationTime;

	if ((d->enableNoisePowerMeasurement) && (t < t_max)) {
		updateAvg(d);
	} else {
		setState(ch, TLStruct::buttonStateReleased);
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::processStateReleased(uint8_t ch)
{
	TLStruct * d;

	d = &(data[ch]);

	if ((d->enableTouchStateMachine) && (isApproached(d))) {
		setState(ch, TLStruct::buttonStateReleasedToApproached);
	} else {
		updateAvg(d);
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::processStateReleasedToApproached(uint8_t ch)
{
	TLStruct * d;

	d = &(data[ch]);

	/* Do not update average in this state. */

	if (!d->enableTouchStateMachine)
		return;

	if (isApproached(d)) {
		if (d->lastSampledAtTime - d->stateChangedAtTime >=
				d->releasedToApproachedTime) {
			setState(ch, TLStruct::buttonStateApproached);
		}
	} else {
		setState(ch, TLStruct::buttonStateReleased);
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::processStateApproached(uint8_t ch)
{
	TLStruct * d;

	d = &(data[ch]);

	if (!d->enableTouchStateMachine)
		return;

	if (isReleased(d)) {
		setState(ch, TLStruct::buttonStateApproachedToReleased);
	} else if (isPressed(d)) {
		setState(ch, TLStruct::buttonStateApproachedToPressed);
	} else if ((d->approachedTimeout > 0) && (d->lastSampledAtTime - 
			d->stateChangedAtTime > d->approachedTimeout)) {
		setState(ch, TLStruct::buttonStateCalibrating);
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::processStateApproachedToPressed(uint8_t ch)
{
	TLStruct * d;

	d = &(data[ch]);

	/* Do not update average in this state. */

	if (!d->enableTouchStateMachine)
		return;

	if (isPressed(d)) {
		if (d->lastSampledAtTime - d->stateChangedAtTime >=
				d->approachedToPressedTime) {
			setState(ch, TLStruct::buttonStatePressed);
		}
	} else {
		setState(ch, TLStruct::buttonStateApproached);
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::processStateApproachedToReleased(uint8_t ch)
{
	TLStruct * d;

	d = &(data[ch]);

	if (!d->enableTouchStateMachine)
		return;

	if (isReleased(d)) {
		if (d->lastSampledAtTime - d->stateChangedAtTime >=
				d->approachedToReleasedTime) {
			setState(ch, TLStruct::buttonStateReleased);
		}
	} else {
		setState(ch, TLStruct::buttonStateApproached);
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::processStatePressed(uint8_t ch)
{
	TLStruct * d;

	d = &(data[ch]);

	if (!d->enableTouchStateMachine)
		return;

	if (isPressed(d)) {
		if ((d->pressedTimeout > 0) && (d->lastSampledAtTime - 
				d->stateChangedAtTime > d->pressedTimeout)) {
			setState(ch, TLStruct::buttonStateCalibrating);
		}
	} else {
		setState(ch, TLStruct::buttonStatePressedToApproached);
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::processStatePressedToApproached(uint8_t ch)
{
	TLStruct * d;

	d = &(data[ch]);

	if (!d->enableTouchStateMachine)
		return;

	if (isPressed(d)) {
		setState(ch, TLStruct::buttonStatePressed);
	} else {
		if (d->lastSampledAtTime - d->stateChangedAtTime >= 
				d->pressedToApproachedTime) {
			setState(ch, TLStruct::buttonStateApproached);
		}
	}
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::processSample(uint8_t ch)
{
	TLStruct * d;

	d = &(data[ch]);

	d->delta = d->capacitance - d->avg;

	switch (d->buttonState) {
	case TLStruct::buttonStatePreCalibrating:
		processStatePreCalibrating(ch);
		break;
	case TLStruct::buttonStateCalibrating:
		processStateCalibrating(ch);
		break;
	case TLStruct::buttonStateNoisePowerMeasurement:
		processStateNoisePowerMeasurement(ch);
		break;
	case TLStruct::buttonStateReleased:
		processStateReleased(ch);
		break;
	case TLStruct::buttonStateReleasedToApproached:
		processStateReleasedToApproached(ch);
		break;
	case TLStruct::buttonStateApproached:
		processStateApproached(ch);
		break;
	case TLStruct::buttonStateApproachedToReleased:
		processStateApproachedToReleased(ch);
		break;
	case TLStruct::buttonStateApproachedToPressed:
		processStateApproachedToPressed(ch);
		break;
	case TLStruct::buttonStatePressed:
		processStatePressed(ch);
		break;
	case TLStruct::buttonStatePressedToApproached:
		processStatePressedToApproached(ch);
		break;
	default:
		/* Error! Illegal state! */
		processStateCalibrating(ch);
	}

	d->buttonStateLabel = this->buttonStateLabels[d->buttonState];
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::correctSample(uint8_t ch)
{
	TLStruct * d;
	float tmp, scale;

	d = &(data[ch]);

	if (d->enableSlewrateLimiter) {
		scale = (float) ((TL_ADC_MAX + 1) << 2);
	} else {
		scale = ((float) (nMeasurementsPerSensor << 1)) *
			((float) (TL_ADC_MAX + 1));
	}

	tmp = 1 - ((float) d->raw) / scale;
	tmp = pow(tmp, -((float) 1) / ((float) d->nCharges)) - ((float) 1);
	tmp = ((float) 1) / tmp;

	d->nChargesNext = (int32_t) (ceilf(tmp));
	if (d->nChargesNext < TL_N_CHARGES_MIN_DEFAULT) {
		d->nChargesNext = TL_N_CHARGES_MIN_DEFAULT;
	}
	if (d->nChargesNext > TL_N_CHARGES_MAX_DEFAULT) {
		d->nChargesNext = TL_N_CHARGES_MAX_DEFAULT;
	}

	tmp = scale * tmp * d->capacitanceScaleFactor /
		d->referenceCapacitance;
	d->capacitance = tmp;
	/* Capacitance can be negative due to noise! */

	d->distance = (d->distanceScaleFactor * TL_PERMITTIVITY_VACUUM *
		d->relativePermittivity * d->area / tmp) - d->distanceOffset;
	/* Distance can be negative due to noise! */
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::updateNCharges(uint8_t ch)
{
	TLStruct * d;

	d = &(data[ch]);
	d->nCharges = d->nChargesNext;
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::resetButtonStateSummaries(uint8_t ch)
{
	data[ch].buttonIsCalibrating = false;
	data[ch].buttonIsReleased = false;
	data[ch].buttonIsApproached = false;
	data[ch].buttonIsPressed = false;
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
int8_t TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::sample(void)
{
	uint16_t length, pos;
	uint8_t ch;
	int sample1 = 0, sample2 = 0;
	int32_t sum;
	unsigned long now;

	length = ((uint16_t) nSensors) * ((uint16_t)
		nMeasurementsPerSensor);

	for (ch = 0; ch < nSensors; ch++) {
		data[ch].raw = 0;
		data[ch].slewrateFirstSample = true;
	}

	for (pos = 0; pos < length; pos++) {
		if (data[scanOrder[pos]].sampleType &
				TLStruct::sampleTypeNormal) {
			ch = scanOrder[pos];
			sample1 = data[scanOrder[pos]].sampleMethod(data,
				nSensors, ch, false);
		}
		if (data[scanOrder[pos]].sampleType &
				TLStruct::sampleTypeInverted) {
			ch = scanOrder[pos];
			sample2 = TL_ADC_MAX - 
				data[scanOrder[pos]].sampleMethod(data,
				nSensors, ch, true);
		}

		/*
		 * For sampleTypeNormal and sampleTypeInverted: scale by factor
		 * 2 to get same amplitude as with sampleTypeDifferential.
		 */
		if (data[scanOrder[pos]].sampleType ==
				TLStruct::sampleTypeNormal) {
			sample1 = sample1 << 1;
		}
		if (data[scanOrder[pos]].sampleType ==
				TLStruct::sampleTypeInverted) {
			sample2 = sample2 << 1;
		}

		sum = sample1 + sample2;
		addSample(scanOrder[pos], sum);
	}
	
	now = millis();

	for (ch = 0; ch < nSensors; ch++) {
		data[ch].lastSampledAtTime = now;
		correctSample(ch);
		processSample(ch);
	}

	this->anyButtonIsApproached = false;
	this->anyButtonIsPressed = false;
	for (ch = 0; ch < nSensors; ch++) {
		resetButtonStateSummaries(ch);
		if (data[ch].buttonState <=
				TLStruct::buttonStateNoisePowerMeasurement) {
			data[ch].buttonIsCalibrating = true;
		}
		if ((data[ch].buttonState >= TLStruct::buttonStateReleased) &&
				(data[ch].buttonState <=
				TLStruct::buttonStateReleasedToApproached)) {
			data[ch].buttonIsReleased = true;
		}
		if (data[ch].buttonState >= TLStruct::buttonStateApproached) {
			data[ch].buttonIsApproached = true;
			this->anyButtonIsApproached = true;
		}
		if (data[ch].buttonState >= TLStruct::buttonStatePressed) {
			data[ch].buttonIsPressed = true;
			this->anyButtonIsPressed = true;
		}
		if (data[ch].sampleMethod == TLSampleMethodTL) {
			updateNCharges(ch);
		}
	}

	return error;
}

template <uint8_t N_SENSORS, uint8_t N_MEASUREMENTS_PER_SENSOR>
void TLSensors<N_SENSORS, N_MEASUREMENTS_PER_SENSOR>::printScanOrder(void)
{
	uint16_t n;

	for (n = 0; n < ((uint16_t) nSensors) * ((uint16_t)
			nMeasurementsPerSensor); n++) {
		Serial.print(scanOrder[n]);
		Serial.print(" ");
	}
	Serial.println();
}

#endif
