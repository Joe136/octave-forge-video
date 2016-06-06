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
#include "lib/VideoReader.h"
#include "includes/logging.h"



//---------------------------Start VideoReader-------------------------------------//
DEFUN_DLD(VideoReader, args, nargout,
"-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{obj} =} VideoReader (@var{filename} [, @var{N}])\n\
   Read video files.\n\
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

   VideoReader *video_ = nullptr;

   // Create VideoReader (dependent on input type (string <--> number) )
   if (args(0).is_string () )
      video_ = new VideoReader (args(0).string_value () );
   else if (args(0).is_scalar_type () ) {
      if (args(0).is_integer_type () )
         video_ = new VideoReader (args(0).int_value () );
      else if (args(0).is_double_type () )
         video_ = new VideoReader (args(0).double_value () );
   } else
      video_ = new VideoReader ();   // This will create a non-valid VideoReader

   video_->setConfig (VideoReader::VR_SilentRead, true);
   video_->setConfig (VideoReader::VR_ZeroImage,  false);

   retval (0) = octave_value (video_);
   return retval;
}//end Fct

