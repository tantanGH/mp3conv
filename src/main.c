#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <iocslib.h>
#include <doslib.h>
#include "keyboard.h"
#include "himem.h"
#include "pcm_encode.h"
#include "adpcm_encode.h"
#include "ym2608_encode.h"
#include "mp3_decode.h"
#include "mp3conv.h"

//
//  show help messages
//
static void show_help_message() {
  printf("usage: mp3conv.x [options] <mp3-file>\n");
  printf("options:\n");
  printf("   -a ... output in 4bit MSM6258V ADPCM format (.pcm, default)\n");
  printf("   -p ... output in 16bit big endian raw format (.sXX|.mXX) format\n");
  printf("   -n ... output in 4bit YM2608 ADPCM (.aXX|.nXX) format\n");
  printf("   -v<n> ... output volume (1-192, default:100)\n");
//  printf("   -u ... use 060turbo/TS-6BE16 high memory\n");
  printf("   -h ... show help message\n");
  printf("   -o <output-file> ... output file name (default:auto assign)\n");
}

int32_t main(int32_t argc, uint8_t* argv[]) {

  // default exit code
  int32_t rc = 1;

  // output format (-1:NONE, 0:ADPCM, 1:PCM, 2:YM2608 ADPCM)
  int16_t out_format = OUTPUT_FORMAT_NONE;

  // output volume
  int16_t out_volume = 100;

  // use high memory
  int16_t use_high_memory = 0;

  // input file name
  uint8_t* mp3_file_name = NULL;

  // output staging file name
  static uint8_t out_staging_file_name[ MAX_PATH_LEN ];
  out_staging_file_name[0] = '\0';

  // output file name
  static uint8_t out_file_name[ MAX_PATH_LEN ];
  out_file_name[0] = '\0';

  // mp3 read buffer pointer
  uint8_t* mp3_buffer = NULL;
  uint8_t* mp3_staging_buffer = NULL;

  // mp3 file handle
  FILE* fp_mp3 = NULL;

  // output file handle
  FILE* fp_out = NULL;

  // mp3 decode handle
  MP3_DECODE_HANDLE mp3 = { 0 };

  // pcm encode handle
  PCM_ENCODE_HANDLE pcm = { 0 };

  // adpcm encode handle
  ADPCM_ENCODE_HANDLE adpcm = { 0 };

  // ym2608 adpcm encode handle
  YM2608_ENCODE_HANDLE nas = { 0 };

  // show title and version
  printf("MP3CONV.X - MP3 to ADPCM/PCM converter for X680x0 version " VERSION " by tantan\n");

  // argument options
  if (argc <= 1) {
    show_help_message();
    goto exit;
  }

  for (int16_t i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && strlen(argv[i]) >= 2) {
      if (argv[i][1] == 'a') {
        if (out_format != OUTPUT_FORMAT_NONE) {
          printf("error: you can select an output format only.\n");
          goto exit;
        }
        out_format = OUTPUT_FORMAT_ADPCM;
      } else if (argv[i][1] == 'p') {
        if (out_format != OUTPUT_FORMAT_NONE) {
          printf("error: you can select an output format only.\n");
          goto exit;
        }
        out_format = OUTPUT_FORMAT_PCM;
      } else if (argv[i][1] == 'n') {
        if (out_format != OUTPUT_FORMAT_NONE) {
          printf("error: you can select an output format only.\n");
          goto exit;
        }
        out_format = OUTPUT_FORMAT_YM2608;
      } else if (argv[i][1] == 'v') {
        out_volume = atoi(argv[i]+2);
        if (out_volume < 1 || out_volume > 192) {
          printf("error: volume range is 1 to 192.\n");
          goto exit;
        }
//      } else if (argv[i][1] == 'u') {
//        if (!himem_isavailable()) {
//          printf("error: high memory driver is not installed.\n");
//          goto exit;
//        }
//        use_high_memory = 1;
      } else if (argv[i][1] == 'h') {
        show_help_message();
        goto exit;
      } else if (argv[i][1] == 'o' && i+1 < argc) {
        strcpy(out_file_name, argv[i+1]);
        strcpy(out_staging_file_name, argv[i+1]);
        i++;
      } else {
        printf("error: unknown option (%s).\n",argv[i]);
        goto exit;
      }
    } else {
      if (mp3_file_name == NULL) {
        mp3_file_name = argv[i];
      } else {
        show_help_message();
        goto exit;
      }
    }
  }

  // input file name check
  if (mp3_file_name == NULL) {
    printf("error: no mp3 file is specified.\n");
    goto exit;
  }
  if (strlen(mp3_file_name) < 5 || stricmp(mp3_file_name + strlen(mp3_file_name) - 4, ".mp3") != 0) {
    printf("error: not a mp3 file (%s).\n", mp3_file_name);
    goto exit;
  }

  // output format confirmation
  if (out_format == OUTPUT_FORMAT_NONE) {
    out_format = OUTPUT_FORMAT_ADPCM;
  }

  // high memory
  if (himem_isavailable()) {
    use_high_memory = 1;
  }

  // open input MP3 file
  fp_mp3 = fopen(mp3_file_name, "rb");
  if (fp_mp3 == NULL) {
    printf("error: cannot open mp3 file (%s).\n", mp3_file_name);
    goto catch;
  }
  
  // read the first 10 bytes of the MP3 file
  uint32_t mp3_offset = 0;
  uint8_t mp3_header[10];
  size_t ret = fread(mp3_header, 1, 10, fp_mp3);
  if (ret != 10) {
    printf("error: cannot read mp3 file (%s).\n", mp3_file_name);
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
  printf("\nMP3 file name: %s\n", mp3_file_name);
  printf("MP3 data size: %d [bytes]\n", mp3_len);

  // allocate mp3 read staging buffer on main memory
  mp3_staging_buffer = himem_malloc(MP3_STAGING_BUFFER_BYTES, 0);     // must be main memory
  if (mp3_staging_buffer == NULL) {
    printf("error: out of memory (cannot allocate memory for mp3 staging read).\n");
    goto catch;
  }    

  // allocate mp3 read buffer
  mp3_buffer = himem_malloc(mp3_len, use_high_memory);
  if (mp3_buffer == NULL) {
    printf("error: out of memory (cannot allocate memory for mp3 read).\n");
    goto catch;
  }

  // read mp3
  printf("\nloading MP3 file into %s memory...\x1b[0K", use_high_memory ? "high" : "main");
  uint32_t read_len = 0;
  do {

    // check esc key to exit
    if (B_KEYSNS() != 0) {
      int16_t scan_code = B_KEYINP() >> 8;
      if (scan_code == KEY_SCAN_CODE_ESC || scan_code == KEY_SCAN_CODE_Q) {
        printf("\rcanceled.\x1b[0K\n");
        rc = 1;
        goto catch;
      }
    }

    size_t len = fread(mp3_staging_buffer, 1, MP3_STAGING_BUFFER_BYTES, fp_mp3);
    memcpy(mp3_buffer + read_len, mp3_staging_buffer, len);
    read_len += len;

  } while (read_len < mp3_len);

  fclose(fp_mp3);
  fp_mp3 = NULL;

  himem_free(mp3_staging_buffer, 0);    // main memory
  mp3_staging_buffer = NULL;

  printf("\rloaded MP3 file into %s memory.\x1b[0K\n\n", use_high_memory ? "high" : "main");
  
  // output staging file name
  if (out_staging_file_name[0] == '\0') {
    strcpy(out_staging_file_name, mp3_file_name);
  }
  int16_t out_file_name_len = strlen(out_staging_file_name);
  if (out_staging_file_name[out_file_name_len - 4] != '.') {
    printf("error: incorrect output file name.\n");
    goto exit;
  }
  out_staging_file_name[ out_file_name_len - 3 ] = '%';
  out_staging_file_name[ out_file_name_len - 2 ] = '%';
  out_staging_file_name[ out_file_name_len - 1 ] = '%';

  // output staging file overwrite check
  struct FILBUF filbuf;
  if (FILES(&filbuf, out_staging_file_name, 0x20) >= 0) {
    printf("warning: output staging file (%s) already exists. overwrite? (y/n)", out_staging_file_name);
    uint8_t c;
    do {
      c = INKEY();
      if (c == 'n' || c == 'N') {
        printf("\ncanceled.\n");
        goto catch;
      }
    } while (c != 'y' && c != 'Y');
    printf("\n");
  }

  // open output staging file
  fp_out = fopen(out_staging_file_name, "wb");
  if (fp_out == NULL) {
    printf("error: cannot open output staging file (%s).\n", out_staging_file_name);
    goto catch;
  }

  if (out_format == OUTPUT_FORMAT_ADPCM) {

    // init adpcm encode handle
    if (adpcm_encode_init(&adpcm, fp_out, use_high_memory) != 0) {
      printf("error: out of memory (adpcm encode handle init error).\n");
      goto catch;
    }

    // init mp3 decoder handle
    if (mp3_init(&mp3, NULL, &adpcm, NULL, out_volume, use_high_memory) != 0) {
      printf("error: out of memory (mp3 decode handle init error).\n");
      goto catch;
    }

  } else if (out_format == OUTPUT_FORMAT_PCM) {

    // init pcm encode handle
    if (pcm_encode_init(&pcm, fp_out, use_high_memory) != 0) {
      printf("error: out of memory (pcm encode handle init error).\n");
      goto catch;
    }

    // init mp3 decoder handle
    if (mp3_init(&mp3, &pcm, NULL, NULL, out_volume, use_high_memory) != 0) {
      printf("error: out of memory (mp3 decode handle init error).\n");
      goto catch;
    }

  } else if (out_format == OUTPUT_FORMAT_YM2608) {

    // init YM2608 adpcm write handle
    if (ym2608_encode_init(&nas, fp_out, use_high_memory) != 0) {
      printf("error: out of memory (YM2608 adpcm encode handle init error).\n");
      goto catch;
    }

    // init mp3 decoder handle
    if (mp3_init(&mp3, NULL, NULL, &nas, out_volume, use_high_memory) != 0) {
      printf("error: out of memory (mp3 decode handle init error).\n");
      goto catch;
    }

  }

  // decode mp3
  rc = mp3_decode(&mp3, mp3_buffer, mp3_len);

  // close out file
  fclose(fp_out);
  fp_out = NULL;

  if (rc == 0) {
    // rename staging file to final output name
    if (out_file_name[0] == '\0') {
      strcpy(out_file_name, out_staging_file_name);
      out_file_name[ strlen(out_staging_file_name) - 3 ] = '\0';
      if (out_format == OUTPUT_FORMAT_ADPCM) {
        if (mp3_file_name[ strlen(mp3_file_name) - 3 ] == 'm') {
          strcat(out_file_name, "pcm");
        } else {
          strcat(out_file_name, "PCM");
        }
      } else if (out_format == OUTPUT_FORMAT_PCM) {
        if (mp3_file_name[ strlen(mp3_file_name) - 3 ] == 'm') {
          strcat(out_file_name, mp3.mp3_channels == 2 ? "s" : "m");
        } else {
          strcat(out_file_name, mp3.mp3_channels == 2 ? "S" : "M");
        }
        strcat(out_file_name, mp3.mp3_sample_rate == 32000 ? "32" :
                              mp3.mp3_sample_rate == 44100 ? "44" :
                              mp3.mp3_sample_rate == 48000 ? "48" : "xx");        
      } else if (out_format == OUTPUT_FORMAT_YM2608) {
        if (mp3_file_name[ strlen(mp3_file_name) - 3 ] == 'm') {
          strcat(out_file_name, mp3.mp3_channels == 2 ? "a" : "n");
        } else {
          strcat(out_file_name, mp3.mp3_channels == 2 ? "A" : "N");
        }
        strcat(out_file_name, mp3.mp3_sample_rate == 32000 ? "32" :
                              mp3.mp3_sample_rate == 44100 ? "44" :
                              mp3.mp3_sample_rate == 48000 ? "48" : "xx");        
      }
    }
    printf("\n");
    if (FILES(&filbuf, out_file_name, 0x20) == 0) {
      printf("warning: output file (%s) already exists. overwrite? (y/n)", out_file_name);
      uint8_t c;
      do {
        c = INKEY();
        if (c == 'n' || c == 'N') {
          printf("\ncanceled.\n");
          goto catch;
        }
      } while (c != 'y' && c != 'Y');
      DELETE(out_file_name);
    }
    printf("\n");
    if (RENAME(out_staging_file_name, out_file_name) < 0) {
      printf("error: output file rename error.\n");
      goto catch;
    }
    printf("Output file name: %s\n", out_file_name);
    rc = 0;
  }

catch:

  // close mp3 decoder handle
  mp3_close(&mp3);

  if (out_format == OUTPUT_FORMAT_ADPCM) {
    // close adpcm encode handle
    adpcm_encode_close(&adpcm);
  } else if (out_format == OUTPUT_FORMAT_PCM) {
    // close pcm encode handle
    pcm_encode_close(&pcm);
  } else if (out_format == OUTPUT_FORMAT_YM2608) {
    // close ym2608 adpcm encode handle
    ym2608_encode_close(&nas);
  }

  // close mp3 file if still opened
  if (fp_mp3 != NULL) {
    fclose(fp_mp3);
    fp_mp3 = NULL;
  }

  // close out file if still opened
  if (fp_out != NULL) {
    fclose(fp_out);
    fp_out = NULL;
  }

  // reclaim mp3 staging buffer
  if (mp3_staging_buffer != NULL) {
    himem_free(mp3_staging_buffer, 0);    // must be main memory
    mp3_staging_buffer = NULL;
  }

  // reclaim mp3 buffer high memory
  if (mp3_buffer != NULL) {
    himem_free(mp3_buffer, use_high_memory);
    mp3_buffer = NULL;
  }

exit:
  // flush key buffer
  while (B_KEYSNS() != 0) {
    B_KEYINP();
  }

  return rc;
}