//
// adclock -  a program for driving an analog time display on the Beaglebone
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

#ifndef __ADCLOCK_H_
#define __ADCLOCK_H_

#include <fstream>
#include <string>

using namespace std;

class TLV5625;
class TiXmlElement;

class ADClock{
	
	public:
		ADClock(int,char **);
		~ADClock();
		
		void log(string);
		void debug(string);
		void run();
		
	private:
		
		enum TestMode {NO_TEST,ZERO_DACS,FSD_DACS,RAMP_DACS};
		
		static void signalHandler(int sig);
		
		void init(int,char **);
		string getConfigPath();
		bool readConfig(string);
		void readDACconfig(TiXmlElement*,double *,double *);
		bool getInt(const char *, int *,string);
		bool getDouble(const char *, double *,string);
		
		void showHelp();
		void showVersion();
		void showLicense();
		void runDACtest(std::string, TLV5625 *, int );
		void waitForKeyPress();
		
		string timestamp();
		
		int  testMode;
		
		string logFileName;
		string debugFileName;
		
		ofstream  appLog;
		ofstream  debugLog;
		
		string timezone;
		
		TLV5625 *hrsDAC,*minsDAC,*secsDAC;
		double hrsUnitsCal,hrsTensCal,minsUnitsCal,minsTensCal,secsUnitsCal,secsTensCal;
};

#endif