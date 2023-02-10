#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stat.h>
#include <iocslib.h>
#include "pcm.h"
#include "adpcm.h"
#include "decode.h"
#include "memory.h"
#include "mp3ex.h"

//
//  show help messages
//
static void show_help_message() {
  printf("usage: mp3ex.x [options] <mp3-file>\n");
  printf("options:\n");
  printf("   -o <out-file> ... output file name\n");
  printf("   -a ... output in ADPCM format (default)\n");
  printf("   -p ... output in PCM format\n");
  printf("   -h ... show help message\n");
}

int32_t main(int32_t argc, uint8_t* argv[]) {

  // default exit code
  int32_t rc = 1;

  // output format (0:ADPCM, 1:PCM)
  int16_t out_format = 0;

  // input file name
  uint8_t* mp3_file_name = NULL;

  // output file name
  uint8_t* out_file_name = NULL;

  // mp3 read buffer pointer
  uint8_t* mp3_buffer = NULL;

  // pcm handle
  PCM_HANDLE pcm = { 0 };

  // adpcm handle
  ADPCM_HANDLE adpcm = { 0 };

  // decoder handle
  MP3_DECODE_HANDLE mp3 = { 0 };

  // show title and version
  printf("MP3EX.X - MP3 to PCM converter and player version " VERSION " by tantan\n");

  // argument options
  if (argc <= 1) {
    show_help_message();
    goto exit;
  }

  for (int16_t i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && strlen(argv[i]) >= 2) {
      if (argv[i][1] == 'o' && i+1<argc) {
        out_file_name = argv[i+1];
      } else if (argv[i][1] == 'a') {
        out_format = 0;
      } else if (argv[i][1] == 'p') {
        out_format = 1;
      } else if (argv[i][1] == 'h') {
        show_help_message();
        goto exit;
      } else {
        printf("error: unknown option (%s).\n",argv[i]);
        goto exit;
      }
    } else {
      mp3_file_name = argv[i];
    }
  }

  if (mp3_file_name == NULL) {
    printf("error: no input file.\n");
    goto exit;
  }

  // open MP3 file
  FILE* fp_mp3 = fopen(mp3_file_name, "rb");
  if (fp_mp3 == NULL) {
    printf("error: cannot open mp3 file (%s).\n", mp3_file_name);
    goto catch;
  }
  
  // read the first 10 bytes of the MP3 file
  uint32_t mp3_offset = 0;
  uint8_t mp3_header[10];
  size_t ret = fread(mp3_header, 1, 10, fp_mp3);
  if (ret != 10) {
    printf("error: cannot read mp3 file.\n");
    goto catch;
  }

  // check if the MP3 file has an ID3v2 tag
  if (mp3_header[0] == 'I' && mp3_header[1] == 'D' && mp3_header[2] == '3') {
    // Extract the tag size
    uint32_t tag_size = ((mp3_header[6] & 0x7f) << 21) | ((mp3_header[7] & 0x7f) << 14) |
                        ((mp3_header[8] & 0x7f) << 7) | (mp3_header[9] & 0x7f);

    mp3_offset = tag_size + 10;
  }

  // check mp3 size
  fseek(fp_mp3, 0, SEEK_END);
  size_t mp3_len = ftell(fp_mp3) - mp3_offset;
  fseek(fp_mp3, mp3_offset, SEEK_SET);

  // mp3 info
  printf("MP3 file name: %s\n", mp3_file_name);
  printf("MP3 data size: %d [bytes]\n", mp3_len);

  // allocate buffer in high memory
  mp3_buffer = malloc_himem(mp3_len, 1);
  if (mp3_buffer == NULL) {
    printf("error: cannot allocate high memory buffer for mp3.\n");
    goto catch;
  }

  // read mp3 to high memory
  uint32_t ofs = 0;
  size_t len;
  do {
    len = fread(mp3_buffer + ofs, 1, mp3_len - ofs, fp_mp3);
    ofs += len;
  } while (ofs < mp3_len);
  fclose(fp_mp3);
  fp_mp3 = NULL;

  // init pcm handle
  if (pcm_init(&pcm, PCM_BUFFER_LEN) != 0) {
    printf("error: pcm handle init error.\n");
    goto catch;
  }

  // init adpcm handle
  if (adpcm_init(&adpcm, ADPCM_BUFFER_LEN) != 0) {
    printf("error: adpcm handle init error.\n");
    goto catch;
  }

  // open output file
  FILE* fp_out = NULL;
  if (out_file_name != NULL) {

    struct stat stat_buf;
    if (stat(out_file_name, &stat_buf) == 0) {
      printf("warning: output file (%s) already exists. overwrite? (y/n)", out_file_name);
      uint8_t c;
      scanf("%c",&c);
      if (c != 'y' && c != 'Y') {
        printf("canceled.\n");
        goto catch;            
      }
    }

    fp_out = fopen(out_file_name, "wb");
    if (fp_out == NULL) {
      printf("error: cannot open output file (%s).\n", out_file_name);
      goto catch;
    }
  }

  // init mp3 decoder handle
  if (decode_init(&mp3, &pcm, &adpcm, out_format, fp_out) != 0) {
    printf("mp3 decode handle init error.\n");
    goto catch;
  }

  // decode mp3
  uint32_t t0 = ONTIME();
  decode_mp3(&mp3, mp3_buffer, mp3_len);
  uint32_t t1 = ONTIME();

  printf("\nCompleted in %4.2f seconds.\n",(t1-t0)/100.0);

  rc = 0;

catch:

  // close handlers
  decode_close(&mp3);
  adpcm_close(&adpcm);
  pcm_close(&pcm);

  // reclaim mp3 buffer high memory
  if (mp3_buffer != NULL) {
    free_himem(mp3_buffer, 1);
    mp3_buffer = NULL;
  }

  // close files if opened
  if (fp_mp3 != NULL) {
    fclose(fp_mp3);
    fp_mp3 = NULL;
  }
  if (fp_out != NULL) {
    fclose(fp_out);
    fp_out = NULL;
  }

exit:
  return rc;
}