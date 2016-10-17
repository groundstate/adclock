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

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <cmath>
#include <cstdlib>
#include <iostream>

#include "ADClock.h"
#include "Debug.h"
#include "Version.h"
#include "tinyxml.h"
#include "TLV5625.h"

#define APP_NAME "adclock"
#define APP_VERSION "0.1"
#define APP_AUTHOR "Michael Wouters"
#define PRETTIFIER "*******************************************************"

extern ADClock* app;
extern ostream *debugStream;

ADClock::ADClock(int argc,char **argv)
{
	debugStream= NULL;
	testMode=NO_TEST;
	logFileName = string(APP_NAME) + ".log";
	int c;
	
	while ((c=getopt(argc,argv,"d:fhlrvz")) != -1){
		switch(c){
			case 'h':showHelp(); exit(EXIT_SUCCESS);
			case 'v':showVersion();exit(EXIT_SUCCESS);
			case 'd':
			{
				string dbgout = optarg;
				if ((string::npos != dbgout.find("stderr"))){
					debugStream = & std::cerr;
				}
				else{
					debugFileName = dbgout;
					debugLog.open(debugFileName.c_str(),ios_base::app);
					if (!debugLog.is_open()){
						cerr << "Unable to open " << dbgout << endl;
						exit(EXIT_FAILURE);
					}
					debugStream = & debugLog;
				}
				break;
			}
			case 'f':
			{
				testMode = FSD_DACS;
				break;
			}
			case 'l':
			{
				showLicense();
				exit(EXIT_SUCCESS);
			}
			case 'r':
			{
				testMode= RAMP_DACS;
				break;
			}
			case 'z':
			{
				testMode= ZERO_DACS;
				break;
			}
			default:
				break;
		}
	}
	
  init( argc, argv );
	
	timezone=":"+timezone; // set from file
	setenv("TZ",timezone.c_str(),1);
	tzset();
	
	secsDAC=new TLV5625(15,14,115);
	minsDAC=new TLV5625(49,3,2);
	//hrsDAC=new TLV5625(60,50,51); // this is for the PCB as designed
	hrsDAC=new TLV5625(30,50,51);   // and this is a hack for my dead GPIO
	
	
}

ADClock::~ADClock()
{
	log("exiting");
	appLog.close();
	debugLog.close();
	hrsDAC->writeDACs(0,0);
	minsDAC->writeDACs(0,0);
	secsDAC->writeDACs(0,0);
	delete hrsDAC;
	delete minsDAC;
	delete secsDAC;
	exit(EXIT_SUCCESS);
}

void ADClock::run()
{
	switch (testMode)
	{
		case ZERO_DACS:
			hrsDAC->writeDACs(0,0);
			minsDAC->writeDACs(0,0);
			secsDAC->writeDACs(0,0);
			waitForKeyPress();
			delete this;
			break;
		case FSD_DACS:
			hrsDAC->writeDACs(2*hrsTensCal,9*hrsUnitsCal);
			minsDAC->writeDACs(5*minsTensCal,9*minsUnitsCal);
			secsDAC->writeDACs(5*secsTensCal,9*secsUnitsCal);
			waitForKeyPress();
			delete this;
			break;
		case RAMP_DACS:
			runDACtest("Seconds - units",secsDAC,1);
			runDACtest("Seconds - tens",secsDAC,2);
			runDACtest("Minutes - units",minsDAC,1);
			runDACtest("Minutes - tens",minsDAC,2);
			runDACtest("Hours - units",hrsDAC,1);
			runDACtest("Hours - tens",hrsDAC,2);
			delete this;
			break;
		default:
			break;
	}
	
	 while (1){
		struct timeval tv;
		gettimeofday(&tv,NULL);
		usleep(1000000-tv.tv_usec);
		gettimeofday(&tv,NULL);
		struct tm *ltm = localtime(&tv.tv_sec);
		DBGMSG(debugStream,tv.tv_usec << ": " << ltm->tm_min << " " << ltm->tm_sec);
		//
		unsigned short int tens=ltm->tm_sec/10;
		unsigned short int units=ltm->tm_sec-10*tens;
		tens=tens*secsTensCal;
		units=units*secsUnitsCal;
		secsDAC->writeDACs(tens,units);
		
		tens=ltm->tm_min/10;
		units=ltm->tm_min-10*tens;
		tens=tens*minsTensCal;
		units=units*minsUnitsCal;
		minsDAC->writeDACs(tens,units);
		
		tens=ltm->tm_hour/10;
		units=ltm->tm_hour-10*tens;
		tens=tens*hrsTensCal;
		units=units*hrsUnitsCal;
		hrsDAC->writeDACs(tens,units);
		
		
	}
}

