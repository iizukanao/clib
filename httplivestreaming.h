#ifndef _CLIB_HTTPLIVESTREAMING_H_
#define _CLIB_HTTPLIVESTREAMING_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <libavformat/avformat.h>

typedef struct HTTPLiveStreaming {
  AVFormatContext *format_ctx;
  char *index_filename;
  int num_recent_files;
  int most_recent_number;
  int segment_duration;
  char *dir;
  int is_started;
  int use_encryption;
  char *encryption_key_uri;
  uint8_t *encryption_key;
  uint8_t *encryption_iv;
} HTTPLiveStreaming;

HTTPLiveStreaming *hls_create();
int hls_write_packet(HTTPLiveStreaming *hls, AVPacket *pkt, int split);
void hls_destroy(HTTPLiveStreaming *hls);

#if defined(__cplusplus)
}
#endif

#endif
