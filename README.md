clib
====

My C libraries for Linux.

## HTTP Live Streaming (httplivestreaming.{c,h})

Creates HTTP Live Streaming MPEG-TS segments and m3u8 file from MPEG-TS stream.

### Functions

    HTTPLiveStreaming *hls_create();

Starts new HTTP Live Streaming and returns a HTTPLiveStreaming struct.

    int hls_write_packet(HTTPLiveStreaming *hls, AVPacket *pkt, int split);

Write `pkt` to HTTP Live Streaming bundled with `hls`. If split is 1, current semgment will be splitted right before this packet.

    void hls_destroy(HTTPLiveStreaming *hls);

Destroy HTTPLivestreaming struct.

## MPEG-TS (mpegts.{c,h})

Writes MPEG-TS with libavformat.

### Functions

    AVFormatContext *mpegts_create_context();

Prepares new MPEG-TS stream and returns the pointer to AVFormatContext.

    void mpegts_open_stream(AVFormatContext *format_ctx, char *filename, int dump_format);

Opens new file for writing. If `dump_format` is 1, details of output format is written to the terminal.

    void mpegts_close_stream(AVFormatContext *format_ctx);

Closes the file. If this function completes, the file is ready for play.

    void mpegts_destroy_context(AVFormatContext *format_ctx);

Destroy AVFormatContext that is pointed by `format_ctx`.

## hooks

Creates hook mechanism using inotify.

## state

Outputs application state as files.

## config.h

To be included from libraries.
