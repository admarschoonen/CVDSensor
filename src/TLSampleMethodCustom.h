/*
 * TLSampleMethodCustom.h - Custom sensing implementation for TouchLibrary for
 * Arduino
 *
 * https://github.com/AdmarSchoonen/TLSensor
 * Copyright (c) 2016 - 2017 Admar Schoonen
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

#ifndef TLSampleMethodCustom_h
#define TLSampleMethodCustom_h

#include <TouchLib.h>

struct TLStructSampleMethodCustom {
	int pin;
};

int TLSampleMethodCustomPreSample(struct TLStruct * data, uint8_t nSensors,
	uint8_t ch) __attribute__ ((weak));

int32_t TLSampleMethodCustomSample(struct TLStruct * data, uint8_t nSensors,
	uint8_t ch, bool inv) __attribute__ ((weak));

int TLSampleMethodCustomPostSample(struct TLStruct * data, uint8_t nSensors,
	uint8_t ch) __attribute__ ((weak));

int32_t TLSampleMethodCustomMapDelta(struct TLStruct * data, uint8_t nSensors,
                uint8_t ch, int length) __attribute__ ((weak));

int TLSampleMethodCustom(struct TLStruct * data, uint8_t nSensors,
	uint8_t ch) __attribute__ ((weak));

#endif