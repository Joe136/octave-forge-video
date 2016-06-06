/* Copyright (C) 2015 Joe136 <Joe136@users.noreply.github.com>

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */


//---------------------------Includes----------------------------------------------//
#include <octave/oct.h>
#include <octave/oct-map.h>
#include <octave/ov-struct.h>
#include "VideoReader.h"
#include <opencv2/opencv.hpp>
#include "includes/logging.h"



//---------------------------Start Constructor-------------------------------------//
VideoReader::VideoReader (void) : octave_class (octave_map (), "VideoReader", std::list<std::string> () ) {
   LOGGING ("VideoReader::VideoReader()\n");

   this->assign ("iscamera", octave_value (false) );
   this->assign ("isvalid",  octave_value (false) );
}//end Constructor



VideoReader::VideoReader (std::string filename) : octave_class (octave_map (), "VideoReader", std::list<std::string> () ) {
   LOGGING ("VideoReader::VideoReader(\"%s\")\n", filename.c_str () );

   m_sFilename = filename;

   m_oVC.open    (m_sFilename);
   //m_oVC.set_log (&octave_stdout);
   //m_oVC.set_err (&octave_stdout);

   m_oConsole.s_iVerbose = 10;
   this->assign ("iscamera", octave_value (false) );

   if (!m_oVC.isOpened() ) {
      //error ("aviread: AVHandler setup failed");
      warning ("VideoReader: OpenCV-VideoCapture setup failed");
      this->assign ("isvalid", octave_value (false) );
      return;
   } else {
      this->assign ("isvalid", octave_value (true) );
      m_bIsValid = true;
   }
}//end Constructor



VideoReader::VideoReader (int device) : octave_class (octave_map (), "VideoReader", std::list<std::string> () ) {
   LOGGING ("VideoReader::VideoReader(%i)\n", device);

   char filename[20];
   snprintf (filename, 20, "<:device:>%i", device);
   m_sFilename = filename;

   m_oVC.open    (device);
   //m_oVC.set_log (&octave_stdout);
   //m_oVC.set_err (&octave_stdout);

   m_oConsole.s_iVerbose = 10;

   if (!m_oVC.isOpened() ) {
      //error ("aviread: AVHandler setup failed");
      warning ("VideoReader: OpenCV-VideoCapture setup failed");
      this->assign ("isvalid",  octave_value (false) );
      this->assign ("iscamera", octave_value (false) );
      return;
   } else {
      this->assign ("isvalid",  octave_value (true) );
      this->assign ("iscamera", octave_value (true) );
      m_bIsValid  = true;
      m_bIsCamera = true;
   }
}//end Constructor



//---------------------------Start Destructor--------------------------------------//
VideoReader::~VideoReader () {
   LOGGING ("VideoReader::~VideoReader()\n");
}//end Destructor



//---------------------------Start set---------------------------------------------//
bool VideoReader::set (std::string type, const octave_value &value) {
   LOGGING ("VideoReader::set(%s, <value>)\n", type.c_str() );
   m_oConsole[40] << "VideoReader::set(\"" << type << "\", <value>)" << utils::Logging::endl;

   bool res = false;
   std::transform(type.begin(), type.end(), type.begin(), ::tolower);

   if (!type.compare ("zeroimage") ) {
      if (value.is_bool_type () )
         res = setConfig (VideoReader::VR_ZeroImage,  value.bool_value () );

   } else if (!type.compare ("silentread") ) {
      if (value.is_bool_type () )
         res = setConfig (VideoReader::VR_SilentRead, value.bool_value () );

   } else if (!type.compare ("preload") ) {
      if (value.is_bool_type () )
         res = setConfig (VideoReader::VR_PreLoad, value.bool_value () );

   } else if (!type.compare ("verbose") ) {
      if (value.is_double_type () )
         res = setConfig (VideoReader::VR_Verbose, value.double_value () * 10);
      else if (value.is_integer_type () )
         res = setConfig (VideoReader::VR_Verbose, value.int_value () * 10);
   }

   return res;
}//end Fct



