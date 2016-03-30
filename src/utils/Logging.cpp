
//---------------------------Includes----------------------------------------------//
#include <string>
#include <iostream>
#include <stdarg.h>

#include "Logging.h"
#include "backtrace/boost/backtrace.hpp"



//---------------------------Defines-----------------------------------------------//
#ifdef CONSOLEDEBUG
// Plot nothing if verbose is set to debug
#define SILENTDEBUG   if (s_iDebugLevel == s_iVerbose) return;
#define SILENTDEBUG2   if (s_iDebugLevel == s_iVerbose) return *this;
#else
#define SILENTDEBUG
#define SILENTDEBUG2
#endif

int                        utils::Logging::s_iVerbose       = 1;
int                        utils::Logging::s_iDebugLevel    = 255;
bool                       utils::Logging::s_bVerboseColors = false;
bool                       utils::Logging::s_bWarnAsErr     = false;
std::string                utils::Logging::s_oProgramName   = std::string();
const utils::Logging::Endl utils::Logging::endl             = utils::Logging::Endl::NONE;



//---------------------------Namespace utils---------------------------------------//
namespace utils {



//---------------------------Start info--------------------------------------------//
void Logging::info (const std::string &message) {
   if (m_iLogLevel > s_iVerbose) return;
   SILENTDEBUG

   if (s_bVerboseColors)
      std::cout << "\033[01;32m" << s_oProgramName << message << "\033[00m" << std::endl;
   else
      std::cout << s_oProgramName << message << std::endl;

   if (m_oLogFile.is_open () )
      m_oLogFile << s_oProgramName << message << '\n';
}//end Fct



void Logging::info (const char message[]) {
   if (m_iLogLevel > s_iVerbose) return;
   SILENTDEBUG

   if (s_bVerboseColors)
      std::cout << "\033[01;32m" << s_oProgramName << message << "\033[00m" << std::endl;
   else
      std::cout << s_oProgramName << message << std::endl;

   if (m_oLogFile.is_open () )
      m_oLogFile << s_oProgramName << message << '\n';
}//end Fct



//---------------------------Start warning-----------------------------------------//
void Logging::warning (const std::string &message) {
   if (s_iVerbose == 0) return;

   std::ostream *wout = &std::cout;
   if (s_bWarnAsErr) wout = &std::cerr;

   if (s_bVerboseColors)
      *wout << "\033[01;30m" << s_oProgramName << "warning: " << message << "\033[00m" << std::endl;
   else
      *wout << s_oProgramName << "warning: " << message << std::endl;

   if (m_oLogFile.is_open () )
      m_oLogFile << s_oProgramName << "warning: " << message << '\n';
}//end Fct



void Logging::warning (const char message[]) {
   if (s_iVerbose == 0) return;

   std::ostream *wout = &std::cout;
   if (s_bWarnAsErr) wout = &std::cerr;

   if (s_bVerboseColors)
      *wout << "\033[01;30m" << s_oProgramName << "warning: " << message << "\033[00m" << std::endl;
   else
      *wout << s_oProgramName << "warning: " << message << std::endl;

   if (m_oLogFile.is_open () )
      m_oLogFile << s_oProgramName << "warning: " << message << '\n';
}//end Fct



//---------------------------Start error-------------------------------------------//
void Logging::error (const std::string &message) {
   if (s_bVerboseColors)
      std::cerr << "\033[01;31m" << s_oProgramName << "error: " << message << "\033[00m" << std::endl;
   else
      std::cerr << s_oProgramName << "error: " << message << std::endl;

   if (m_oLogFile.is_open () )
      m_oLogFile << s_oProgramName << "error: " << message << std::endl;
}//end Fct



void Logging::error (const char message[]) {
   if (s_bVerboseColors)
      std::cerr << "\033[01;31m" << s_oProgramName << "error: " << message << "\033[00m" << std::endl;
   else
      std::cerr << s_oProgramName << "error: " << message << std::endl;

   if (m_oLogFile.is_open () )
      m_oLogFile << s_oProgramName << "error: " << message << std::endl;
}//end Fct



//---------------------------Start trace-------------------------------------------//
void Logging::trace (const char message[]) {
   boost::runtime_error re (message);

   if (s_bVerboseColors) {
      std::cerr << "\n" "\033[01;31m" << s_oProgramName << "trace: " << message << "\n" "Backtrace:\n";
      re.trace (std::cerr);
      std::cerr << "\033[00m" << std::endl;
   } else {
      std::cerr << "\n" << s_oProgramName << "trace: " << message << "\n" "Backtrace:\n";
      re.trace (std::cerr);
      std::cerr.flush ();
   }

   if (m_oLogFile.is_open () ) {
      m_oLogFile << "\n" << s_oProgramName << "trace: " << message << "\n" "Backtrace:\n";
      re.trace (m_oLogFile);
      m_oLogFile.flush ();
      //m_oLogFile << s_oProgramName << "error: " << message << std::endl;
   }
}//end Fct



//---------------------------Start printf------------------------------------------//
void Logging::printf (const char message[], ...) {
   if (m_iLogLevel > s_iVerbose) return;
   SILENTDEBUG

   char buffer[512];
   va_list args;
   va_start (args, message);
   vsnprintf (buffer, 512, message, args);
   va_end (args);

   //if (s_bVerboseColors)
   //   std::cout << "\033[01;32m" << s_oProgramName << buffer << "\033[00m" << std::endl;
   //else
      std::cout << s_oProgramName << buffer << std::endl;

   if (m_oLogFile.is_open () )
      m_oLogFile << s_oProgramName << buffer << '\n';
}//end Fct



//---------------------------Start operator<<--------------------------------------//
Logging &Logging::operator<< (const std::string &message) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (const char message[]) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (char *message) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct


/*
template<class T>
Logging &Logging::operator<< (T number) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::string message = std::to_string (number);
   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct

template<class T> Logging& Logging::operator<< <int>(T);
template<class T> Logging& Logging::operator<< (long);
template<class T> Logging& Logging::operator<< (long long);

template<> Logging& Logging::operator<< <const int>(const int);
template<> Logging& Logging::operator<< <const long>(const long);
template<> Logging& Logging::operator<< <const long long>(const long long);

template<> Logging& Logging::operator<< <unsigned int>(const unsigned int);
template<> Logging& Logging::operator<< <unsigned long>(const unsigned long);
template<> Logging& Logging::operator<< <unsigned long long>(const unsigned long long);

template<> Logging& Logging::operator<< <float>(const float);
template<> Logging& Logging::operator<< <double>(const double);
template<> Logging& Logging::operator<< <long double>(const long double);
*/


Logging &Logging::operator<< (const bool flag) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::string message = flag?"true":"false";
   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (const int number) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::string message = std::to_string (number);
   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (const long number) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::string message = std::to_string (number);
   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (const long long number) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::string message = std::to_string (number);
   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (const unsigned int number) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::string message = std::to_string (number);
   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (const unsigned long number) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::string message = std::to_string (number);
   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (const unsigned long long number) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::string message = std::to_string (number);
   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (const float number) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::string message = std::to_string (number);
   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (const double number) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::string message = std::to_string (number);
   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (const long double number) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::string message = std::to_string (number);
   std::cout << message;
   if (m_oLogFile.is_open () )
      m_oLogFile << message;

   return *this;
}//end Fct



Logging &Logging::operator<< (const Endl &) {
   if (m_iLogLevel > s_iVerbose) return *this;
   SILENTDEBUG2

   std::cout << std::endl;
   if (m_oLogFile.is_open () )
      m_oLogFile << std::endl;

   return *this;
}//end Fct



//---------------------------Start colorLog----------------------------------------//
std::string Logging::colorLog (std::string message) {
   if (!s_bVerboseColors)
      return message;

   return std::string () + "\033[01;32m" + message + "\033[00m";
}//end Fct



//---------------------------Start colorError--------------------------------------//
std::string Logging::colorError (std::string message) {
   if (!s_bVerboseColors)
      return message;

   return std::string () + "\033[01;31m" + message + "\033[00m";
}//end Fct



//---------------------------Start openLogFile-------------------------------------//
bool Logging::openLogFile (const std::string &filename) {
   if (!filename.size () )
      return false;

   if (m_oLogFile.is_open () )
      m_oLogFile.close ();

   m_oLogFile.open (filename, std::ofstream::out);

   if (m_oLogFile.is_open () ) {
      m_oFilename = filename;
      return true;
   } else {
      m_oFilename.clear ();
      return false;
   }
}//end Fct

};//end namespace

