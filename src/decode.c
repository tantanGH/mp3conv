#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "decode.h"
#include "pcm.h"
#include "adpcm.h"

// decoder handle global reference for callback functions
static MP3_DECODE_HANDLE* g_decode;

//
//  libmad high level API callback: error
//
static enum mad_flow mad_callback_error(void* data, MAD_STREAM* stream, MAD_FRAME* frame) {

  MAD_BUFFER* buffer = data;

  printf("error: decoding error 0x%04x (%s) at byte offset %u\n",
	  stream->error, mad_stream_errorstr(stream),
	  stream->this_frame - buffer->start);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}

//
//  libmad high level API callback: input
//
static enum mad_flow mad_callback_input(void* data, MAD_STREAM* stream) {

  MAD_BUFFER* buffer = data;

  if (!buffer->length)
    return MAD_FLOW_STOP;

  mad_stream_buffer(stream, buffer->start, buffer->length);

  buffer->length = 0;

  return MAD_FLOW_CONTINUE;
}

//
//  libmad high level API callback: header
//
static enum mad_flow mad_callback_header(void* data, const MAD_HEADER* header) {

  if (g_decode->mp3_bit_rate < 0) {
    g_decode->mp3_bit_rate = header->bitrate;
  }
  
  if (g_decode->mp3_sample_rate < 0) {
    g_decode->mp3_sample_rate = header->samplerate;
    printf("MP3 sampling rate: %d [Hz]\n", header->samplerate);
  }
  
  return MAD_FLOW_CONTINUE;
}

//
//  inline helper: 24bit signed int to 16bit signed int
//
static inline int16_t scale(mad_fixed_t sample)
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
//  libmad high level API callback: output
//
static enum mad_flow mad_callback_output(void* data, const MAD_HEADER* header, MAD_PCM* pcm) {

  uint16_t nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];

  if (g_decode->mp3_num_channels < 0) {
    g_decode->mp3_num_channels = nchannels;
    printf("MP3 channels: %s\n", nchannels == 2 ? "stereo" : "mono");
  }

  int16_t* pcm_buffer = g_decode->pcm->buffer;
  size_t ofs = g_decode->pcm->buffer_ofs / 2;     // 8bit offset to 16bit offset

  while (nsamples--) {

    // in ADPCM output case, do down sampling
    if (g_decode->out_format == 0) {
      g_decode->consumed += ADPCM_SAMPLE_RATE;
      if (g_decode->consumed < g_decode->mp3_sample_rate) {
        left_ch++;
        if (nchannels == 2) right_ch++;
        continue;
      } else {
        g_decode->consumed -= g_decode->mp3_sample_rate;
      }
    }

    /* output sample(s) in 16-bit signed big-endian PCM */
    pcm_buffer[ ofs++ ] = scale(*left_ch++);

    if (nchannels == 2) {
      pcm_buffer[ ofs++ ] = scale(*right_ch++);
    }

    g_decode->pcm->num_samples++;
    g_decode->pcm->buffer_ofs = ofs * 2;    // 16bit offset to 8bit offset

  }

  if (g_decode->out_format == 0) {
    int16_t orig_buffer_id = g_decode->adpcm->current_buffer_id;
    adpcm_encode(g_decode->adpcm, pcm_buffer, ofs * 2, 16, nchannels);
    g_decode->pcm->buffer_ofs = 0;          // clear pcm buffer
    if (g_decode->out_fp != NULL) {
      int16_t buffer_id = g_decode->adpcm->current_buffer_id;
      if (buffer_id != orig_buffer_id) {
        adpcm_write_buffer(g_decode->adpcm, g_decode->out_fp, g_decode->adpcm->buffers[ orig_buffer_id ], ADPCM_BUFFER_SIZE);
        putchar('.');
      }
    }
  } else {
    if (g_decode->out_fp != NULL) {
      pcm_write(g_decode->pcm, g_decode->out_fp);
      putchar('.');
    }
  }

  return MAD_FLOW_CONTINUE;
}

//
//  init decoder handle
//
int32_t decode_init(MP3_DECODE_HANDLE* decode, PCM_HANDLE* pcm, ADPCM_HANDLE* adpcm, int16_t out_format, FILE* fp) {

  decode->mp3_bit_rate = -1;
  decode->mp3_sample_rate = -1;
  decode->mp3_num_channels = -1;
  decode->num_samples = 0;
  decode->consumed = 0;
  decode->out_format = out_format;
  decode->out_fp = fp;
  decode->pcm = pcm;
  decode->adpcm = adpcm;

  return 0;
}

//
//  close decoder handle
//
void decode_close(MP3_DECODE_HANDLE* decode) {
  if (decode->out_fp != NULL && decode->out_format == 0) {
    // write the remained data
    adpcm_write(decode->adpcm, decode->out_fp);
  }
}

//
//  decode mp3
//
int32_t decode_mp3(MP3_DECODE_HANDLE* decode, void* mp3_data, size_t mp3_data_len) {

  MAD_BUFFER buffer;
  MAD_DECODER decoder;
  int16_t result;

  /* initialize our private message structure */

  g_decode = decode;

  buffer.start  = mp3_data;
  buffer.length = mp3_data_len;

  /* configure input, output, and error functions */

  mad_decoder_init(&decoder, &buffer, mad_callback_input, mad_callback_header, NULL, mad_callback_output, mad_callback_error, NULL); 

  /* start decoding */

  result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

  /* release the decoder */

  mad_decoder_finish(&decoder);

  return result;
}
