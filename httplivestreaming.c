#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

#include "mpegts.h"
#include "httplivestreaming.h"

void encrypt_most_recent_file(HTTPLiveStreaming *hls) {
  char filepath[1024];
  FILE *fp;
  uint8_t *input_data;
  uint8_t *encrypted_data;
  int input_size;
  EVP_CIPHER_CTX enc_ctx;

  // init cipher context
  EVP_CIPHER_CTX_init(&enc_ctx);
  if (hls->encryption_key == NULL) {
    fprintf(stderr, "Warning: encryption_key is not set\n");
  }
  if (hls->encryption_iv == NULL) {
    fprintf(stderr, "Warning: encryption_iv is not set\n");
  }
  EVP_EncryptInit_ex(&enc_ctx, EVP_aes_128_cbc(), NULL, hls->encryption_key, hls->encryption_iv);

  // read original data
  snprintf(filepath, 1024, "%s/%d.ts", hls->dir, hls->most_recent_number);
  fp = fopen(filepath, "rb+");
  if (fp == NULL) {
    perror("Read ts file failed");
    return;
  }
  fseek(fp, 0, SEEK_END);
  input_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  input_data = malloc(input_size);
  if (input_data == NULL) {
    perror("Can't malloc for input_data");
    return;
  }
  fread(input_data, 1, input_size, fp);

  // encrypt the data
  int c_len = input_size + AES_BLOCK_SIZE;
  int f_len;
  int encrypted_size;
  encrypted_data = malloc(c_len);
  if (encrypted_data == NULL) {
    perror("Can't malloc for encrypted_data");
    return;
  }
  EVP_EncryptUpdate(&enc_ctx, encrypted_data, &c_len, input_data, input_size);
  EVP_EncryptFinal_ex(&enc_ctx, encrypted_data+c_len, &f_len);
  encrypted_size = c_len + f_len;

  // write data to the same file
  fseek(fp, 0, SEEK_SET);
  fwrite(encrypted_data, 1, encrypted_size, fp);
  ftruncate(fileno(fp), encrypted_size);
  fclose(fp);

  // free up variables
  free(encrypted_data);
  free(input_data);
  EVP_CIPHER_CTX_cleanup(&enc_ctx);
}

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
  snprintf(buf, 128, "#EXTM3U\n#EXT-X-VERSION:5\n#EXT-X-TARGETDURATION:%d\n#EXT-X-MEDIA-SEQUENCE:%d\n",
      hls->segment_duration, hls->most_recent_number);
  fwrite(buf, 1, strlen(buf), file);

  // insert encryption header if needed
  if (hls->use_encryption) {
    if (hls->encryption_key_uri == NULL) {
      fprintf(stderr, "Error: encryption_key_uri is not set\n");
    } else {
      snprintf(buf, 128, "#EXT-X-KEY:METHOD=AES-128,URI=\"%s\",IV=0x",
          hls->encryption_key_uri);
      fwrite(buf, 1, strlen(buf), file);
      for (i = 0; i < 16; i++) {
        snprintf(buf + i * 2, 3, "%02x", hls->encryption_iv[i]);
      }
      snprintf(buf + 32, 2, "\n");
      fwrite(buf, 1, 33, file);
    }
  }

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

  int last_seq = hls->most_recent_number - hls->num_recent_files - hls->num_retained_old_files;
  if (last_seq >= 1) {
    snprintf(filepath, 1024, "%s/%d.ts", hls->dir, last_seq);
    unlink(filepath);
  }

  return 0;
}

void hls_destroy(HTTPLiveStreaming *hls) {
  mpegts_close_stream(hls->format_ctx);
  if (hls->use_encryption) {
    encrypt_most_recent_file(hls);
    if (hls->encryption_key_uri != NULL) {
      free(hls->encryption_key_uri);
    }
    if (hls->encryption_key != NULL) {
      free(hls->encryption_key);
    }
    if (hls->encryption_iv != NULL) {
      free(hls->encryption_iv);
    }
  }
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
    if (hls->use_encryption) {
      encrypt_most_recent_file(hls);
    }
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
  hls->num_recent_files = 3;
  hls->num_retained_old_files = 10;
  hls->most_recent_number = 0;
  hls->segment_duration = 1;
  hls->dir = ".";
  hls->is_started = 0;
  hls->use_encryption = 0;
  hls->encryption_key_uri = NULL;
  hls->encryption_key = NULL;
  hls->encryption_iv = NULL;
  return hls;
}
