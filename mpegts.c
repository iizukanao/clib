#include <libavutil/avutil.h>

#include "mpegts.h"
#include "config.h"

void setup_video_stream(AVFormatContext *format_ctx) {
  AVStream *video_stream;
  AVCodecContext *video_codec_ctx;

  video_stream = avformat_new_stream(format_ctx, 0);
  if (!video_stream) {
    fprintf(stderr, "avformat_new_stream failed\n");
    exit(1);
  }
  video_stream->id = format_ctx->nb_streams - 1;

  video_codec_ctx                = video_stream->codec;
  video_codec_ctx->codec_id      = CODEC_ID_H264;
  video_codec_ctx->codec_type    = AVMEDIA_TYPE_VIDEO;
  video_codec_ctx->codec_tag     = 0;
  video_codec_ctx->bit_rate      = H264_BIT_RATE;
  video_codec_ctx->profile       = FF_PROFILE_H264_CONSTRAINED_BASELINE;
  video_codec_ctx->level         = 30;
  video_codec_ctx->time_base.num = 1;
  video_codec_ctx->time_base.den = TARGET_FPS;
  video_codec_ctx->pix_fmt       = 0;
  video_codec_ctx->width         = WIDTH;
  video_codec_ctx->height        = HEIGHT;
  video_codec_ctx->has_b_frames  = 0;
  video_codec_ctx->flags         |= CODEC_FLAG_GLOBAL_HEADER;
}

static int is_sample_fmt_supported(AVCodec *codec, enum AVSampleFormat sample_fmt) {
  const enum AVSampleFormat *p = codec->sample_fmts;

  while (*p != AV_SAMPLE_FMT_NONE) {
    if (*p == sample_fmt) {
      return 1;
    }
    p++;
  }

  return 0;
}

void setup_audio_stream(AVFormatContext *format_ctx) {
  AVCodec *aac_codec;
  AVCodecContext *audio_codec_ctx = NULL;
  AVStream *audio_stream;

  aac_codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
  if (!aac_codec) {
    fprintf(stderr, "codec not found\n");
    exit(1);
  }

  audio_stream = avformat_new_stream(format_ctx, aac_codec);
  if (!audio_stream) {
    fprintf(stderr, "avformat_new_stream for audio error\n");
    exit(1);
  }
  audio_stream->id = format_ctx->nb_streams - 1;
  audio_codec_ctx = audio_stream->codec;

  audio_codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
  if ( ! is_sample_fmt_supported(aac_codec, audio_codec_ctx->sample_fmt) ) {
    fprintf(stderr, "Sample format %s is not supported",
        av_get_sample_fmt_name(audio_codec_ctx->sample_fmt));
    exit(1);
  }

  audio_codec_ctx->bit_rate = AAC_BIT_RATE;
  audio_codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
  audio_codec_ctx->profile = FF_PROFILE_AAC_LOW;
  audio_codec_ctx->sample_rate = AUDIO_SAMPLE_RATE;
  audio_codec_ctx->channel_layout = AV_CH_LAYOUT_MONO;
  audio_codec_ctx->channels = av_get_channel_layout_nb_channels(audio_codec_ctx->channel_layout);

  if (avcodec_open2(audio_codec_ctx, aac_codec, NULL) < 0) {
    fprintf(stderr, "avcodec_open failed\n");
    exit(1);
  }
}

void mpegts_destroy_context(AVFormatContext *format_ctx) {
  int i;
  for (i = 0; i < format_ctx->nb_streams; i++) {
    avcodec_close(format_ctx->streams[i]->codec);
  }
  av_free(format_ctx);
}

void mpegts_close_stream(AVFormatContext *format_ctx) {
  av_write_trailer(format_ctx);
  avio_close(format_ctx->pb);
}

void mpegts_open_stream(AVFormatContext *format_ctx, char *outputfilename, int dump_format) {
  if (dump_format) {
    av_dump_format(format_ctx, 0, outputfilename, 1);
  }

  if (strcmp(outputfilename, "-") == 0) {
    outputfilename = "pipe:1";
  }

  if (avio_open(&format_ctx->pb, outputfilename, AVIO_FLAG_WRITE) < 0) {
    fprintf(stderr, "avio_open failed\n");
    exit(1);
  }

  if (avformat_write_header(format_ctx, NULL)) {
    fprintf(stderr, "avformat_write_header failed\n");
    exit(1);
  }
}

AVFormatContext *mpegts_create_context() {
  AVFormatContext *format_ctx;
  AVOutputFormat *out_fmt;

  av_register_all();

  out_fmt = av_guess_format("mpegts", NULL, NULL);
  out_fmt->flags |= ~AVFMT_GLOBALHEADER;
  if (!out_fmt) {
    fprintf(stderr, "av_guess_format failed\n");
    exit(1);
  }

  format_ctx = avformat_alloc_context();
  if (!format_ctx) {
    fprintf(stderr, "avformat_alloc_context failed\n");
    exit(1);
  }
  format_ctx->oformat = out_fmt;

  setup_video_stream(format_ctx);
  setup_audio_stream(format_ctx);

  return format_ctx;
}