//---------------------------Start fullCheck---------------------------------------//
octave_value VideoReader::fullCheck () {
   LOGGING ("VideoReader::fullCheck()\n");
   m_oConsole[40] << "VideoReader::fullCheck()" << utils::Logging::endl;

   Cell              status;
   uint32_t          pos = 0;
   octave_struct     summary;
   std::stringstream ss;

   m_oConsole[10] << "Initialize check" << utils::Logging::endl;

   //status.insert (summary, 0, pos++);

   summary.assign ("checked", octave_value(false) );
   summary.assign ("valid",   octave_value(false) );
   status.insert (octave_value (std::string ("Filename: ") + m_sFilename), 0, pos++);

   if (!m_bIsValid) {
      status.insert (octave_value (std::string ("VideoReader not valid") ), 0, pos++);
      m_oConsole[10] << "Check complete, errors occurred" << utils::Logging::endl;
      return status;
   }

   summary.assign ("valid",    octave_value(true) );
   summary.assign ("duration", octave_value(0) );

   int    w  = (int)m_oVC.get (CV_CAP_PROP_FRAME_WIDTH);
   int    h  = (int)m_oVC.get (CV_CAP_PROP_FRAME_HEIGHT);
   double f  =      m_oVC.get (CV_CAP_PROP_FPS);
   int    maxX = w;
   int    maxY = h;

   ss.str ("");
   ss << "Resolution: " << w << "x" << h << "@" << f;
   status.insert (octave_value (ss.str () ), 0, pos++);

   ss.str ("");
   ss << "Duration: " << (int)m_oVC.get (CV_CAP_PROP_FRAME_COUNT);
   status.insert (octave_value (ss.str () ), 0, pos++);

#ifdef FFMPEG_HACK
   cv::CvCapture_FFMPEG *cap = m_oVC.getFFmpegCap ();
   if (!(!cap || !cap->ic) ) {
      ss.str ("");
      ss << "FFmpeg Duration: " << std::fixed << (double)cap->ic->duration << std::scientific;
      status.insert (octave_value (ss.str () ), 0, pos++);
   }
#endif


   cv::Mat frame;
   int     from  = 0;
   int     to    = m_oVC.get (CV_CAP_PROP_FRAME_COUNT);
   bool    allOK = true;

   if (to == 0) {
      ss.str ("");
      ss << "It seems, there are no Frames";
      status.insert (octave_value (ss.str () ), 0, pos++);
      m_oConsole[10] << "Check complete, errors occurred" << utils::Logging::endl;
      return status;
   }


   m_oConsole[10] << "Check all frames" << utils::Logging::endl;


   for (from = 0; from < to; ++from) {
      m_iGrabbedFrameNum = from;
      if (!m_oVC.read (frame) ) {
         ss.str ("");
         ss << "Read error on frame " << from + 1;
         status.insert (octave_value (ss.str () ), 0, pos++);
         allOK = false;
         break;               //TODO continue till end of frames
      }

      if (frame.cols != w || frame.rows != h) {
         ss.str ("");
         ss << "Dimension mismatch on frame " << (from + 1) << ". [" << frame.cols << " " << frame.rows << "] ~= [" << w << " " << h << "]";
         status.insert (octave_value (ss.str () ), 0, pos++);
         allOK = false;
         maxX = (frame.cols>maxX)?frame.cols:maxX;
         maxY = (frame.rows>maxY)?frame.rows:maxY;
      }

   }//end for 1


   m_oConsole[10] << "Check complete";

   if (!allOK)
      m_oConsole[10] << ", errors occurred";

   m_oConsole[10] << utils::Logging::endl;

   summary.assign ("checked", octave_value(true) );
   summary.assign ("status", octave_value(allOK) );
   summary.assign ("maxX", octave_value(maxX) );
   summary.assign ("maxY", octave_value(maxY) );
   summary.assign ("duration", octave_value(from) );

   return status;
}//end Fct

