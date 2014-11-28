// tutorial02.c
// Based on https://github.com/chelyaev/ffmpeg-tutorial/blob/master/tutorial02.c

#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <SDL.h>

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
}

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx = NULL;
    int i, videoStream;
    AVCodecContext *pCodecCtx = NULL;
    AVCodec *pCodec = NULL;
    AVDictionary *optionsDict = NULL;
    AVFrame *pFrame = NULL;
    AVFrame *pFrameRGB = NULL;
    uint8_t *buffer = NULL;
    int numBytes;
    int frameFinished;
    AVPacket packet;
    struct SwsContext *sws_ctx = NULL;

    SDL_Surface *screen = NULL;

    if (argc < 2) {
        printf("Please provide a movie file\n");
        return -1;
    }

    // Register all file formats and codecs
    av_register_all();

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }

    // Open video file
    if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0) {
        return -1; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        return -1; // Couldn't find stream information
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, argv[1], 0);

    // Find the first video stream
    videoStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1) {
        return -1; // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }

    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, &optionsDict) < 0) {
        return -1; // Could not open codec
    }

    // Allocate video frame
    pFrame = avcodec_alloc_frame();

    // Make a screen to put our video
#ifndef __DARWIN__
    screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
#else
    screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 24, 0);
#endif
    if (!screen) {
        fprintf(stderr, "SDL: could not set video mode - exiting\n");
        exit(1);
    }

    // Allocate an AVFrame structure
    pFrameRGB = avcodec_alloc_frame();
    if (pFrameRGB == NULL) {
        return -1;
    }

    // Determine required buffer size and allocate buffer
    numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
                                  pCodecCtx->height);
    buffer = (uint8_t *) av_malloc(numBytes*sizeof(uint8_t));

    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
                   pCodecCtx->width, pCodecCtx->height);

    // Read frames and save first five frames to disk
    i = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {
            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
                                 &packet);

            // Did we get a video frame?
            if (frameFinished) {
                // Convert the image from its native format to RGB
                sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height,
                          pFrameRGB->data, pFrameRGB->linesize);

                // Save the frame to disk
                if (++i <= 5) {
                    SaveFrame(pFrameRGB, pCodecCtx->width,
                              pCodecCtx->height, i);
                }
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }

    // Free the RGB image
    av_free(buffer);
    av_free(pFrameRGB);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return 0;
}
