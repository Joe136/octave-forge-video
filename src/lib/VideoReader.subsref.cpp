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



//---------------------------Start subsref-----------------------------------------//
octave_value_list VideoReader::subsref (const std::string &type, const std::list< octave_value_list > &idx, int nargout) {
   LOGGING ("VideoReader::subsref(\"%s\", <idx>[%lu], %i)\n", type.c_str (), idx.size (), nargout);   //TODO 'type' and 'idx' could be nullptr

   octave_value_list retval;

   // Variables
   if (idx.size () == 1 && type.length () >= 1 && type[0] == '.') {
      std::string  s = idx.front ()(0).string_value ();
      std::transform(s.begin(), s.end(), s.begin(), ::tolower);

      if (!s.compare ("numberofframes") )
         retval(0) = octave_value (m_oVC.get (CV_CAP_PROP_FRAME_COUNT) );

      else if (!s.compare ("height") )
         retval(0) = octave_value (m_oVC.get (CV_CAP_PROP_FRAME_HEIGHT) );

      else if (!s.compare ("width") )
         retval(0) = octave_value (m_oVC.get (CV_CAP_PROP_FRAME_WIDTH) );

      else if (!s.compare ("framerate") )
         retval(0) = octave_value (m_oVC.get (CV_CAP_PROP_FPS) );

      else if (!s.compare ("userdata") )
         retval(0) = octave_value (m_oUserData);

      else if (!s.compare ("tag") )
         retval(0) = octave_value (m_sTag);

      else if (!s.compare ("currenttime") )
         retval(0) = octave_value (m_oVC.get (CV_CAP_PROP_POS_MSEC) / 1000);
         //warning ("VideoReader: 'currenttime' not implemented yet");

      else if (!s.compare ("duration") ) {
#ifdef FFMPEG_HACK
         cv::CvCapture_FFMPEG *cap = m_oVC.getFFmpegCap ();
         if (!cap || !cap->ic)
            retval(0) = octave_value (-1);
         else {
            retval(0) = octave_value ( ( (double)cap->ic->duration) / 1000000.0f);
         }
#else
         warning ("VideoReader: 'duration' not implemented yet");
#endif

      } else if (!s.compare ("time") ) {
#ifdef FFMPEG_HACK
         cv::CvCapture_FFMPEG *cap = m_oVC.getFFmpegCap ();
         if (!cap || !cap->ic)
            retval(0) = octave_value (-1);
         else {
            char s[20];
            double duration = (double)cap->ic->duration;
            int msec = duration / 1000;
            int sec  = msec / 1000;
            int min  = sec / 60;
            int hour = min / 60;
            snprintf (s, 20, "%02i:%02i:%02i.%03i", hour, min % 60, sec % 60, msec % 1000);
            retval(0) = octave_value (std::string (s) );
         }
#else
         warning ("VideoReader: 'time' not implemented yet");
#endif

      } else if (!s.compare ("path") )
         retval(0) = octave_value (m_sFilename);

      else if (!s.compare ("iscamera") )
         retval(0) = octave_value (m_bIsCamera);

      else if (!s.compare ("isvalid") )
         retval(0) = octave_value (m_bIsValid);

      else if (!s.compare ("name") )
         warning ("VideoReader: 'name' not implemented yet");

      else if (!s.compare ("bitsperpixel") )
         warning ("VideoReader: 'bitsperpixel' not implemented yet");

      else if (!s.compare ("type") )
         retval(0) = octave_value (std::string ("VideoReader") );
         //warning ("VideoReader: 'type' not implemented yet");

      else if (!s.compare ("videoformat") )
         retval(0) = octave_value (fourccToString (m_oVC.get (CV_CAP_PROP_FOURCC) ) );   //TODO this is not the same as matlab says, it's the image type: e.g. 'RGB24'
         //warning ("VideoReader: 'videoformat' not implemented yet");

      else if (!s.compare ("fourcc") )
         retval(0) = octave_value (fourccToString (m_oVC.get (CV_CAP_PROP_FOURCC) ) );

      else
         error ("VideoReader: unknown VideoReader property %s", s.c_str () );

   // Functions
   } else if (idx.size () == 2 && type.length () >= 1 && type[0] == '.' && type[1] == '(') {
      std::string s = idx.front ()(0).string_value ();
      std::transform(s.begin(), s.end(), s.begin(), ::tolower);

      if (!s.compare ("read") ) {
         octave_value_list list = *(++ idx.begin () );
         int  from   = 1;
         int  to     = m_oVC.get (CV_CAP_PROP_FRAME_COUNT);   //TODO this call doesn't work
         bool native = false;

         if (list.length () > 0) {
            //for (int i = 0; i < list.length(); ++i) {
            //   list(i).print_info (octave_stdout);
            //}//end for

            if (list(0).is_real_matrix () ) {
               Matrix m = list(0).matrix_value ();

               if (m.length () >= 1) {
                  from = (int)m(0);

                  if (m.length () >= 2)
                     to = (int)m(1);
                  else
                     to = from;
               } else {
                  error ("VideoReader: argument 1 is an empty vector");
                  return retval;
               }

            } else if (list(0).is_scalar_type () ) {
               from = (int)list(0).scalar_value ();
               to   = from;
            }

            for (int i = 0; i < list.length(); ++i) {
               if (list(i).is_char_matrix () ) {
                  std::string ss = list(i).char_matrix_value ().row_as_string (0);
                  std::transform(ss.begin(), ss.end(), ss.begin(), ::tolower);

                  if (!ss.compare ("native") )
                     native = true;
               }
            }//end for
         }

         retval(0) = read (from, to, native);

      } else if (!s.compare ("readframe") ) {
         retval(0) = read ();

      } else if (!s.compare ("hasframe") ) {
         warning ("VideoReader: 'hasFrame' not implemented yet");

      } else if (!s.compare ("get") ) {
         warning ("VideoReader: 'get' not implemented yet");

      } else if (!s.compare ("set") ) {
         octave_value_list list = *(++ idx.begin () );

         if (list.length () >= 2 && list.length () % 2 == 0) {
            boolNDArray resMap = boolNDArray (dim_vector (list.length () / 2, 1) );

            for (int i = 0, n = 0; i < list.length(); i += 2, ++n) {
               if (list(i).is_char_matrix () )
                  resMap(n) = this->set (list(i).char_matrix_value ().row_as_string (0), list(i + 1) );
               else
                  error ("VideoReader: argument %i (should be a parameter name) is not a string", i);
            }//end for

            retval(0) = resMap;
         } else
            error ("VideoReader: uneven count of arguments");

//      } else if (!s.compare ("codecs") ) {
//         retval(0) = octave_value (m_oVC.print_codecs () );

      } else if (!s.compare ("getfileformats") ) {
         //retval(0) = octave_value (m_oVC.print_file_formats () );
         warning ("VideoReader: 'getfileformats' not implemented yet");

      } else if (!s.compare ("fullcheck") ) {
         retval(0) = fullCheck ();

      } else
         error ("VideoReader: unknown VideoReader method '%s'", s.c_str () );
   }

   return retval;
}//end Fct

