clib
====

C libraries that are used for my projects.

## HTTP Live Streaming (httplivestreaming.{c,h})

Creates HTTP Live Streaming MPEG-TS segments and m3u8 file from MPEG-TS stream.

### Functions

    HTTPLiveStreaming *hls_create();

Starts new HTTP Live Streaming and returns a HTTPLiveStreaming struct.

    int hls_write_packet(HTTPLiveStreaming *hls, AVPacket *pkt, int split);

Write `pkt` to HTTP Live Streaming bundled with `hls`. If split is 1, current semgment will be splitted right before this packet.

    void hls_destroy(HTTPLiveStreaming *hls);

Destroy HTTPLivestreaming struct.

## mpegts

Writes MPEG-TS with libavformat.

## hooks

Creates hook mechanism using inotify.

## state

Outputs application state as files.

## config.h

To be included from libraries.
