#include <unistd.h>

#include "mpegts.h"
#include "httplivestreaming.h"

int write_index(HTTPLiveStreaming *hls, int is_end) {
  FILE *file;
  char buf[128];
  char tmp_filepath[1024];
  char filepath[1024];
  int i;

  snprintf(tmp_filepath, 1024, "%s/_%s", hls->dir, hls->index_filename);
  file = fopen(tmp_filepath, "w");
  if (!file) {
    perror("fopen");
    return -1;
  }

  // header
  snprintf(buf, 128, "#EXTM3U\n#EXT-X-TARGETDURATION:%d\n#EXT-X-MEDIA-SEQUENCE:%d\n",
      hls->segment_duration, hls->most_recent_number);
  fwrite(buf, 1, strlen(buf), file);

  // segments
  int from_seq = hls->most_recent_number - hls->num_recent_files + 1;
  if (from_seq < 1) {
    from_seq = 1;
  }
  for (i = from_seq; i <= hls->most_recent_number; i++) {
    snprintf(buf, 128, "#EXTINF:%d,\n%d.ts\n",
        hls->segment_duration, i);
    fwrite(buf, 1, strlen(buf), file);
  }

  if (is_end) {
    // end mark
    fwrite("#EXT-X-ENDLIST\n", 1, 15, file);
  }

  fclose(file);

  snprintf(filepath, 1024, "%s/%s", hls->dir, hls->index_filename);
  rename(tmp_filepath, filepath);

  int last_seq = hls->most_recent_number - hls->num_recent_files;
  if (last_seq >= 1) {
    snprintf(filepath, 1024, "%s/%d.ts", hls->dir, last_seq);
    unlink(filepath);
  }

  return 0;
}

void hls_destroy(HTTPLiveStreaming *hls) {
  mpegts_close_stream(hls->format_ctx);
  write_index(hls, 1);
  mpegts_destroy_context(hls->format_ctx);
  free(hls);
}

void create_new_ts(HTTPLiveStreaming *hls) {
  char filepath[1024];

  hls->most_recent_number++;
  snprintf(filepath, 1024, "%s/%d.ts", hls->dir, hls->most_recent_number);
  mpegts_open_stream(hls->format_ctx, filepath, 0);
}

int hls_write_packet(HTTPLiveStreaming *hls, AVPacket *pkt, int split) {
  if ( ! hls->is_started ) {
    hls->is_started = 1;
    create_new_ts(hls);
  }

  if (split) {
    mpegts_close_stream(hls->format_ctx);
    write_index(hls, 0);
    create_new_ts(hls);
  }

  return av_interleaved_write_frame(hls->format_ctx, pkt);
}

HTTPLiveStreaming *hls_create() {
  HTTPLiveStreaming *hls = malloc(sizeof(HTTPLiveStreaming));
  AVFormatContext *format_ctx = mpegts_create_context();
  hls->format_ctx = format_ctx;
  hls->index_filename = "index.m3u8";
  hls->num_recent_files = 5;
  hls->most_recent_number = 0;
  hls->segment_duration = 2;
  hls->dir = ".";
  hls->is_started = 0;
  return hls;
}