void ADClock::log(string msg)
{
	//appLog <<timestamp() << " " << msg << endl;
	//appLog.flush();
	DBGMSG(debugStream,"LOG:" << msg);
}

	
//
// Private members
//

void ADClock::init(int,char **)
{
	// Set defaults
	timezone = "UTC";
	
	/* Cal factor is (DAC value for max deflection) / (max digit on meter) */
	hrsUnitsCal=235.0/10.0;
	hrsTensCal=242.0/5.0;
	
	minsUnitsCal=235.0/10.0;
	minsTensCal=240.0/5.0;
	
	secsUnitsCal=235.0/10.0;
	secsTensCal=240.0/5.0;
	
	appLog.open(logFileName.c_str(),ios_base::app);
	if (!appLog.is_open()){
		cerr << "Unable to open " << logFileName << endl;
	}
	log(PRETTIFIER);
	log(string(APP_NAME) +  string(" v") + 
		string(APP_VERSION) + string(", last modified ") + string(LAST_MODIFIED));
	log(PRETTIFIER);
	
	string cfgPath=getConfigPath();
	if (cfgPath.size() > 0)
		readConfig(cfgPath);
	else
		cerr << "A configuration file could not be found" << endl;
	
	struct sigaction sa;
	sigset_t sigioset;
	
	sa.sa_handler = signalHandler;
	
	sigemptyset(&(sa.sa_mask)); // we block nothing 
	sa.sa_flags=0;    
	
	sigaddset(&sigioset,SIGTERM);
	if (sigaction(SIGTERM,&sa,NULL)==-1)
		perror("Wot?");
	
	sigaddset(&sigioset,SIGQUIT);
	sigaction(SIGQUIT,&sa,NULL);
	
	sigaddset(&sigioset,SIGINT);
	if (sigaction(SIGINT,&sa,NULL)==-1)
		perror("WotWot?");
	
	sigaddset(&sigioset,SIGHUP);
	sigaction(SIGHUP,&sa,NULL);
	
}

string ADClock::getConfigPath()
{

	struct stat statbuf;
	 
	std::string cfgPath("./adclock.xml");// working directory first
	if ((0 == stat(cfgPath.c_str(),&statbuf)))
		return cfgPath;
	
	char *homedir = getenv("HOME");
	cfgPath= string(homedir) +string("/adclock/adclock.xml");
	if ((0 == stat(cfgPath.c_str(),&statbuf)))
		return cfgPath;

	cfgPath= string(homedir) + string("/.adclock/adclock.xml");
	if ((0 == stat(cfgPath.c_str(),&statbuf)))
		return cfgPath;
	
	cfgPath= "/usr/local/share/adclock/adclock.xml";
	if ((0 == stat(cfgPath.c_str(),&statbuf)))
		return cfgPath;
	
	cfgPath= "/usr/share/adclock/adclock.xml";
	if ((0 == stat(cfgPath.c_str(),&statbuf)))
		return cfgPath;
	
	cfgPath="";
	
	return cfgPath;
}

bool ADClock::readConfig(string configPath)
{
	TiXmlDocument doc( configPath.c_str() );
  if (!doc.LoadFile()){
		cerr << "The configuration file could not be opened" << endl;
		return false;
	}
	
	log(string("using config file ") + configPath);
		
  TiXmlElement* root = doc.RootElement( );

  if (0 != strcmp(root->Value(), "adclock") ) {
		return false;
  }
	
	for ( TiXmlElement* elem = root->FirstChildElement();elem;elem = elem->NextSiblingElement() ){
		if (0 == strcmp(elem->Value(),"timezone")){
			const char *text = elem->GetText();
			if (text)
				timezone = text;
		}
		else if (0 == strcmp(elem->Value(),"hours")){
			readDACconfig(elem,&hrsUnitsCal,&hrsTensCal);
		}
		else if (0 == strcmp(elem->Value(),"minutes")){
			readDACconfig(elem,&minsUnitsCal,&minsTensCal);
		}
		else if (0 == strcmp(elem->Value(),"seconds")){
			readDACconfig(elem,&secsUnitsCal,&secsTensCal);
		}
	}
	
	return true;
}

void ADClock::readDACconfig(TiXmlElement* root,double *units,double *tens)
{
	for ( TiXmlElement* elem = root->FirstChildElement();elem;elem = elem->NextSiblingElement() ){
		if (0 == strcmp(elem->Value(),"unitscal"))
				getDouble(elem->GetText(),units,"Invalid value for unitscal");
		else if (0 == strcmp(elem->Value(),"tenscal"))
				getDouble(elem->GetText(),tens,"Invalid value for tenscal");
	}
}

