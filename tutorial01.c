// tutorial01.c

#include <libavformat/avformat.h>

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

    if (argc < 2) {
        printf("Please provide a movie file\n");
        return -1;
    }

    // Register all file formats and codecs
    av_register_all();

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

    // Allocate an AVFrame structure
    pFrameRGB = avcodec_alloc_frame();
    if (pFrameRGB == NULL) {
        return -1;
    }

    // Determine required buffer size and allocate buffer
    numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
                                  pCodecCtx->height);
    buffer = (uint8_t *) av_malloc(numBytes*sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
                   pCodecCtx->width, pCodecCtx->height);

    return 0;
}
