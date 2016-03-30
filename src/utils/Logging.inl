
//---------------------------Includes----------------------------------------------//
#include <iostream>



//---------------------------Defines-----------------------------------------------//
#ifdef CONSOLEDEBUG
// Plot nothing if verbose is set to debug
#define SILENTDEBUG   if (s_iDebugLevel == s_iVerbose) return;
#else
#define SILENTDEBUG
#endif



//---------------------------Namespace utils---------------------------------------//
namespace utils {



//---------------------------Start log---------------------------------------------//
void Logging::log (const std::string &message) {
   if (m_iLogLevel > s_iVerbose) return;
   SILENTDEBUG

   std::cout << s_oProgramName << message << '\n';
   if (m_oLogFile.is_open () )
      m_oLogFile << s_oProgramName << message << '\n';
}//end Fct



void Logging::log (const char message[]) {
   if (m_iLogLevel > s_iVerbose) return;
   SILENTDEBUG

   std::cout << s_oProgramName << message << '\n';
   if (m_oLogFile.is_open () )
      m_oLogFile << s_oProgramName << message << '\n';
}//end Fct



//---------------------------Start debug-------------------------------------------//
#ifdef CONSOLEDEBUG
void Logging::debug (const std::string &message) {
   if (s_iDebugLevel > s_iVerbose) return;

   std::cout << s_oProgramName << message << std::endl;
   if (m_oLogFile.is_open () )
      m_oLogFile << s_oProgramName << message << std::endl;
}//end Fct



void Logging::debug (const char message[]) {
   if (s_iDebugLevel > s_iVerbose) return;

   std::cout << s_oProgramName << message << std::endl;
   if (m_oLogFile.is_open () )
      m_oLogFile << s_oProgramName << message << std::endl;
}//end Fct
#else
// Remove all 'debug' messages from binary (anti-reverse-engineering)
void Logging::debug (const std::string &message) {}
void Logging::debug (const char message[]) {}
#endif



//---------------------------Start operator[]--------------------------------------//
Logging& Logging::operator[] (const int loglevel) {
   m_iLogLevel = loglevel;

   return *this;
}

};//end namespace