bool ADClock::getInt(const char *txt,int *val,string msg){
	int tmp;
	if (txt){
		errno=0;
		char *endptr;
		tmp =  strtol(txt,&endptr,10); 
		if ((endptr==txt) || (errno == ERANGE && (tmp == HUGE_VALL || tmp == -HUGE_VALL)) || (errno != 0 && tmp == 0)) {
			log(msg);
			return false;
		}
		*val=tmp;
		return true;
	}
	return false;
}

bool ADClock::getDouble(const char *txt, double *val,string msg){
	double tmp;
	if (txt){
		errno=0;
		char *endptr;
		tmp =  strtod(txt,&endptr); 
		if ((endptr==txt) || (errno == ERANGE && (tmp == HUGE_VAL || tmp == -HUGE_VAL)) || (errno != 0 && tmp == 0)) {
			log(msg);
			return false;
		}
		*val=tmp;
		return true;
	}
	return false;
}

void ADClock::showHelp()
{
	cout << "Usage: " << APP_NAME << " [options]" << endl;
	cout << "Available options are" << endl;
	cout << "\t-d <file> Turn on debugging to <file> (use 'stderr' for output to stderr)" << endl;
	cout << "\t-h  Show this help" << endl;
	cout << "\t-f  Set DACs FSD" << endl;
	cout << "\t-l  Show license" << endl;
	cout << "\t-r  Ramp DACs" << endl;
	cout << "\t-v  Show version" << endl;
	cout << "\t-z  Set all DACS to zero" << endl;
}

void ADClock::showVersion()
{
	cout << APP_NAME << " v" <<APP_VERSION << ", last modified " << LAST_MODIFIED << endl;
	cout << "Written by " << APP_AUTHOR << endl;
	cout << "This ain't no stinkin' Perl script!" << endl;
}

void ADClock::showLicense()
{
	std::cout << APP_NAME << " - a program for driving an analog time display on the Beaglebone" << std::endl;
	std::cout <<  std::endl;
	std::cout << " The MIT License (MIT)" << std::endl;
	std::cout <<  std::endl;
	std::cout << " Copyright (c)  2014  Michael J. Wouters" << std::endl;
	std::cout <<  std::endl; 
	std::cout << " Permission is hereby granted, free of charge, to any person obtaining a copy" << std::endl;
	std::cout << " of this software and associated documentation files (the \"Software\"), to deal" << std::endl;
	std::cout << " in the Software without restriction, including without limitation the rights" << std::endl;
	std::cout << " to use, copy, modify, merge, publish, distribute, sublicense, and/or sell" << std::endl;
	std::cout << " copies of the Software, and to permit persons to whom the Software is" << std::endl;
	std::cout << " furnished to do so, subject to the following conditions:" << std::endl;
	std::cout << std::endl; 
	std::cout << " The above copyright notice and this permission notice shall be included in" << std::endl;
	std::cout << " all copies or substantial portions of the Software." << std::endl;
	std::cout << std::endl;
	std::cout << " THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR" << std::endl;
	std::cout << " IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY," << std::endl;
	std::cout << " FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE" << std::endl;
	std::cout << " AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER" << std::endl;
	std::cout << " LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM," << std::endl;
	std::cout << " OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN" << std::endl;
	std::cout << " THE SOFTWARE." << std::endl;
}

void ADClock::runDACtest(std::string msg, TLV5625 *dac, int chan)
{
	std::cout << msg << endl;
	for (int i=0;i<255;i++){
		if (chan==1)
			dac->writeDACs((unsigned short int) i,0);
		else
			dac->writeDACs(0,(unsigned short int) i);
		usleep(5000);
	}
	usleep(1000);
	for (int i=255;i>=0;i--){
		if (chan==1)
			dac->writeDACs((unsigned short int) i,0);
		else
			dac->writeDACs(0,(unsigned short int) i);
		usleep(5000);
	}
}

void ADClock::waitForKeyPress()
{
	cout << "Press ENTER to quit" << endl;
	cin.get();
}

string ADClock::timestamp()
{
	time_t tt =  time(0);
	struct tm *gmt = gmtime(&tt);
	char tc[128];
	
	strftime(tc,128,"%F %T",gmt);
	return std::string(tc);
}

void ADClock::signalHandler(int sig)
{
	
	switch (sig){
		case SIGTERM:
		case SIGQUIT:
		case SIGINT:
			delete app;
			break;
		case SIGHUP:
			//clearClients();
			break;
		default:
			cerr << "Unhandled signal " << sig << endl;
			break;
	}
}
