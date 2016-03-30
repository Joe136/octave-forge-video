
#pragma once


//---------------------------Includes----------------------------------------------//
#include <string>
#include <fstream>



//---------------------------Namespace utils---------------------------------------//
namespace utils {



//---------------------------Class Logging-----------------------------------------//
class Logging {
public:
   enum Endl { NONE };


public:// Constructors, Destructors
   Logging () : m_iLogLevel (1) {}


public:// Functions
   inline void log     (const std::string &message);
   inline void debug   (const std::string &message);
   void        info    (const std::string &message);
   void        warning (const std::string &message);
   void        error   (const std::string &message);

   inline void log     (const char message[]);
   inline void debug   (const char message[]);
   void        info    (const char message[]);
   void        warning (const char message[]);
   void        error   (const char message[]);

   void        trace   (const char message[]);
   void        printf  (const char message[], ...);


public:// Operators
   inline Logging &operator[] (const int loglevel);

   Logging        &operator<< (const std::string &message);
   Logging        &operator<< (const char message[]);
   Logging        &operator<< (char *message);

   Logging        &operator<< (const bool flag);

   Logging        &operator<< (const int number);
   Logging        &operator<< (const long number);
   Logging        &operator<< (const long long number);
   Logging        &operator<< (const unsigned int number);
   Logging        &operator<< (const unsigned long number);
   Logging        &operator<< (const unsigned long long number);
   Logging        &operator<< (const float number);
   Logging        &operator<< (const double number);
   Logging        &operator<< (const long double number);

   Logging        &operator<< (const Endl &);
/*
   template<class T>
   Logging        &operator<< (T number);
*/

public:// Static Functions
   static std::string colorLog (std::string message);
   static std::string colorError (std::string message);


public:// Functions
   bool openLogFile (const std::string &filename);


public:// Static Variables
   static int         s_iVerbose;
   static int         s_iDebugLevel;
   static bool        s_bVerboseColors;
   static bool        s_bWarnAsErr;
   static std::string s_oProgramName;
   static const Endl  endl;


private:
   std::ofstream m_oLogFile;
   std::string   m_oFilename;
   int           m_iLogLevel;


};//end class


};//end namespace

#include "Logging.inl"

