ffmpeg-playground
=================

My experiments with ffmpeg.

Что такое ffmpeg?
-----------------

Как пишут на [официальном сайте](http://ffmpeg.org):

> FFmpeg is the leading multimedia framework, able to decode, encode,
> transcode, mux, demux, stream, filter and play pretty much anything
> that humans and machines have created.

И это утверждение, похоже, соответствует действительности. FFmpeg
используют такие проекты, как MPlayer, VLC, YouTube, Chrome и др.

Несколько лет назад (в 2011 году) внутри проекта случились какие-то
разногласия между разработчиками, появился форк libav, однако на деле
ffmpeg "пилится" активнее (подробности
[тут](http://blog.pkh.me/p/13-the-ffmpeg-libav-situation.html) и
[тут](https://github.com/mpv-player/mpv/wiki/FFmpeg-versus-Libav)).

Составные части
---------------

### Утилиты

-   ffmpeg -- конвертирует видео из одного формата в другой
-   ffplay -- простой плеер на SDL и библиотеках ffmpeg
-   ffserver -- потоковый сервер для видео- или радиовещания
-   ffprobe -- простой анализатор мультимедиа

### Библиотеки

-   libavutil -- вспомогательная библиотека общих для ffmpeg функций
-   libavcodec -- библиотека со всеми аудио/видеокодеками
-   libavformat -- библиотка ввода/вывода и мультиплексирования/демультиплексирования
-   libavdevice -- библиотека устройств ввода/вывода для захвата и рендеринга мультимедиа
-   libavfilter -- позволяет изменять видеопоток между декодером и кодером "на лету"
-   libswscale -- библиотека для масштабирования видео
-   libswresample -- библиотека передискретизации аудио (ресамплинга)

Как это все работает
--------------------

[Оригинальный туториал](http://dranger.com/ffmpeg/tutorial01.html)

[ffmpeg-tutorial](https://github.com/chelyaev/ffmpeg-tutorial)

### Tutorial 01: Первый кадр

Видеофайл штука довольно сложная. Во-первых, файл сам по себе является
*контейнером* (container), и тип контейнера определяет, где находится
информация внутри файла. В качестве примера контейнеров можно привести форматы
AVI и QuickTime. Внутри файла может быть несколько *потоков* (streams).
Например, обычно в фильме есть аудиопоток и видеопоток. "Поток" означает
непрерывное поступление данных, доступных по мере течения времени. Процесс
"разделения файла" на потоки называется демультиплексированием (demuxing).
Элементы данных в потоке называют *кадрами* (frames). Каждый поток закодирован
(сжат) каким-либо *кодеком* (codec). В качестве примера кодека можно привести
DivX и MP3. *Пакет* (packet) содержит одни или несколько раскодированных
кадров.

Очень грубо обработку видео и аудиопотоков можно описать так:

    10 OPEN video_stream FROM video.avi
    20 READ packet FROM video_stream INTO frame
    30 IF frame NOT COMPLETE GOTO 20
    40 DO SOMETHING WITH frame
    50 GOTO 20

В этом туториале мы собираемся открыть файл, прочитать видеопоток внутри него и
сохранить первый кадр в PPM файл.

#### Зависимости

Для начала установим необходимые зависимости
    $ sudo apt-get install libavutil-dev libavcodec-deb libavformat-dev

А для экспериментов скачаем короткиий анимационный фильм [Big Buck
Bunny](http://download.blender.org/peach/bigbuckbunny_movies/big_buck_bunny_480p_surround-fix.avi)
(210M).


#### Открытие файла

Во-первых, инициализируем библиотеку.

```cpp
#include <libavformat/avformat.h>

int main(int argc, char *argv[]) {
    // Register all file formats and codecs
    av_register_all();

    return 0;
}
```

av_register_all() регистрирует все доступные форматы файлов и кодеков для
автоматического использования в случае открытия соответствующего файла. Функцию
нужно вызвать только раз, поэтому мы делаем это в main(). Возможно
зарегистрировать форматы файлов и кодеков индивидуально, но в большинстве
случаев в этом нет необходимости.

Теперь мы можем открыть файл.
```cpp
// Open video file
AVFormatContext *pFormatCtx = NULL;
if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0) {
    return -1; // Couldn't open file
}
```

Функция читает заголовок файла и сохраняет информацию в структуре AVFileContext.

#### Информация о потоках

Читаем информацию о потоках
```cpp
// Retrieve stream information
if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
    return -1; // Couldn't find stream information
}
```

Функция заполняет FormatCtx->streams необходимой информацией.
Выведем ее с помощью удобной функции
```cpp
// Dump information about file onto standard error
av_dump_format(pFormatCtx, 0, argv[1], 0);
```

#### Компиляция
    gcc -o tutorial01 tutorial01.c -lavformat

#### Запуск
    $ ./tutorial01 big_buck_bunny_480p_surround-fix.avi
    Input #0, avi, from 'big_buck_bunny_480p_surround-fix.avi':
      Duration: 00:09:56.45, start: 0.000000, bitrate: 2957 kb/s
        Stream #0.0: Video: mpeg4 (Simple Profile), yuv420p, 854x480 [PAR 1:1 DAR 427:240], 24 tbn, 24 tbc
        Stream #0.1: Audio: ac3, 48000 Hz, 5.1, fltp, 448 kb/s


#### Находим первый видеопоток

Пройдемся по массиву указателей `pFormatCtx->streams` (размер массива
`pFormatCtx->nb_streams`), пока не найдем видеопоток.
```cpp
AVCodecContext *pCodecCtx = NULL;
int i, videoStream;
// Find the first video stream
videoStream=-1;
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

```

#### Откроем необходимый кодек

Обратите внимание, что при сборке необходимо слинковаться с libavcodec

```cpp
AVCodec *pCodec = NULL;
AVDictionary *optionsDict = NULL;
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
```

#### Сохранение данных

Теперь нам необходимо место для хранения кадра
```cpp
AVFrame *pFrame = NULL;
// Allocate video frame
pFrame = avcodec_alloc_frame();
```

Поскольку мы планируем сохранить кадр в PPM файл, который хранит 25-битный RGB,
нам нужно сконвертировать кадр из его родного формата в RGB. ffmpeg сделает это
преобразование за нас. Выделим место под преобразованный кадр.

```cpp
// Allocate an AVFrame structure
AVFrame *pFrameRGB = NULL;
pFrameRGB = avcodec_alloc_frame();
if (pFrameRGB == NULL) {
    return -1;
}
```

Несмотря на то, что мы выделили место для фрейма, нам все еще необходимо место
для хранения данных при конвертации. Мы используем `avpicture_get_size()` для
выяснения необходимого размера, и выделим место вручную.

```cpp
uint8_t *buffer;
int numBytes;
// Determine required buffer size and allocate buffer
numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
                              pCodecCtx->height);
buffer = (uint8_t *) av_malloc(numBytes*sizeof(uint8_t));
```

`av_malloc()` -- это обёртка ffmpeg вокруг malloc, которая проверяет
выравнивание и всё такое. Она *не* защищает от утечек, двойного освобождения и
других проблем.

Для использования `av_malloc` приложение необходимо слинковать с `libavutil`

Теперь используем `avpicture_fill()` для связывания кадра с нашим буфером.
```cpp
// Assign appropriate parts of buffer to image planes in pFrameRGB
// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
// of AVPicture
avpicture_fill((AVPicture *)pFrameRGB. buffer, PIX_FMT_RGB24,
               pCodecCtx->width, pCodecCtx->height);
```

Теперь мы готовы прочитать поток.

#### Чтение данных

Теперь мы собираемся прочитать пакет из потока, декодировать его в наш кадр,
после чего сконвертировать и сохранить этот кадр.

Структура SwsContext и функции sws_scale и sws_getContext определены в
libswscale. Не забудьте подключить соответствующий заголовочный файл `#include
<libswscale/swscale.h>` и слинковать с `libswscale`.

```cpp
int frameFinished;
AVPacket packet;
struct SwsContext *sws_ctx = NULL;

sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
            pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
            PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

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
```

Теперь реализуем функцию `SaveFrame` (про ppm формат можно почитать
[здесь](https://ru.wikipedia.org/wiki/Portable_anymap)):

```cpp
void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
    FILE *pFile;
    char szFilename[32];
    int y;

    // Open file
    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile = fopen(szFilename, "wb");
    if (pFile == NULL) {
        return;
    }

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for (y = 0; y < height; y++) {
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
    }

    // Close file
    fclose(pFile);
}
```

Осталось только все почистить:
```cpp
// Free the RGB image
av_free(buffer);
av_free(pFrameRGB);

// Free the YUV frame
av_free(pFrame);

// Close the codec
avcodec_close(pCodecCtx);

// Close the video file
avformat_close_input(&pFormatCtx);
```

### Tutorial 02: Вывод на экран

#### SDL и видео

Для отрисовки на экране мы используем [SDL](http://www.libsdl.org) (Simple
DirectMedia Layer). В данном туториале используем библиотеку из репозитория. `$
sudo apt-get install libsdl1.2-dev`

SDL в целом имеет много методов для отрисовки изображений на экран, и в
частности, один, вполне подходящий для показа видео -- YUV overlay.
[YUV](http://en.wikipedia.org/wiki/YCbCr) -- это цветовая модель, в которой
цвет представляется как 3 компоненты (яркость Y и две цветоразностных U и V).

Наш план состоит в том, чтобы заменить функцию SaveFrame() из предыдущего
туториала, а вместо этого показывать кадры на экране.

Но сперва посмотрим, как использовать SDL.

```cpp
#include <SDL.h>

int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }
    return 0;
}
```

Для компиляции используем утилиту `sdl-config`, которая просто возвращает
правильные флаги:

    $ sdl-config --cflags
    -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
    $ sdl-config --libs
    -L/usr/lib/x86_64-linux-gnu -lSDL

    $ gcc -o tutorial02 tutorial02.c `sdl-config --cflags --libs`

Теперь нам необходимо место на экране для показа изображений (surface).

```cpp
// Make a screen to put our video
SDL_Surface *screen = NULL;
screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
if (!screen) {
    fprintf(stderr, "SDL: could not set video mode - exiting\n");
    exit(1);
}
```

Этот код настраивает экран с переданной шириной и высотой. Следущая опция
устанавливает битовую глубину экрана (0 означает "такая же как у текущего
экрана", не работает для OS X).
