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
#include <octave/ov-class.h>
#include "AVHandler.h"



//---------------------------Class VideoReader-------------------------------------//
class VideoReader : public octave_class {
public:// Constructors, Destructors
   VideoReader (void);
   VideoReader (std::string filename);

   ~VideoReader (void);

//   std::string class_name (void) const { return "VideoReader"; }


public:// Functions
   virtual octave_value_list subsref    (const std::string &type, const std::list< octave_value_list > &idx, int nargout);
   virtual void              print_raw  (std::ostream &os, bool pr_as_read_syntax=false) const;
   virtual size_t            byte_size  (void) const { return sizeof (VideoReader); }
   virtual Matrix            size       (void)       { return Matrix (1, 2, 1.0); }   // size = 1x1

   //virtual bool in_class_method (void) const { printf ("point5");return Matrix (1, 2, 1.0); }

   //virtual Array< std::string >  cellstr_value (void) const { printf ("point2"); }
   //virtual Cell   cell_value (void) const { printf ("point3"); }
   //virtual octave_function *  function_value (bool silent=false) { printf ("point4"); }


protected:// Functions
   virtual octave_value read (int from = 0, int to = 0, bool native = false);


protected:// Variables
   bool         m_bIsValid = false;
   std::string  m_sFilename;
   AVHandler    m_oAV = AVHandler ();

   octave_value m_oUserData;
   std::string  m_sTag;

   size_t       m_iFrameNum = 1;

};//end Class

