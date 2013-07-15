clib
====

My C libraries for Linux.

## HTTP Live Streaming (httplivestreaming.{c,h})

Create HTTP Live Streaming MPEG-TS segments and m3u8 file from MPEG-TS stream.

### Functions

    HTTPLiveStreaming *hls_create();

Start new HTTP Live Streaming and returns a HTTPLiveStreaming struct.

    int hls_write_packet(HTTPLiveStreaming *hls, AVPacket *pkt, int split);

Write `pkt` to HTTP Live Streaming bundled with `hls`. If split is 1, current semgment will be splitted right before this packet.

    void hls_destroy(HTTPLiveStreaming *hls);

Destroy HTTPLivestreaming struct.

### Initialization code for AES-128 encryption

    HTTPLiveStreaming *hls;

    hls = hls_create();
    hls->dir = "/some/output/dir";
    hls->use_encryption = 1;

    // URI for the key
    hls->encryption_key_uri = malloc(10);
    if (hls->encryption_key_uri == NULL) {
      perror("malloc hls->encryption_key_uri");
      return;
    }
    memcpy(hls->encryption_key_uri, "video.key", 10);

    // Byte array of the key
    hls->encryption_key = malloc(16);
    if (hls->encryption_key == NULL) {
      perror("malloc hls->encryption_key");
      return;
    }
    uint8_t tmp_key[] = {
      0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
      0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    };
    memcpy(hls->encryption_key, tmp_key, 16);

    // Byte array of the initialization vector
    hls->encryption_iv = malloc(16);
    if (hls->encryption_iv == NULL) {
      perror("malloc hls->encryption_iv");
      return;
    }
    uint8_t tmp_iv[] = {
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
    memcpy(hls->encryption_iv, tmp_iv, 16);

## MPEG-TS (mpegts.{c,h})

Write MPEG-TS with libavformat.

### Functions

    AVFormatContext *mpegts_create_context();

Prepare new MPEG-TS stream and returns the pointer to AVFormatContext.

    void mpegts_open_stream(AVFormatContext *format_ctx, char *filename, int dump_format);

Open new file for writing. If `dump_format` is 1, details of output format is written to the terminal.

    void mpegts_close_stream(AVFormatContext *format_ctx);

Close the file. If this function completes, the file is ready for play.

    void mpegts_destroy_context(AVFormatContext *format_ctx);

Destroy AVFormatContext that is pointed by `format_ctx`.

## hooks (hooks.{c,h})

Hook mechanism using inotify.

### Functions

    int clear_hooks(char *dirname);

Remove all hooks (files) that exist in `dirname`.

    void start_watching_hooks(pthread_t *thread, char *dir, void (*callback)(char *, char *), int read_content);

Start `thread` that watches for hooks in `dir`. `callback` function is called with the hook name as the first argument. If `read_content` is 1, contents of hook file is also passed to `callback` as second argument.

    void stop_watching_hooks();

Stop hook watcher thread.

## state (state.{c,h})

Output application state as files.

### Functions

    void state_set(char *dir, char *name, char *value);

Output state as a file `name` in directory `dir` with the contents of `value.`

    void state_get(char *dir, char *name, char **buf);

Get the state of a file `name` in directory `dir`. The value of the state is stored in `buf` as a string.

## config.h

Intended to be included from libraries.
