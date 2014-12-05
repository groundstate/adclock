//
// adclock - a program for driving an analog time display on the Beaglebone
//
// The MIT License (MIT)
//
// Copyright (c) 2014  Michael J. Wouters
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include "GPIO.h"
#include "TLV5625.h"

TLV5625::TLV5625(unsigned int DATAGPIO,unsigned int CLKGPIO,unsigned int CSGPIO)
{
	dataGPIO= new GPIO(DATAGPIO,GPIO::OUTPUT);
	clkGPIO = new GPIO(CLKGPIO,GPIO::OUTPUT);
	csGPIO  = new GPIO(CSGPIO,GPIO::OUTPUT);
	
	csGPIO->set(GPIO::HIGH); // disable inputs to the DAC
	clkGPIO->set(GPIO::LOW);
}

TLV5625::~TLV5625()
{
	delete dataGPIO;
	delete clkGPIO;
	delete csGPIO;
}
		
void TLV5625::writeDACs(unsigned short int dacA,unsigned short int dacB)
{
	DACA=dacA;
	DACB=dacB;
	
	// 16 bit data word
	// D15-D12 are control bits, D11-D4 data, D3-D0 unused
	// SLOW mode is used (settling time 10us)
	
	unsigned short int vala=(DACA << 4) | REG1;
	unsigned short int valb=(DACB << 4) | REG0;
	
	// no need for any delays between writes - setup times are tens of ns
	csGPIO->set(GPIO::LOW); // falling edge on CS reqd
	for (int i=0;i<16;i++){ // write data for DACB to BUFFER
			if (0x8000 & valb)
				dataGPIO->set(GPIO::HIGH);
			else
				dataGPIO->set(GPIO::LOW);
			valb = valb << 1;
			clkGPIO->set(GPIO::HIGH);
			clkGPIO->set(GPIO::LOW);
	}
	
	// one more clock to update the DAC output
	clkGPIO->set(GPIO::HIGH);
	clkGPIO->set(GPIO::LOW);
	csGPIO->set(GPIO::HIGH);
	
	csGPIO->set(GPIO::LOW);
	for (int i=0;i<16;i++){ // write data to DACA and update B from BUFFER
			if (0x8000 & vala)
				dataGPIO->set(GPIO::HIGH);
			else
				dataGPIO->set(GPIO::LOW);
			vala = vala<< 1;
			clkGPIO->set(GPIO::HIGH);
			clkGPIO->set(GPIO::LOW);
	}
	
	clkGPIO->set(GPIO::HIGH);
	clkGPIO->set(GPIO::LOW);
	csGPIO->set(GPIO::HIGH);
	
}
