#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iocslib.h>
#include "keyboard.h"
#include "himem.h"
#include "pcm.h"
#include "adpcm.h"
#include "nas_adpcm.h"
#include "mp3.h"

//
//  init mp3 decoder handle
//
int32_t mp3_init(MP3_DECODE_HANDLE* decode, PCM_WRITE_HANDLE* pcm, ADPCM_WRITE_HANDLE* adpcm, NAS_ADPCM_WRITE_HANDLE* nas, int16_t use_high_memory) {

  decode->pcm = pcm;
  decode->adpcm = adpcm;
  decode->nas = nas;

  decode->use_high_memory = use_high_memory;

  decode->mp3_sample_rate = -1;
  decode->mp3_channels = -1;

  memset(&(decode->mad_stream), 0, sizeof(MAD_STREAM));
  memset(&(decode->mad_frame), 0, sizeof(MAD_FRAME));
  memset(&(decode->mad_synth), 0, sizeof(MAD_SYNTH));
  memset(&(decode->mad_timer), 0, sizeof(MAD_TIMER));

  decode->buffer_len = MP3_DECODE_BUFFER_LEN;
  decode->buffer = himem_malloc(decode->buffer_len * sizeof(int16_t), decode->use_high_memory);

  return (decode->buffer != NULL) ? 0 : -1;
}

//
//  close mp3 decoder handle
//
void mp3_close(MP3_DECODE_HANDLE* decode) {
  // reclaim buffer
  if (decode->buffer != NULL) {
    himem_free(decode->buffer, decode->use_high_memory);
    decode->buffer = NULL;
  }
}

