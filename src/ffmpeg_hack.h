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
#include <opencv2/opencv.hpp>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>


namespace cv {

//---------------------------Struct CvCapture_-------------------------------------//
struct CvCapture_ {
   virtual ~CvCapture_() {}
   virtual double getProperty(int) const { return 0; }
   virtual bool setProperty(int, double) { return 0; }
   virtual bool grabFrame() { return true; }
   virtual IplImage* retrieveFrame(int) { return 0; }
   virtual int getCaptureDomain() { return -7; } // Return the type of the capture object: CV_CAP_VFW, etc...
};//end Struct



//---------------------------Struct Image_FFMPEG-----------------------------------//
struct Image_FFMPEG {
   unsigned char* data;
   int            step;
   int            width;
   int            height;
   int            cn;
};//end Struct



//---------------------------Struct CvCapture_FFMPEG-------------------------------//
struct CvCapture_FFMPEG {
   bool open( const char* filename );
   void close();
   double getProperty(int) const;
   bool setProperty(int, double);
   bool grabFrame();
   bool retrieveFrame(int, unsigned char** data, int* step, int* width, int* height, int* cn);
   void init();
   void seek(int64_t frame_number);
   void seek(double sec);
   bool slowSeek( int framenumber );
   int64_t get_total_frames() const;
   double get_duration_sec() const;
   double get_fps() const;
   int get_bitrate() const;
   double r2d(AVRational r) const;
   int64_t dts_to_frame_number(int64_t dts);
   double dts_to_sec(int64_t dts);

   AVFormatContext   *ic;
   AVCodec           *avcodec;
   int                video_stream;
   AVStream          *video_st;
   AVFrame           *picture;
   AVFrame            rgb_picture;
   int64_t            picture_pts;
   AVPacket           packet;
   Image_FFMPEG       frame;
   struct SwsContext *img_convert_ctx;
   int64_t            frame_number, first_frame_number;
   double             eps_zero;
   char              *filename;
};//end Struct



//---------------------------Struct CvCapture_FFMPEG_proxy-------------------------//
class CvCapture_FFMPEG_proxy : public CvCapture_ {
public:
   CvCapture_FFMPEG_proxy ();
   virtual ~CvCapture_FFMPEG_proxy();

   virtual double    getProperty(int propId) const;
   virtual bool      setProperty(int propId, double value);
   virtual bool      grabFrame();
   virtual IplImage* retrieveFrame(int);
   virtual bool      open( const char* filename );
   virtual void      close();

   void* ffmpegCapture;
   IplImage frame;
};//end Class

}//end Namespace

