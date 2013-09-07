#ifndef _CLIB_MPEGTS_H_
#define _CLIB_MPEGTS_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <libavformat/avformat.h>

AVFormatContext *mpegts_create_context();
void mpegts_open_stream(AVFormatContext *format_ctx, char *filename, int dump_format);
void mpegts_open_stream_without_header(AVFormatContext *format_ctx, char *filename, int dump_format);
void mpegts_close_stream(AVFormatContext *format_ctx);
void mpegts_close_stream_without_trailer(AVFormatContext *format_ctx);
void mpegts_destroy_context(AVFormatContext *format_ctx);

#if defined(__cplusplus)
}
#endif

#endif
