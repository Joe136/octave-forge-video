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
#include <octave/ov-class.h>
#include <opencv2/opencv.hpp>
#include "utils/Logging.h"

#ifdef FFMPEG_HACK
#include "includes/ffmpeg_hack.h"
#endif



//---------------------------Class VideoReader-------------------------------------//
class VideoReader : public octave_class {
public:// Enums
   enum ConfigType {
      VR_None = 0,
      VR_Verbose,
      VR_ZeroImage,
      VR_SilentRead,
      VR_PreLoad
   };//end Enum


   enum PreLoadState {
      PL_None = 0,
      PL_Stopped,
      PL_Deinitialize,
      PL_Initialize,
      PL_Sarted
   };//end Enum


   // Hijack OpenCV-VideoCapture class
   class VideoCapture_ : public cv::VideoCapture {
   public:
      VideoCapture_ ()                           : cv::VideoCapture () {}
      VideoCapture_ (const cv::String& filename) : cv::VideoCapture (filename) {}
      VideoCapture_ (int device)                 : cv::VideoCapture (device) {}

      cv::Ptr<CvCapture> &getCap () { return cap; }
      //cv::Ptr<CvCapture> &getICap () { return icap; }

#ifdef FFMPEG_HACK
      cv::CvCapture_ *getCap_ () { return (cv::CvCapture_*)&*cap; }
      cv::CvCapture_FFMPEG *getFFmpegCap () {
         cv::CvCapture_FFMPEG_proxy *proxy = (cv::CvCapture_FFMPEG_proxy*)&*cap;

          if (!proxy)
            return nullptr;
         else
            return (cv::CvCapture_FFMPEG*)proxy->ffmpegCapture;
      }//end Fct
#endif
   };//end Class


public:// Constructors, Destructors
   VideoReader (void);
   VideoReader (std::string filename);
   VideoReader (int device);

   ~VideoReader (void);

//   std::string class_name (void) const { return "VideoReader"; }


public:// overloaded Octave Functions
   virtual octave_value_list subsref    (const std::string &type, const std::list< octave_value_list > &idx, int nargout);
   virtual void              print      (std::ostream &os, bool pr_as_read_syntax=false); //const;
   virtual void              print_info (std::ostream &os, const std::string &prefix); //const;
   virtual bool              print_name_tag (std::ostream &os, const std::string &name); //const;
   virtual void              print_raw  (std::ostream &os, bool pr_as_read_syntax=false); //const;
   virtual void              print_with_name (std::ostream &os, const std::string &name, bool print_padding = true); //const;
   virtual size_t            byte_size  (void) const { return sizeof (VideoReader); }
   virtual Matrix            size       (void)       { return Matrix (1, 2, 1.0); }   // size = 1x1

   //virtual bool in_class_method (void) const { printf ("point5");return Matrix (1, 2, 1.0); }

   //virtual Array< std::string >  cellstr_value (void) const { printf ("point2"); }
   //virtual Cell   cell_value (void) const { printf ("point3"); }
   //virtual octave_function *  function_value (bool silent=false) { printf ("point4"); }


public:// Functions
   template<class T>
   bool setConfig (const ConfigType type, T value);
   bool set       (std::string type, const octave_value &value);
   static inline std::string fourccToString (int fourcc);


protected:// Functions
   virtual octave_value read      (int from = 0, int to = 0, bool native = false);
   //virtual octave_value write     (int from = 0, int to = 0, bool native = false);

   virtual octave_value fullCheck ();

   bool initPreLoad (bool value);
   void preload ();


protected:// Static Functions
   static void initPreLoad (void *args) {}


protected:// Variables
   utils::Logging   m_oConsole;
   bool             m_bIsValid = false;
   std::string      m_sFilename;
   bool             m_bIsCamera = false;
   VideoCapture_    m_oVC = VideoCapture_ ();

   octave_value     m_oUserData;
   std::string      m_sTag;
   uint8NDArray     m_oPreloadImage;

   size_t           m_iFrameNum = 1;
   size_t           m_iGrabbedFrameNum = 0xefffffff;
   size_t           m_iPreloadedFrameNum = 0xefffffff;

   //std::semaphore   m_oSemaphoreInitPL;
   PreLoadState     m_PreLoadState = PL_Stopped;

   // Configuration
   bool m_bZeroImage = true;

};//end Class



//---------------------------Start setConfig---------------------------------------//
template<class T>
bool VideoReader::setConfig (const ConfigType type, T value) {
   bool res = false;

   switch (type) {
   case VR_None: break;
   case VR_Verbose:   m_oConsole.s_iVerbose = (int32_t)value; res = true; break;
   case VR_ZeroImage: m_bZeroImage = (bool)value;   res = true; break;
   //case VR_SilentRead: m_oVC.set_silentRead ( (bool)value); res = true; break;
   case VR_PreLoad:   res = initPreLoad ( (bool)value); break;
   };//end switch

   return res;
}//end Fct



//---------------------------Start fourccToString----------------------------------//
std::string VideoReader::fourccToString (int fourcc) {
   return cv::format ("%c%c%c%c", fourcc & 255, (fourcc >> 8) & 255, (fourcc >> 16) & 255, (fourcc >> 24) & 255);
}//end Fct

