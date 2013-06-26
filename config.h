#ifndef _CLIB_CONFIG_H_
#define _CLIB_CONFIG_H_

#if defined(__cplusplus)
extern "C" {
#endif

// Dimension of video image
#define WIDTH 480
#define HEIGHT 360

// Video frame rate
#define TARGET_FPS 30

// Bit rate of H.264 video
#define H264_BIT_RATE 500000

// Sample rate of audio
#define AUDIO_SAMPLE_RATE 22050

// Bit rate of AAC audio
#define AAC_BIT_RATE 32000

#if defined(__cplusplus)
}
#endif

#endif