//
//  inline helper: 24bit signed int to 16bit signed int
//
static inline int16_t scale_16bit(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

//
//  decode mp3
//
int32_t mp3_decode(MP3_DECODE_HANDLE* decode, uint8_t* mp3_data, size_t mp3_data_len) {

  int32_t rc = -1;

  decode->mp3_sample_rate = -1;
  decode->mp3_channels = -1;
  
  uint32_t resample_counter = 0;
  size_t decoded_len = 0;
  size_t buffer_ofs = 0;

  mad_stream_init(&(decode->mad_stream));
  mad_frame_init(&(decode->mad_frame));
  mad_synth_init(&(decode->mad_synth));
  mad_timer_reset(&(decode->mad_timer));

  mad_stream_buffer(&(decode->mad_stream), mp3_data, mp3_data_len);

  uint32_t t0 = ONTIME();

  for (;;) {

    // check esc key to exit
    if (B_KEYSNS() != 0) {
      int16_t scan_code = B_KEYINP() >> 8;
      if (scan_code == KEY_SCAN_CODE_ESC || scan_code == KEY_SCAN_CODE_Q) {
        printf("\rcanceled.\x1b[0K\n");
        rc = 1;
        goto exit;
      }
    }

    int16_t result = mad_frame_decode(&(decode->mad_frame), &(decode->mad_stream));
    if (result == -1) {
      if (decode->mad_stream.error == MAD_ERROR_BUFLEN) {
        // MP3 EOF
        break;
      } else if (MAD_RECOVERABLE(decode->mad_stream.error)) {
        continue;
      } else {
        printf("error: %s\x1b[0K\n", mad_stream_errorstr(&(decode->mad_stream)));
        goto exit;
      }
    }

    mad_synth_frame(&(decode->mad_synth), &(decode->mad_frame));
    mad_timer_add(&(decode->mad_timer), decode->mad_frame.header.duration);

    MAD_PCM* mad_pcm = &(decode->mad_synth.pcm);

    // extract sample rate and channels
    if (decode->mp3_sample_rate < 0) {
      printf("MP3 sample rate: %d [Hz]\n", mad_pcm->samplerate);
      printf("MP3 channels: %s\n\n", mad_pcm->channels == 1 ? "mono" : "stereo");
      decode->mp3_sample_rate = mad_pcm->samplerate;
      decode->mp3_channels = mad_pcm->channels;
    }

    if (mad_pcm->channels == 2) {

      if (decode->adpcm != NULL) {

        for (int32_t i = 0; i < mad_pcm->length; i++) {

          // down sampling
          resample_counter += ADPCM_SAMPLE_RATE;
          if (resample_counter < mad_pcm->samplerate) {
            continue;
          }
          resample_counter -= mad_pcm->samplerate;

          decode->buffer[ buffer_ofs++ ] = scale_16bit(mad_pcm->samples[0][i]);
          decode->buffer[ buffer_ofs++ ] = scale_16bit(mad_pcm->samples[1][i]);

          if (buffer_ofs >= decode->buffer_len) {
            if (adpcm_write(decode->adpcm, decode->buffer, buffer_ofs, mad_pcm->channels) != 0) {
              goto exit;
            }
            decoded_len += buffer_ofs;
            buffer_ofs = 0;
            printf("\rconverting to MSM6258V ADPCM... (%d samples in %4.2f sec) [ESC] to cancel", decoded_len, (ONTIME() - t0) / 100.0);
          }

        }

      } else if (decode->pcm != NULL) {

        for (int32_t i = 0; i < mad_pcm->length; i++) {

          decode->buffer[ buffer_ofs++ ] = scale_16bit(mad_pcm->samples[0][i]);
          decode->buffer[ buffer_ofs++ ] = scale_16bit(mad_pcm->samples[1][i]);

          if (buffer_ofs >= decode->buffer_len) {
            if (pcm_write(decode->pcm, decode->buffer, buffer_ofs) != 0) {
              goto exit;
            }
            decoded_len += buffer_ofs;
            buffer_ofs = 0;
            printf("\rconverting to 16bit raw PCM (big)... (%d samples in %4.2f sec) [ESC] to cancel", decoded_len, (ONTIME() - t0) / 100.0);
          }

        }

      } else if (decode->nas != NULL) {

        for (int32_t i = 0; i < mad_pcm->length; i++) {

          decode->buffer[ buffer_ofs++ ] = scale_16bit(mad_pcm->samples[0][i]);
          decode->buffer[ buffer_ofs++ ] = scale_16bit(mad_pcm->samples[1][i]);

          if (buffer_ofs >= decode->buffer_len) {
            if (nas_adpcm_write(decode->nas, decode->buffer, buffer_ofs, mad_pcm->channels) != 0) {
              goto exit;
            }
            decoded_len += buffer_ofs;
            buffer_ofs = 0;
            printf("\rconverting to YM2608 ADPCM... (%d samples in %4.2f sec) [ESC] to cancel", decoded_len, (ONTIME() - t0) / 100.0);
          }

        }

      }

    } else {

      if (decode->adpcm != NULL) {

        for (int32_t i = 0; i < mad_pcm->length; i++) {

          // down sampling
          resample_counter += ADPCM_SAMPLE_RATE;
          if (resample_counter < mad_pcm->samplerate) {
            continue;
          }
          resample_counter -= mad_pcm->samplerate;

          decode->buffer[ buffer_ofs++ ] = scale_16bit(mad_pcm->samples[0][i]);

          if (buffer_ofs >= decode->buffer_len) {
            if (adpcm_write(decode->adpcm, decode->buffer, buffer_ofs, mad_pcm->channels) != 0) {
              goto exit;
            }
            decoded_len += buffer_ofs;
            buffer_ofs = 0;
            printf("\rconverting to MSM6258V ADPCM... (%d samples in %4.2f sec) [ESC] to cancel", decoded_len, (ONTIME() - t0) / 100.0);
          }

        }

      } else if (decode->pcm != NULL) {

        for (int32_t i = 0; i < mad_pcm->length; i++) {

          decode->buffer[ buffer_ofs++ ] = scale_16bit(mad_pcm->samples[0][i]);

          if (buffer_ofs >= decode->buffer_len) {
            if (pcm_write(decode->pcm, decode->buffer, buffer_ofs) != 0) {
              goto exit;
            }
            decoded_len += buffer_ofs;
            buffer_ofs = 0;
            printf("\rconverting to 16bit raw PCM (big)... (%d samples in %4.2f sec) [ESC] to cancel", decoded_len, (ONTIME() - t0) / 100.0);
          }

        }

      } else if (decode->nas != NULL) {

        for (int32_t i = 0; i < mad_pcm->length; i++) {

          decode->buffer[ buffer_ofs++ ] = scale_16bit(mad_pcm->samples[0][i]);

          if (buffer_ofs >= decode->buffer_len) {
            if (nas_adpcm_write(decode->nas, decode->buffer, buffer_ofs, mad_pcm->channels) != 0) {
              goto exit;
            }
            decoded_len += buffer_ofs;
            buffer_ofs = 0;
            printf("\rconverting to YM2608 ADPCM... (%d samples in %4.2f sec) [ESC] to cancel", decoded_len, (ONTIME() - t0) / 100.0);
          }

        }

      }

    }

  }

  // flush unwritten data
  if (buffer_ofs > 0) {
    if (decode->adpcm != NULL) {
      if (adpcm_write(decode->adpcm, decode->buffer, buffer_ofs, decode->mp3_channels) != 0) {
        goto exit;
      }
      decoded_len += buffer_ofs;
      buffer_ofs = 0;
      adpcm_flush(decode->adpcm);
    } else if (decode->pcm != NULL) {
      if (pcm_write(decode->pcm, decode->buffer, buffer_ofs) != 0) {
        goto exit;
      }
      decoded_len += buffer_ofs;
      buffer_ofs = 0;
      pcm_flush(decode->pcm);
    } else if (decode->nas != NULL) {
      if (nas_adpcm_write(decode->nas, decode->buffer, buffer_ofs, decode->mp3_channels) != 0) {
        goto exit;
      }
      decoded_len += buffer_ofs;
      buffer_ofs = 0;
      nas_adpcm_flush(decode->nas);
    }
  }

  printf("\rcompleted in %4.2f sec.\x1b[0K\n", (ONTIME() - t0) / 100.0);

  rc = 0;

exit:
  return rc;
}
