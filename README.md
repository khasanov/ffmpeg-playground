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


