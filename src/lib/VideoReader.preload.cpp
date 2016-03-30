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



//---------------------------Start preload-----------------------------------------//
/**
 *  Decode the next frame
 */
void VideoReader::preload () {
   LOGGING ("VideoReader::preload()\n");
/*

   if (!m_bIsValid)
      return;

   int32_t from = m_iFrameNum;
   int32_t to   = m_iFrameNum;

   //if (native)
   //   warning ("VideoReader.read: 'native' not implemented yet");

//printf ("time %f\n", m_oVC.get (CV_CAP_PROP_POS_MSEC) );

   if ( (from - 1) != (int)m_oVC.get (CV_CAP_PROP_POS_FRAMES) && from != m_iGrabbedFrameNum) {
      m_oVC.set (CV_CAP_PROP_POS_FRAMES, (double)(from - 1) );
   }

//printf ("pos %f\n", m_oVC.get (CV_CAP_PROP_POS_FRAMES) );
//printf ("time %f\n", m_oVC.get (CV_CAP_PROP_POS_MSEC) );

   dim_vector    d;
   cv::Mat       frame;
   unsigned int  height = (int)m_oVC.get (CV_CAP_PROP_FRAME_HEIGHT);
   unsigned int  width  = (int)m_oVC.get (CV_CAP_PROP_FRAME_WIDTH);
   unsigned int  depth  = 3;


   //TODO check if needed
   if () {
      dim_vector d = dim_vector (height, width, depth);
      m_oPreloadImage = uint8NDArray (d);
   }

   if (m_bZeroImage)
      m_oPreloadImage.fill (0);

   octave_idx_type  posR  = 0;
   octave_idx_type  posG  = 0;
   octave_idx_type  posB  = 0;

   if (from == m_iGrabbedFrameNum) {
      if (!m_oVC.retrieve (frame) ) {
         error ("VideoReader: cannot read frame %d", from);
         break;
      }
   } else {
      if (!m_oVC.read (frame) ) {
         error ("VideoReader: cannot read frame %d", from);
         break;
      }
      m_iGrabbedFrameNum = from;
   }

   //pos   = ( ( ( (0) * (to-from) + i) * depth + <0|1|2>) * width + x) * height + y;
   posR  = 0 * (height * width * depth);
   posG  = posR + (height * width);
   posB  = posG + (height * width);

   for (unsigned int x = 0; x < width; ++x) {
      //int row   = 3 * x;
      //int pixel = row;

      for (unsigned int y = 0; y < height; ++y) {
         //pixel = y * frame->linesize[0] + 3 * x;

         m_oPreloadImage(posR++) = (unsigned int)frame.at<cv::Vec3b> (y, x)[2];
         m_oPreloadImage(posG++) = (unsigned int)frame.at<cv::Vec3b> (y, x)[1];
         m_oPreloadImage(posB++) = (unsigned int)frame.at<cv::Vec3b> (y, x)[0];

         //pixel += frame->linesize[0];
      }//end for 3
   }//end for 2

   m_iFrameNum = from;
*/
}//end Fct


//---------------------------Start initPreLoad-----------------------------------------//
/**
 *  Decode the next frame
 */
bool VideoReader::initPreLoad (bool value) {
   LOGGING ("VideoReader::initPreLoad(%s)\n", BOOL_TO_STR(value) );

   //getsemaphe

   //if (m_oSemaphoreInitPL)
}//end Fct

