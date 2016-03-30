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
#include "VideoReader.h"
#include <opencv2/opencv.hpp>
#include "includes/logging.h"



//---------------------------Start print-------------------------------------------//
void VideoReader::print_raw (std::ostream &os, bool pr_as_read_syntax) { //const {
   LOGGING ("VideoReader::print_raw(<ostream>, %s)\n", BOOL_TO_STR (pr_as_read_syntax) );

   os << "Summary of Video Reader Object";

   newline (os); newline (os);

   int    w  = (int)m_oVC.get (CV_CAP_PROP_FRAME_WIDTH);
   int    h  = (int)m_oVC.get (CV_CAP_PROP_FRAME_HEIGHT);
   double f  =      m_oVC.get (CV_CAP_PROP_FPS);
   int    fc = (int)m_oVC.get (CV_CAP_PROP_FRAME_COUNT);


   os << "\tName: " << m_sFilename; newline (os);
   os << "\tResolution: " << w << "x" << h << "@" << f; newline (os);
   os << "\tDuration: ";


#ifdef FFMPEG_HACK
   cv::CvCapture_FFMPEG *cap = m_oVC.getFFmpegCap ();
   if (!(!cap || !cap->ic) ) {
      char s[20];
      double duration = (double)cap->ic->duration;
      int msec = duration / 1000;
      int sec  = msec / 1000;
      int min  = sec / 60;
      int hour = min / 60;
      snprintf (s, 20, "%02i:%02i:%02i.%03i", hour, min % 60, sec % 60, msec % 1000);
      os << s << " / ";
   }
#endif
   os << fc << " Frames"; newline (os);

   newline (os);
}//end Fct



void VideoReader::print (std::ostream &os, bool pr_as_read_syntax) { //const {
   LOGGING ("VideoReader::print(<ostream>, %s)\n", BOOL_TO_STR (pr_as_read_syntax) );
   print_raw (os, pr_as_read_syntax);
}//end Fct



void VideoReader::print_info (std::ostream &os, const std::string &prefix) { //const {
   LOGGING ("VideoReader::print_info(<ostream>, %s)\n", prefix);
   print (os);
}//end Fct



bool VideoReader::print_name_tag (std::ostream &os, const std::string &name) { //const {
   LOGGING ("VideoReader::print_name_tag(<ostream>, %s)\n", name);
   print_raw (os);
   return true;
}//end Fct



void VideoReader::print_with_name (std::ostream &os, const std::string &name, bool print_padding) { //const {
   LOGGING ("VideoReader::print_with_name(<ostream>, %s, %s)\n", name, BOOL_TO_STR (print_padding) );

   os << name << " ="; newline (os);

   print_raw (os);
}//end Fct

