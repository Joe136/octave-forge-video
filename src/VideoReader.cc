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
#include "AVHandler.h"
#include "logging.h"



//---------------------------Start VideoReader-------------------------------------//
DEFUN_DLD(VideoReader, args, nargout,
"-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{obj} =} VideoReader (@var{filename} [, @var{N}])\n\
@end deftypefn\n\
\n\
@seealso{avifile, aviinfo, addframe}")
{
   LOGGING ("VideoReader(...)\n");

   octave_value_list retval;

   //if (args.length() == 0 || args.length () > 2) {
   if (args.length() < 1) {
      print_usage();
      return retval;
   }

   VideoReader *video_ = new VideoReader (args(0).string_value () );

   retval (0) = octave_value (video_);
   return retval;
}//end Fct



//---------------------------Start Constructor-------------------------------------//
VideoReader::VideoReader (void) : octave_class (octave_map (), "VideoReader", std::list<std::string> () ) {
   LOGGING ("VideoReader::VideoReader()\n");
}//end Constructor



VideoReader::VideoReader (std::string filename) : octave_class (octave_map (), "VideoReader", std::list<std::string> () ) {
   LOGGING ("VideoReader::VideoReader(\"%s\")\n", filename.c_str () );

   m_sFilename = filename;

   m_oAV.set_filename (m_sFilename);
   m_oAV.set_log (&octave_stdout);

   if (m_oAV.setup_read() != 0) {
      //error ("aviread: AVHandler setup failed");
      warning ("VideoReader: AVHandler setup failed");
      this->assign ("isvalid", octave_value (false) );
      return;
   } else
      m_bIsValid = true;
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
      octave_value t;

      if (!s.compare ("numberofframes") )
         t = octave_value (m_oAV.get_total_frames () );

      else if (!s.compare ("height") )
         t = octave_value (m_oAV.get_height () );

      else if (!s.compare ("width") )
         t = octave_value (m_oAV.get_width () );

      else if (!s.compare ("framerate") )
         t = octave_value (m_oAV.get_framerate () );

      else if (!s.compare ("userdata") )
         t = octave_value (m_oUserData);

      else if (!s.compare ("tag") )
         t = octave_value (m_sTag);

      else if (!s.compare ("path") )
         t = octave_value (m_sFilename);

      else if (!s.compare ("isvalid") )
         t = octave_value (m_bIsValid);

      //else if (!s.compare ("VideoFormat") )
      //   t = octave_value (m_oAV.print_file_formats () );

      else
         error ("VideoReader: unknown VideoReader property %s", s.c_str () );

      retval(0) = t;

   } else if (idx.size () == 2 && type.length () >= 1 && type[0] == '.' && type[1] == '(') {
      std::string s = idx.front ()(0).string_value ();
      std::transform(s.begin(), s.end(), s.begin(), ::tolower);

      if (!s.compare ("read") ) {
         octave_value_list list = *(++ idx.begin () );
         int  from   = 1;
         int  to     = m_oAV.get_total_frames ();   //TODO this call doesn't work
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

      } else if (!s.compare ("getfileformats") ) {
         warning ("VideoReader: 'getFileFormats' not implemented yet");

      } else if (!s.compare ("set") ) {
         warning ("VideoReader: 'set' not implemented yet");

      } else
         error ("VideoReader: unknown VideoReader method '%s'", s.c_str () );
   }

   return retval;
}//end Fct



//---------------------------Start print_raw---------------------------------------//
void VideoReader::print_raw (std::ostream &os, bool pr_as_read_syntax) const {
   LOGGING ("VideoReader::print_raw(<ostream>, %s)\n", BOOL_TO_STR (pr_as_read_syntax) );

   os << "Summary of Video Reader Object";

   newline (os); newline (os);

   os << "\tName: " << m_sFilename; newline (os);
   os << "\tResolution: " << m_oAV.get_width () << "x" << m_oAV.get_height () << "@" << m_oAV.get_framerate (); newline (os);

   newline (os);
}//end Fct



//---------------------------Start read--------------------------------------------//
octave_value VideoReader::read (int from, int to, bool native) {
   LOGGING ("VideoReader::read(%i, %i, %s)\n", from, to, BOOL_TO_STR(native) );

   if (!m_bIsValid)
      return octave_value ();

   if (native) {
      warning ("VideoReader.read: 'native' not implemented yet");
   }

   if (from)
      m_iFrameNum = from;
   else
      from = m_iFrameNum;

   if (!to)
      to = from;

   //if (to > m_oAV.get_total_frames () )
   //   to = m_oAV.get_total_frames ();

   dim_vector    d;
   AVFrame      *frame;
   unsigned int  height = m_oAV.get_height ();
   unsigned int  width  = m_oAV.get_width ();
   unsigned int  depth  = 3;

   if (to > from) {
      d = dim_vector (height, width, depth, to - from + 1);
   } else {
      d = dim_vector (height, width, depth);
   }

   uint8NDArray    image = uint8NDArray (d, 0);
   octave_idx_type posR  = 0;
   octave_idx_type posG  = 0;
   octave_idx_type posB  = 0;

   for (int i = 0; from <= to; ++from, ++i) {
      if (m_oAV.read_frame (from) != 0) {
         error ("VideoReader: cannot read frame %d", from);
         break;
      }

      //pos   = ( ( ( (0) * (to-from) + i) * depth + 0) * width + x) * height + y;
      posR  = i * (height * width * depth);
      posG  = posR + (height * width);
      posB  = posG + (height * width);
      frame = m_oAV.get_rgbframe ();

      for (unsigned int x = 0; x < width; ++x) {
         int row   = 3 * x;
         int pixel = row;

         for (unsigned int y = 0; y < height; ++y) {
            //pixel = y * frame->linesize[0] + row;

            image(posR++) = (unsigned int)frame->data[0][pixel + 2];
            image(posG++) = (unsigned int)frame->data[0][pixel + 1];
            image(posB++) = (unsigned int)frame->data[0][pixel + 0];

            pixel += frame->linesize[0];
         }//end for 3
      }//end for 2
   }//end for 1

   m_iFrameNum = from;

   return image;
}//end Fct

