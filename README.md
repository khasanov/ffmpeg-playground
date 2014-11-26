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
-   libavformat -- библиотка с мультиплексорами и демультиплексорами
-   libavdevice -- библиотека устройств ввода/вывода для захвата и рендеринга мультимедиа
-   libavfilter -- позволяет изменять видеопоток между декодером и кодером "на лету"
-   libswscale -- библиотека для масштабирования видео
-   libswresample -- библиотека передискретизации аудио (ресамплинга)

Как это все работает
--------------------

[Оригинальный туториал](http://dranger.com/ffmpeg/tutorial01.html)

[ffmpeg-tutorial](https://github.com/chelyaev/ffmpeg-tutorial)
