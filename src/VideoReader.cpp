/* Copyright (C) 2014 Joe136 <Joe136@users.noreply.github.com>

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
#include "logging.h"



//---------------------------Start Constructor-------------------------------------//
VideoReader::VideoReader (void) : octave_class (octave_map (), "VideoReader", std::list<std::string> () ) {
   LOGGING ("VideoReader::VideoReader()\n");
}//end Constructor



VideoReader::VideoReader (std::string filename) : octave_class (octave_map (), "VideoReader", std::list<std::string> () ) {
   LOGGING ("VideoReader::VideoReader(\"%s\")\n", filename.c_str () );

   m_sFilename = filename;

   m_oVC.open    (m_sFilename);
   //m_oVC.set_log (&octave_stdout);
   //m_oVC.set_err (&octave_stdout);

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



//---------------------------Start Destructor--------------------------------------//
VideoReader::~VideoReader () {
   LOGGING ("VideoReader::~VideoReader()\n");
}//end Destructor



//---------------------------Start subsref-----------------------------------------//
octave_value_list VideoReader::subsref (const std::string &type, const std::list< octave_value_list > &idx, int nargout) {
   LOGGING ("VideoReader::subsref(\"%s\", <idx>[%lu], %i)\n", type.c_str (), idx.size (), nargout);   //TODO 'type' and 'idx' could be nullptr

   octave_value_list retval;

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
         warning ("VideoReader: 'currenttime' not implemented yet");

      else if (!s.compare ("duration") )
         warning ("VideoReader: 'duration' not implemented yet");

      else if (!s.compare ("path") )
         retval(0) = octave_value (m_sFilename);

      else if (!s.compare ("isvalid") )
         retval(0) = octave_value (m_bIsValid);

      else if (!s.compare ("name") )
         warning ("VideoReader: 'name' not implemented yet");

      else if (!s.compare ("bitsperpixel") )
         warning ("VideoReader: 'bitsperpixel' not implemented yet");

      else if (!s.compare ("type") )
         warning ("VideoReader: 'type' not implemented yet");

      else if (!s.compare ("videoformat") )
         warning ("VideoReader: 'videoformat' not implemented yet");

      else
         error ("VideoReader: unknown VideoReader property %s", s.c_str () );

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

      } else if (!s.compare ("codecs") ) {
         //retval(0) = octave_value (m_oVC.print_codecs () );
         warning ("VideoReader: 'codecs' not implemented yet");

      } else if (!s.compare ("getfileformats") ) {
         //retval(0) = octave_value (m_oVC.print_file_formats () );
         warning ("VideoReader: 'getfileformats' not implemented yet");

      } else
         error ("VideoReader: unknown VideoReader method '%s'", s.c_str () );
   }

   return retval;
}//end Fct



//---------------------------Start set---------------------------------------------//
bool VideoReader::set (std::string type, const octave_value &value) {
   LOGGING ("VideoReader::set(%s, <value>)\n", type.c_str() );

   bool res = false;
   std::transform(type.begin(), type.end(), type.begin(), ::tolower);

   if (!type.compare ("zeroimage") ) {
      if (value.is_bool_type () )
         res = setConfig (VideoReader::VR_ZeroImage,  value.bool_value () );

   } else if (!type.compare ("silentread") ) {
      if (value.is_bool_type () )
         res = setConfig (VideoReader::VR_SilentRead, value.bool_value () );
   }

   return res;
}//end Fct



//---------------------------Start print_raw---------------------------------------//
void VideoReader::print_raw (std::ostream &os, bool pr_as_read_syntax) { //const {
   LOGGING ("VideoReader::print_raw(<ostream>, %s)\n", BOOL_TO_STR (pr_as_read_syntax) );

   os << "Summary of Video Reader Object";

   newline (os); newline (os);

   int    w = (int)m_oVC.get (CV_CAP_PROP_FRAME_WIDTH);
   int    h = (int)m_oVC.get (CV_CAP_PROP_FRAME_HEIGHT);
   double f =      m_oVC.get (CV_CAP_PROP_FPS);

   os << "\tName: " << m_sFilename; newline (os);
   os << "\tResolution: " << w << "x" << h << "@" << f; newline (os);

   newline (os);
}//end Fct



//---------------------------Start read--------------------------------------------//
/**
 *    from = 1-based index of the starting frame
 */
octave_value VideoReader::read (int from, int to, bool native) {
   LOGGING ("VideoReader::read(%i, %i, %s)\n", from, to, BOOL_TO_STR(native) );

   if (!m_bIsValid)
      return octave_value ();

   if (native)
      warning ("VideoReader.read: 'native' not implemented yet");

   if (from)
      m_iFrameNum = from;
   else
      from = m_iFrameNum;

   if (!to)
      to = from;

   if ((to - 1) > m_oVC.get (CV_CAP_PROP_FRAME_COUNT) )
      to = m_oVC.get (CV_CAP_PROP_FRAME_COUNT) + 1;

   if ( (from - 1) != (int)m_oVC.get (CV_CAP_PROP_POS_FRAMES) )
      m_oVC.set (CV_CAP_PROP_POS_FRAMES, (double)(from - 1) );

   dim_vector    d;
   cv::Mat       frame;
   unsigned int  height = (int)m_oVC.get (CV_CAP_PROP_FRAME_HEIGHT);
   unsigned int  width  = (int)m_oVC.get (CV_CAP_PROP_FRAME_WIDTH);
   unsigned int  depth  = 3;

   if (to > from) {
      d = dim_vector (height, width, depth, to - from + 1);
   } else {
      d = dim_vector (height, width, depth);
   }

   uint8NDArray image = uint8NDArray (d);

   if (m_bZeroImage)
      image.fill (0);

   octave_idx_type  posR  = 0;
   octave_idx_type  posG  = 0;
   octave_idx_type  posB  = 0;

   for (int i = 0; from <= to; ++from, ++i) {
      if (!m_oVC.read (frame) ) {
         error ("VideoReader: cannot read frame %d", from);
         break;
      }

      //pos   = ( ( ( (0) * (to-from) + i) * depth + <0|1|2>) * width + x) * height + y;
      posR  = i * (height * width * depth);
      posG  = posR + (height * width);
      posB  = posG + (height * width);

      for (unsigned int x = 0; x < width; ++x) {
         //int row   = 3 * x;
         //int pixel = row;

         for (unsigned int y = 0; y < height; ++y) {
            //pixel = y * frame->linesize[0] + 3 * x;

            image(posR++) = (unsigned int)frame.at<cv::Vec3b> (y, x)[2];
            image(posG++) = (unsigned int)frame.at<cv::Vec3b> (y, x)[1];
            image(posB++) = (unsigned int)frame.at<cv::Vec3b> (y, x)[0];

            //pixel += frame->linesize[0];
         }//end for 3
      }//end for 2
   }//end for 1

   m_iFrameNum = from;

   return image;
}//end Fct

