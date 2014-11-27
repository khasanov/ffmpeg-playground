// tutorial01.c

#include <libavformat/avformat.h>

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx = NULL;

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

    return 0;
}
