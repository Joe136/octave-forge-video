/* Copyright (c) 2005 Stefan van der Walt <stefan@sun.ac.za>
 *
 * Based in part on the libavformat example, which is
 * Copyright (c) 2003 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "AVHandler.h"

#include <string>
#include <sstream>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <ext/stdio_filebuf.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

extern "C" {
#if defined (HAVE_FFMPEG_AVFORMAT_H)
#include <ffmpeg/swscale.h>
#elif defined(HAVE_LIBAVFORMAT_AVFORMAT_H)
#include <libswscale/swscale.h>
#include <libavutil/dict.h>
#include <libavutil/rational.h>
#else
#error "Missing ffmpeg headers"
#endif
}

#ifdef _MSC_VER
#define snprintf _snprintf
#endif


AVHandler::~AVHandler(void) {
    if (frame)    av_frame_free (&frame);
    if (rgbframe) av_frame_free (&rgbframe);

    // we can use FFMpeg's `av_close_input_file` for input files, but
    // output files must be closed manually

    // flush buffers, write headers and close output file
    if (av_output) {
        if (av_output->pb->buf_ptr) {
            while (write_frame() > 0) {}
            av_write_trailer(av_output);
            if (avio_close( av_output->pb ) < 0)
                (*out) << "AVHandler: cannot close output file" << std::endl;
        }
        av_free(av_output);
    }

    // close codec
    if (vstream) {
        if (vstream->codec->codec) {
            avcodec_close( vstream->codec );
        }
    }

    if (av_input) {
        avformat_close_input(&av_input);
    } else {
        // close output stream
        if (vstream) av_freep(&vstream);
    }

    if (video_outbuf) {
        av_free(video_outbuf);
    }

}

int
AVHandler::setup_write() {
    av_register_all();

    AVOutputFormat *avifmt = NULL;
    while (NULL != (avifmt = av_oformat_next(avifmt))) {
        if (std::string(avifmt->name) == "avi") {
            break;
        }
    }

    if (!avifmt) {
        (*out) << "AVHandler: Error finding AVI output format" << std::endl;
        return -1;
    }

    av_output = avformat_alloc_context();
    if (!av_output) {
        (*out) << "AVHandler: Memory error allocating format context" << std::endl;
        return -1;
    }

    // use AVI encoding
    av_output->oformat = avifmt;
    av_output->oformat->audio_codec = CODEC_ID_NONE;

    if (avifmt->video_codec != CODEC_ID_NONE) {
        if (add_video_stream() != 0) return -1;
    }

    snprintf(av_output->filename, sizeof(av_output->filename), "%s", filename.c_str());

    assert(vstream == av_output->streams[vstream->index]);

    av_dict_set(&vstream->metadata, "title", title.c_str(), 0);
    av_dict_set(&vstream->metadata, "author", author.c_str(), 0);
    av_dict_set(&vstream->metadata, "comment", comment.c_str(), 0);

    if (avio_open(&av_output->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0) {
	(*out) << "AVHandler: Could not open \"" << filename << "\" for output" << std::endl;
	return -1;
    }

    if (init_video_codecs() != 0) return -1;

    if (frame)    av_frame_free (&frame);
    if (rgbframe) av_frame_free (&rgbframe);

    frame    = create_frame(vstream->codec->pix_fmt);
    rgbframe = create_frame(PIX_FMT_RGB24);

    if (!frame || !rgbframe) return -1;

    if (avformat_write_header(av_output, NULL) < 0) {
	(*out) << "AVHandler: Error writing headers" << std::endl;
	return -1;
    }

    av_dict_free(&vstream->metadata);

    return 0;
}

int
AVHandler::setup_read() {
    av_register_all();

    if (avformat_open_input(&av_input, filename.c_str(), NULL, NULL) != 0) {
        (*out) << "AVHandler: Could not open \"" << filename << "\" for reading" << std::endl;
        return -1;
    }

    if (avformat_find_stream_info(av_input, NULL) < 0) {
        (*out) << "AVHandler: No stream information available" << std::endl;
        return -1;
    }

    for (unsigned int i=0; i < av_input->nb_streams; i++) {
        if (av_input->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            vstream = av_input->streams[i];
            break;
        }
    }
    if (!vstream) {
        (*out) << "AVHandler: No video streams found" << std::endl;
        return -1;
    }

    for (unsigned int i=0; i < av_input->nb_streams; i++) {
        if (av_input->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            astream = av_input->streams[i];
            break;
        }
    }

    AVCodec *codec;
    codec = avcodec_find_decoder(vstream->codec->codec_id);

    if (!codec) {
        (*out) << "AVHandler: Cannot find codec used in stream" << std::endl;
        return -1;
    }
    codec_name = codec->name;

    // We can handle truncated bitstreams
    if (codec->capabilities & CODEC_CAP_TRUNCATED)
        vstream->codec->flags |= CODEC_FLAG_TRUNCATED;

    if (avcodec_open2(vstream->codec, codec, NULL) < 0) {
        (*out) << "AVHandler: Cannot open codec " << codec_name << std::endl;
        vstream->codec->codec = NULL;
        return -1;
    }

    /// XXX TODO XXX Verify that this calculation is correct
    AVRational av_fr = vstream->r_frame_rate;
    framerate = (double)av_fr.num / (double)av_fr.den;
    width = vstream->codec->width;
    height = vstream->codec->height;

    if (vstream->metadata) {
    AVDictionaryEntry *entry = NULL;

    entry = av_dict_get(vstream->metadata, "title", NULL, AV_DICT_IGNORE_SUFFIX);
    if (entry) title = entry->value;
    entry = av_dict_get(vstream->metadata, "author", NULL, AV_DICT_IGNORE_SUFFIX);
    if (entry) author = entry->value;
    entry = av_dict_get(vstream->metadata, "comment", NULL, AV_DICT_IGNORE_SUFFIX);
    if (entry) comment = entry->value;
    }

    if (rgbframe) av_frame_free (&rgbframe);
    rgbframe = create_frame(PIX_FMT_RGB24);
    if (!rgbframe) return -1;

    return 0;
}

void
AVHandler::draw_background(unsigned char r, unsigned char g, unsigned char b) {
    int x, y;
    for (y = 0; y < vstream->codec->height; y++) {
        for (x = 0; x < vstream->codec->width; x++) {
            rgbframe->data[0][y * rgbframe->linesize[0] + 3*x] = r;
            rgbframe->data[0][y * rgbframe->linesize[0] + 3*x+1] = g;
            rgbframe->data[0][y * rgbframe->linesize[0] + 3*x+2] = b;
        }
    }
}

int
AVHandler::write_frame() {
    if (video_outbuf == NULL) {
        return 0;
    }

    lock_parameters = true;

    AVCodecContext *c = vstream->codec;

    if (frame && rgbframe) {
      SwsContext *sc = sws_getContext(c->width, c->height, PIX_FMT_RGB24,
                                      c->width, c->height, c->pix_fmt,
                                      SWS_BICUBIC, 0, 0, 0);
      sws_scale(sc, rgbframe->data, rgbframe->linesize, 0,
                c->height, frame->data, frame->linesize);
      //sws_freeContext (sc);
    }

    if (NULL == frame && !(c->codec->capabilities & CODEC_CAP_DELAY)) {
    // this is a workaround to avoid segmentation fault of calling
    // avcodec_encode_video2() below
    return 0;
    }

    int output_ready;
    AVPacket pkt;

    pkt.data = video_outbuf;
    pkt.size = VIDEO_OUTBUF_SIZE;

	if (avcodec_encode_video2(c, &pkt, frame, &output_ready) < 0) {
	    (*out) << "AVHandler: error encoding video frame" << std::endl;
	    return -1;
	}

    if (output_ready) {
	if (c->coded_frame)
	    pkt.pts = c->coded_frame->pts;
	if (c->coded_frame && c->coded_frame->key_frame)
	    pkt.flags |= AV_PKT_FLAG_KEY;
	/// XXX FIXME XXX does this ensure that the first frame is always a key frame?
	
	if (av_write_frame(av_output, &pkt) != 0) {
	    (*out) << "AVHandler: error writing video frame" << std::endl;
	    return -1;
	}

    }

    frame_nr++;
    return output_ready ? pkt.size : 0;
}

int
AVHandler::read_frame(unsigned int nr, bool force) {
    nr--; // zero-based calculations

    if (!force && frame_nr && nr == frame_nr + 1) {
        int ret = next_frame ();
        if (ret == 0)
            return ret;
    }

    AVCodecContext *cc = vstream->codec;

    // Calculate timestamp of target frame
    uint64_t start_time = 0;
    if ((uint64_t)vstream->start_time != (uint64_t)AV_NOPTS_VALUE) {
        start_time = vstream->start_time;
    }
    uint64_t target_timestamp = start_time + nr * (uint64_t)(AV_TIME_BASE / framerate);

    if (((target_timestamp - start_time) < 0) ||
        ((target_timestamp - start_time) >= (uint64_t)av_input->duration)) {
        (*err) << "AVHandler: Invalid frame requested" << std::endl;
        return -1;
    }

    // Seek to closest keyframe
    if (av_seek_frame(av_input, -1, target_timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
        (*err) << "AVHandler: Error seeking to " << target_timestamp << std::endl;
        return -1;
    }
    cc->skip_frame = AVDISCARD_NONKEY;
    frame_nr = nr;

    // Flush stream buffers after seek
    avcodec_flush_buffers(cc);

    // Calculate stream time base
    double stream_time_base = 1;
    if (vstream->time_base.den != 0) {
        stream_time_base = (double)vstream->time_base.num / vstream->time_base.den;
    }

    if (frame) av_frame_free(&frame);
    frame = av_frame_alloc();

    uint64_t current_timestamp = 0;
    AVPacket packet;
    std::shared_ptr<__gnu_cxx::stdio_filebuf<char> > fb_err;
    int      old_err;


    // Redirect stderr to /dev/null
    if (silentRead) {
        fb_err = std::make_shared<__gnu_cxx::stdio_filebuf<char> > (open("/dev/null", O_RDWR), std::ios_base::out);
        old_err = dup (STDERR_FILENO);
        dup2 (fb_err->fd(), STDERR_FILENO);
    }

    while (current_timestamp <= target_timestamp) {

        // Read until we find a packet from the video stream
        packet.stream_index = -1;
        while (packet.stream_index != vstream->index) {
            if (av_read_frame(av_input, &packet)) {
                (*err) << "AVHandler: Error reading packet after timestamp " << current_timestamp << std::endl;
                av_free_packet(&packet);
                av_frame_free(&frame);
                if (silentRead) { dup2(old_err, STDERR_FILENO); close (old_err); fb_err.reset (); }
                return -1;
            }

            if (av_input->pb->eof_reached) {
                (*out) << "AVHandler: EOF reached" << std::endl;
            }
        }//end while 2

        // Decode the packet into a frame
        int frameFinished;

        // HACK for CorePNG to decode as normal PNG by default
        packet.flags = AV_PKT_FLAG_KEY;

        if (avcodec_decode_video2(cc, frame, &frameFinished, &packet) < 0) {
            (*err) << "AVHandler: Error decoding video stream" << std::endl;
            av_free_packet(&packet);
            av_frame_free(&frame);
            if (silentRead) { dup2(old_err, STDERR_FILENO); close (old_err); fb_err.reset (); }
            return -1;
        }

        if (frameFinished) {
            current_timestamp = (uint64_t)(vstream->cur_dts * AV_TIME_BASE * (long double)stream_time_base);
        }

        if (current_timestamp <= target_timestamp)
            av_free_packet(&packet);
    }//end while 1

    // Restore redirection of stderr
    if (silentRead) {
        dup2(old_err, STDERR_FILENO);
        close (old_err);
        fb_err.reset ();
    }

    cc->skip_frame = AVDISCARD_NONE;

    SwsContext *sc = sws_getContext(cc->width, cc->height, cc->pix_fmt,
                                    cc->width, cc->height, PIX_FMT_RGB24,
                                    SWS_BICUBIC, 0, 0, 0);
    sws_scale(sc, frame->data, frame->linesize, 0,
              cc->height, rgbframe->data, rgbframe->linesize);

    sws_freeContext (sc);
    av_free_packet(&packet);
    av_frame_free(&frame);

    return 0;
}

int
AVHandler::next_frame() {
    AVCodecContext *cc = vstream->codec;

    cc->skip_frame = AVDISCARD_NONKEY;
//    cc->skip_frame = AVDISCARD_NONE;

    if (frame) av_frame_free(&frame);
    frame = av_frame_alloc();

    AVPacket packet;
    std::shared_ptr<__gnu_cxx::stdio_filebuf<char> > fb_err;
    int      old_err;
    ++frame_nr;


    // Redirect stderr to /dev/null
    if (silentRead) {
        fb_err = std::make_shared<__gnu_cxx::stdio_filebuf<char> > (open("/dev/null", O_RDWR), std::ios_base::out);
        old_err = dup (STDERR_FILENO);
        dup2 (fb_err->fd(), STDERR_FILENO);
    }


    // Decode the packet into a frame
    int frameFinished = 0;

    while (!frameFinished) {
        // Read until we find a packet from the video stream
        packet.stream_index = -1;
        while (packet.stream_index != vstream->index) {
            if (av_read_frame(av_input, &packet)) {
                if (!silentRead) (*err) << "AVHandler: Error reading packet for target frame " << frame_nr << std::endl;
                av_free_packet(&packet);
                av_frame_free(&frame);
                if (silentRead) { dup2(old_err, STDERR_FILENO); close (old_err); fb_err.reset (); }
                return -1;
            }

            if (av_input->pb->eof_reached) {
                (*out) << "AVHandler: EOF reached" << std::endl;
            }
        }//end while 2

        // HACK for CorePNG to decode as normal PNG by default
        packet.flags = AV_PKT_FLAG_KEY;

        if (avcodec_decode_video2(cc, frame, &frameFinished, &packet) < 0) {
            if (!silentRead) (*err) << "AVHandler: Error decoding video stream for target frame " << frame_nr << std::endl;
            av_free_packet(&packet);
            av_frame_free(&frame);
            if (silentRead) { dup2(old_err, STDERR_FILENO); close (old_err); fb_err.reset (); }
            return -1;
        }
    }//end while 1


    // Restore redirection of stderr
    if (silentRead) {
        dup2(old_err, STDERR_FILENO);
        close (old_err);
        fb_err.reset ();
    }

    cc->skip_frame = AVDISCARD_NONE;

    SwsContext *sc = sws_getContext(cc->width, cc->height, cc->pix_fmt,
                                    cc->width, cc->height, PIX_FMT_RGB24,
                                    SWS_BICUBIC, 0, 0, 0);
    sws_scale(sc, frame->data, frame->linesize, 0,
              cc->height, rgbframe->data, rgbframe->linesize);

    sws_freeContext (sc);
    av_free_packet(&packet);
    av_frame_free(&frame);

    return 0;
}

std::string
AVHandler::print_file_formats() {
    std::stringstream ss;

    ss << "Supported file formats:" << std::endl;
    av_register_all();

    AVOutputFormat *ofmt = NULL;
    while (NULL != (ofmt = av_oformat_next(ofmt))) {
        ss << ofmt->name << " ";
    }
    ss << std::endl;

    return ss.str();
}

std::string
AVHandler::print_codecs() {
    std::stringstream ss;

    ss << "Supported video codecs:" << std::endl;
    av_register_all();

    AVCodec *codec;
    for (codec = av_codec_next(0); codec != NULL; codec = av_codec_next(codec)) {
        if ((codec->type == AVMEDIA_TYPE_VIDEO) &&
            (codec->encode2)) {
            ss << codec->name << " ";
        }
    }
    ss << std::endl;

    return ss.str();
}

int
AVHandler::add_video_stream() {
    AVCodecContext *cc;

    // FIXME: vstream = avformat_new_stream(av_output, av_find_default_stream_index(av_output));
    vstream = avformat_new_stream(av_output, NULL);
    if (!vstream) {
        (*out) << "AVHandler: error opening video output stream" << std::endl;
        return -1;
    }

    cc = vstream->codec;

    cc->codec_type = AVMEDIA_TYPE_VIDEO;

    cc->bit_rate = bitrate;
    cc->width = width;
    cc->height = height;

    // XXX TODO XXX Make sure this calculation is correct //
#if 0
    cc->time_base.num = 1;
    cc->time_base.den = (int)(framerate);
#else
    cc->time_base = av_inv_q(av_d2q(framerate, INT_MAX));
#endif
    cc->pix_fmt = PIX_FMT_YUV420P;

    cc->gop_size = gop_size;

    return 0;
}

int
AVHandler::init_video_codecs() {
    AVCodec *codec;
    AVCodecContext *cc;

    cc = vstream->codec;

    //codec = avcodec_find_encoder(cc->codec_id);
    codec = avcodec_find_encoder_by_name(codec_name.c_str());

    if (!codec) {
        (*out) << "AVHandler: cannot find codec " << codec_name << std::endl;
        return -1;
    }

    if (avcodec_open2(cc, codec, NULL) < 0) {
        (*out) << "AVHandler: cannot open codec" << std::endl;
        cc->codec = NULL;
        return -1;
    }

    // XXX FIXME XXX What is the best size for video_outbuf?
    video_outbuf = (uint8_t *)malloc(VIDEO_OUTBUF_SIZE);

    return 0;
}

AVFrame*
AVHandler::create_frame(PixelFormat fmt) {
    AVFrame *frame;
    uint8_t *frame_buf;

    frame = av_frame_alloc();
    if (!frame) {
        (*out) << "AVHandler: cannot allocate frame" << std::endl;
        return NULL;
    }

    int size = avpicture_get_size(fmt,
                                  vstream->codec->width,
                                  vstream->codec->height);

    frame_buf = (uint8_t *)malloc(size);
    if (!frame_buf) {
        av_frame_free(&frame);
        (*out) << "AVHandler: error initialising frame" << std::endl;
        return NULL;
    }

    avpicture_fill((AVPicture *)frame, frame_buf, fmt,
                   vstream->codec->width, vstream->codec->height);

    return frame;
}
