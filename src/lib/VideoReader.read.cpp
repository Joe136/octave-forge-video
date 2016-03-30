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



//---------------------------Start read--------------------------------------------//
/**
 *    from = 1-based index of the starting frame
 */
octave_value VideoReader::read (int from, int to, bool native) {
   LOGGING ("VideoReader::read(%i, %i, %s)\n", from, to, BOOL_TO_STR(native) );
   m_oConsole[40] << "VideoReader::read(" << from << ", " << to << ", " << native << ")" << utils::Logging::endl;

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

   if ( (to - 1) > m_oVC.get (CV_CAP_PROP_FRAME_COUNT) )
      to = m_oVC.get (CV_CAP_PROP_FRAME_COUNT) + 1;

   if (to < from)
      from = to;

//printf ("time %f\n", m_oVC.get (CV_CAP_PROP_POS_MSEC) );

   if ( (from - 1) != (int)m_oVC.get (CV_CAP_PROP_POS_FRAMES) && from != m_iGrabbedFrameNum) {
/*      // There is no reason to correct the position to keyframe, opencv does it too
#ifdef FFMPEG_HACK
      if (from <= 1) {
         m_oVC.set (CV_CAP_PROP_POS_FRAMES, (double)(from - 1) );
      } else {
         m_oVC.set (CV_CAP_PROP_POS_FRAMES, (double)(from - 2) );
         m_oVC.grab ();
//printf ("prestep %f %f %i %li\n", m_oVC.get (CV_CAP_PROP_POS_FRAMES), m_oVC.get (CV_CAP_PROP_POS_MSEC), m_oVC.getFFmpegCap ()->picture->key_frame, m_oVC.getFFmpegCap ()->picture->pkt_pts);

         // Jump back to last keyframe
         cv::CvCapture_FFMPEG *cap = m_oVC.getFFmpegCap ();
         if (cap && cap->picture) {
            // If it's not a keyframe
            if (!cap->picture->key_frame) {
               int stepback = 2;

               // Step back to keyframe
               while (!cap->picture->key_frame && (from > stepback) ) {
                  ++stepback;
//printf ("stepback %i %i %f %i\n", from, stepback, (double)(from - stepback), cap->picture->pict_type);
                  m_oVC.set (CV_CAP_PROP_POS_FRAMES, (double)(from - stepback) );
                  m_oVC.grab ();
//printf ("stepback %f %f %i %li\n", m_oVC.get (CV_CAP_PROP_POS_FRAMES), m_oVC.get (CV_CAP_PROP_POS_MSEC), cap->picture->key_frame, cap->picture->pkt_pts);
               }
//printf ("pos %f\n", m_oVC.get (CV_CAP_PROP_POS_FRAMES) );
               // Step forward to target frame
               while (m_oVC.get (CV_CAP_PROP_POS_FRAMES) < (double)(from - 1) ) {
                  m_oVC.grab ();
//printf ("stepforward %f %f %i %li\n", m_oVC.get (CV_CAP_PROP_POS_FRAMES), m_oVC.get (CV_CAP_PROP_POS_MSEC), cap->picture->key_frame, cap->picture->pkt_pts);
               }//end while
            }
         }
      }
#else
*/
      m_oVC.set (CV_CAP_PROP_POS_FRAMES, (double)(from - 1) );
//#endif
   }

//printf ("pos %f\n", m_oVC.get (CV_CAP_PROP_POS_FRAMES) );
//printf ("time %f\n", m_oVC.get (CV_CAP_PROP_POS_MSEC) );

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

      //---------------------Read frame--------------------------------------------//
      if (from == m_iGrabbedFrameNum) {
         m_oConsole[45] << "VideoReader::OpenCV::retrieve()" << utils::Logging::endl;
         if (!m_oVC.retrieve (frame) ) {
            error ("VideoReader: cannot read frame %d", from);
            break;
         }
      } else {
         m_oConsole[45] << "VideoReader::OpenCV::read()" << utils::Logging::endl;
         m_iGrabbedFrameNum = from;
         if (!m_oVC.read (frame) ) {
            error ("VideoReader: cannot read frame %d", from);
            break;
         }
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

