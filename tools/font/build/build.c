
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#ifdef BUILD2
#include "u8g2.h"
#endif

extern const uint8_t *u8x8_font_list[] ;
extern char *u8x8_font_names[] ;
extern const uint8_t *u8g2_font_list[] ;
extern char *u8g2_font_names[] ;

#ifdef BUILD2
extern void u8g2_Setup_TGA(u8g2_t *u8g2, const u8g2_cb_t *u8g2_cb);
extern void tga_save(const char *name);
#endif


/*===================================*/

int add_to_str(char **dest, const char *s)
{
  void *t;
  
  if ( *dest == NULL )
  {
    *dest = strdup(s);
    if ( *dest == NULL )
      return 0;
  }
  else
  {
    t = realloc(*dest, strlen(*dest) + strlen(s) + 1);
    if ( t == NULL )
      return 0;
    *dest = (char *)t;
    strcat(*dest, s);
  }
  
  return 1;
}

/*
  copy file from source_file_name to dest_file_name
*/
int file_copy(const char *source_file_name, const char *dest_file_name)
{
  int ch;
  FILE *source_fp;
  FILE *dest_fp;
  
  source_fp = fopen(source_file_name, "r");
  dest_fp = fopen(dest_file_name, "w");
  
  if ( source_fp == NULL || dest_fp == NULL )
    return 0;
  
  while( ( ch = fgetc(source_fp) ) != EOF )
    fputc(ch, dest_fp);
  
  fclose(source_fp);
  fclose(dest_fp);
  
  return 1;
}


/*
  Insert "text" between lines "start_line" and "end_line" of file "filename"
*/
int insert_into_file(const char *filename, const char *text, const char *start_line, const char *end_line)
{
  static char line[1024*4];
  const char *tmpname = "tmp.h";
  FILE *source_fp;
  FILE *dest_fp;
  
  if ( file_copy(filename, tmpname) == 0 )
    return 0;

  source_fp = fopen(tmpname, "r");
  dest_fp = fopen(filename, "w");

  if ( source_fp == NULL || dest_fp == NULL )
    return 0;
  
  for(;;)
  {
    if ( fgets(line, 1024*4, source_fp) == NULL )
      break;
    if ( strncmp(line, start_line, strlen(start_line)) == 0 )
    {
      fputs(line, dest_fp);
      fputs(text, dest_fp);
      fputs("\n", dest_fp);
      
      for(;;)
      {
	if ( fgets(line, 1024*4, source_fp) == NULL )
	  break;
	if ( strncmp(line, end_line, strlen(end_line)) == 0 )
	{
	  fputs(line, dest_fp);
	  break;
	}
      }
    }
    else
    {
      fputs(line, dest_fp);
    }
  }

  fclose(source_fp);
  fclose(dest_fp);
  
  unlink(tmpname);

  return 1;
}

/*===================================*/


struct groupinfo
{
  char *groupname;
};

struct fontinfo
{
  int is_ttf;		/* 0 = bdf, 1= ttf */
  char *filename;	/* filename including extension */
  char *name;
  int group;		/* group-index */
  int	pt;			/* point size, ttf only */
  int build_mode;	/* Or'd BM_T, BM_H, BM_M, BM_8 */
  int font_mode;	/* Or'd FM_C and FM_8 */
  int map_mode;	/* Or'd MM_F, FM_N and FM_R */
  char *map_custom;	/* e.g. 32,42-58>32 */
  char *map_custom_postfix;
};

typedef void (*cbfn_t)(int i, int fm, char *fms, int bm, char *bms, int mm, char *mms);



struct groupinfo gi[] = {
  { "U8glib" },
  { "X11" },
  { "fontstruct" },
  { "cu12" },
  { "profont" },		/* 4 */
  { "adobe" },
};

#define BM_T	1	/* Transparent = build mode 0 proportional */
#define BM_H	2	/* Common Height = build mode 1 */
#define BM_M	4	/* Monospace = build mode 2 */
#define BM_8	8	/* 8x8 = build mode 3 */
#define FM_C	1	/* u8g2 compressed font */
#define FM_8	2	/* u8x8 uncompressed font */
#define MM_F	1	/* full */
#define MM_R	2	/* reduced */
#define MM_N	4	/* numbers */
#define MM_C	8	/* custom */


struct fontinfo fi[] = {
  { 0, "u8glib_4.bdf", 		"u8glib_4", 		0, 0, BM_T|BM_H, FM_C, MM_F|MM_R, "", "" },
  { 0, "m2icon_5.bdf", 		"m2icon_5", 		0, 0, BM_T, FM_C, MM_F, "", ""},
  { 0, "m2icon_7.bdf", 		"m2icon_7", 		0, 0, BM_T, FM_C, MM_F, "", ""},
  { 0, "m2icon_9.bdf", 		"m2icon_9", 		0, 0, BM_T, FM_C, MM_F, "", ""},
  { 0, "cursor.bdf", 			"cursor", 			1, 0, BM_T, FM_C, MM_C, "0-223>32", "f" },
  { 0, "cursor.bdf", 			"cursor", 			1, 0, BM_T, FM_C, MM_C, "0-80>32", "r" },
  { 0, "micro.bdf", 			"micro", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "4x6.bdf", 			"4x6", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "5x7.bdf", 			"5x7", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "5x7.bdf", 			"5x7", 			1, 0, BM_8, FM_8, MM_F|MM_R|MM_N, "", "" },
  { 0, "5x8.bdf", 			"5x8", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "5x8.bdf", 			"5x8", 			1, 0, BM_8, FM_8, MM_F|MM_R|MM_N, "", "" },
  { 0, "6x10.bdf", 			"6x10", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "6x12.bdf", 			"6x12", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "6x13.bdf", 			"6x13", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "6x13B.bdf", 			"6x13B", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "6x13O.bdf", 			"6x13O", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "7x13.bdf", 			"7x13", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "7x13B.bdf", 			"7x13B", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "7x13O.bdf", 			"7x13O", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "7x14.bdf", 			"7x14", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "7x14B.bdf", 			"7x14B", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "8x13.bdf", 			"8x13", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "8x13B.bdf", 			"8x13B", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "8x13O.bdf", 			"8x13O", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "9x15.bdf", 			"9x15", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "9x15B.bdf", 			"9x15B", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "9x18.bdf", 			"9x18", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "9x18B.bdf", 			"9x18B", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "10x20.bdf", 			"10x20", 			1, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "profont10.bdf", 		"profont10", 		4, 0, BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "profont11.bdf", 		"profont11", 		4, 0, BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "profont12.bdf", 		"profont12", 		4, 0, BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "profont15.bdf", 		"profont15", 		4, 0, BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "profont17.bdf", 		"profont17", 		4, 0, BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "profont22.bdf", 		"profont22", 		4, 0, BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "profont29.bdf", 		"profont29", 		4, 0, BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "amstrad_cpc.bdf", 	"amstrad_cpc", 	2, 0, BM_8, FM_C|FM_8, MM_F|MM_R|MM_N, "" , ""},
  { 0, "cu12.bdf", 			"cu12", 			3, 0, BM_T|BM_H|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },

  { 0, "courB08.bdf", 		"courB08", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "courB10.bdf", 		"courB10", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "courB12.bdf", 		"courB12", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "courB14.bdf", 		"courB14", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "courB18.bdf", 		"courB18", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "courB24.bdf", 		"courB24", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "courR08.bdf", 		"courR08", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "courR10.bdf", 		"courR10", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "courR12.bdf", 		"courR12", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "courR14.bdf", 		"courR14", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "courR18.bdf", 		"courR18", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "courR24.bdf", 		"courR24", 		5, 0, BM_T|BM_M, FM_C, MM_F|MM_R|MM_N, "", "" },

  { 0, "helvB08.bdf", 		"helvB08", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "helvB10.bdf", 		"helvB10", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "helvB12.bdf", 		"helvB12", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "helvB14.bdf", 		"helvB14", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "helvB18.bdf", 		"helvB18", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "helvB24.bdf", 		"helvB24", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "helvR08.bdf", 		"helvR08", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "helvR10.bdf", 		"helvR10", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "helvR12.bdf", 		"helvR12", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "helvR14.bdf", 		"helvR14", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "helvR18.bdf", 		"helvR18", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },
  { 0, "helvR24.bdf", 		"helvR24", 		5, 0, BM_T|BM_H, FM_C, MM_F|MM_R|MM_N, "", "" },

  
};




char *bdf_path = "../bdf/";
char *bdfconv_path = "../bdfconv/bdfconv";
FILE *u8g2_font_list_fp;
FILE *u8x8_font_list_fp;
char *u8g2_prototypes = NULL;
char *u8x8_prototypes = NULL;
char *u8g2_fonts_filename = "../../../csrc/u8g2_fonts.c";
char *u8x8_fonts_filename = "../../../csrc/u8x8_fonts.c";

char target_font_identifier[1024];
char bdf_cmd[2048];
char font_prototype[2048];
char tga_filename[2048];
char convert_cmd[2048];

#ifdef BUILD2

u8g2_t u8g2;
int u8g2_fnt_cnt = 0;

void overview_draw_line(int i, uint16_t encoding_start, uint16_t x, uint16_t y, uint16_t w)
{
  int j;

  u8g2_SetFont(&u8g2, u8g2_font_list[u8g2_fnt_cnt]);
  u8g2_SetFontDirection(&u8g2, 0);
  u8g2_DrawString(&u8g2, 0, 40, u8g2_font_names[u8g2_fnt_cnt]);
  for( j = 0; j < 16; j++)
  {
    if ( u8g2_IsGlyph(&u8g2, encoding_start + j) != 0 )
    {
      u8g2_DrawGlyph(&u8g2, x+j*w, y, encoding_start + j);
    }
  }
}

int is_overview_line_empty(uint16_t encoding_start)
{
  int j;
  for( j = 0; j < 16; j++)
  {
    if ( u8g2_IsGlyph(&u8g2, encoding_start + j) != 0 )
    {
      return 0;
    }
  }
  return 1;
}

void overview_draw_table(int i, uint16_t x, uint16_t y)
{
  int cw, ch;
  int line;
  uint16_t encoding;
  
  u8g2_SetFont(&u8g2, u8g2_font_7x13_tr);
  u8g2_DrawString(&u8g2, 0, 20, u8g2_font_names[u8g2_fnt_cnt]);


  u8g2_SetFont(&u8g2, u8g2_font_list[u8g2_fnt_cnt]);
  u8g2_SetFontDirection(&u8g2, 0);

  ch = u8g2_GetMaxCharHeight(&u8g2);
  cw = u8g2_GetMaxCharWidth(&u8g2);
  
  line = 0;
  for(;;)
  {
    encoding = line*16;
    if ( is_overview_line_empty(encoding) == 0 )
    {
      overview_draw_line(i, encoding, x, y, cw);
      y += ch;
    }
    line++;
    if ( line > 16 )
      break;
  }
}

void overviewpic(int i, int fm, char *fms, int bm, char *bms, int mm, char *mms)
{
  int cw, ch;
  //if ( fm == FM_8 ) 
  if ( fm == FM_C ) 
  {
    printf("overview picture %s\n", target_font_identifier);
    u8g2_Setup_TGA(&u8g2, &u8g2_cb_r0);
    u8x8_display_Init(u8g2_GetU8x8(&u8g2));
    u8x8_display_SetPowerSave(u8g2_GetU8x8(&u8g2), 0);  
    //u8g2_SetFont(&u8g2, u8g2_font_helvB14_tr);

    
    u8g2_FirstPage(&u8g2);
    do
    {
      u8g2_SetFont(&u8g2, u8g2_font_list[u8g2_fnt_cnt]);
      u8g2_SetFontDirection(&u8g2, 0);
      u8g2_DrawString(&u8g2, 0, 40, u8g2_font_names[u8g2_fnt_cnt]);
      
      ch = u8g2_GetMaxCharHeight(&u8g2);
      cw = u8g2_GetMaxCharWidth(&u8g2);

      overview_draw_table(i, 0, 60);

      //overviewline(i, '@', 0, 80, cw);      
    } while( u8g2_NextPage(&u8g2) );

    strcpy(tga_filename, target_font_identifier);
    strcat(tga_filename, ".tga");
    
    tga_save("font.tga");
    
    sprintf(convert_cmd, "convert font.tga -trim %s.png", target_font_identifier );
    system(convert_cmd);

    u8g2_fnt_cnt++;
  }
}

#endif

void bdfconv(int i, int fm, char *fms, int bm, char *bms, int mm, char *mms)
{
  strcpy(bdf_cmd, bdfconv_path);
  strcat(bdf_cmd, " ");

  if ( fm == FM_C ) strcat(bdf_cmd, " -f 1");
  if ( fm == FM_8 ) strcat(bdf_cmd, " -f 2");
  
  if ( bm == BM_T ) strcat(bdf_cmd, " -b 0");
  if ( bm == BM_H ) strcat(bdf_cmd, " -b 1");
  if ( bm == BM_M ) strcat(bdf_cmd, " -b 2");
  if ( bm == BM_8 ) strcat(bdf_cmd, " -b 3");
  
  if ( mm == MM_F ) strcat(bdf_cmd, " -m '32-255>32'");
  if ( mm == MM_R ) strcat(bdf_cmd, " -m '32-127>32'");
  if ( mm == MM_N ) strcat(bdf_cmd, " -m '32,42-58>32'");
  if ( mm == MM_C ) 
  {
    strcat(bdf_cmd, " -m '");
    strcat(bdf_cmd, fi[i].map_custom);
    strcat(bdf_cmd, "'");
  }

  strcat(bdf_cmd, " ");
  strcat(bdf_cmd, bdf_path);
  strcat(bdf_cmd, fi[i].filename);


  strcat(bdf_cmd, " -n ");
  strcat(bdf_cmd, target_font_identifier);

  strcat(bdf_cmd, " -o ");
  //strcat(bdf_cmd, target_font_identifier);
  strcat(bdf_cmd, "font");
  strcat(bdf_cmd, ".c");
  
/*
    fprintf(out_fp, "const uint8_t %s[%d] U8X8_FONT_SECTION(\"%s\") \n", fontname, bf->target_cnt, fontname);
    fprintf(out_fp, "const uint8_t %s[%d] U8G2_FONT_SECTION(\"%s\") \n", fontname, bf->target_cnt, fontname);

*/
  strcpy(font_prototype, "extern const uint8_t ");
  strcat(font_prototype, target_font_identifier);
  strcat(font_prototype, "[]");

  if ( fm == FM_8 ) 
  {
    strcat(bdf_cmd, " && cat font.c >>");
    strcat(bdf_cmd, u8x8_fonts_filename);
    strcat(font_prototype, " U8X8_FONT_SECTION(\"");    
    strcat(font_prototype, target_font_identifier);
    strcat(font_prototype, "\");\n");
    add_to_str(&u8x8_prototypes, font_prototype);
  }
  else
  {    
    strcat(bdf_cmd, " && cat font.c >>");
    strcat(bdf_cmd, u8g2_fonts_filename);
    strcat(font_prototype, " U8G2_FONT_SECTION(\"");    
    strcat(font_prototype, target_font_identifier);
    strcat(font_prototype, "\");\n");
    add_to_str(&u8g2_prototypes, font_prototype);
  }

  printf("%s\n", bdf_cmd);
  system(bdf_cmd);
  
}

void fontlist_identifier(int i, int fm, char *fms, int bm, char *bms, int mm, char *mms)
{
  if ( fm == FM_C ) 
  {
    fprintf(u8g2_font_list_fp, "  %s,\n", target_font_identifier);
  }
  if ( fm == FM_8 ) 
  {
    fprintf(u8x8_font_list_fp, "  %s,\n", target_font_identifier);
  }
  
}

void fontlist_name(int i, int fm, char *fms, int bm, char *bms, int mm, char *mms)
{
  if ( fm == FM_C ) 
  {
    fprintf(u8g2_font_list_fp, "  \"%s\",\n", target_font_identifier);
  }
  if ( fm == FM_8 ) 
  {
    fprintf(u8x8_font_list_fp, "  \"%s\",\n", target_font_identifier);
  }
  
}

void gen_font(int i, int fm, char *fms, int bm, char *bms, int mm, char *mms, cbfn_t cb )
{
  
  if ( fm == FM_8 )
    if ( bm != BM_8 )
      return;
  
  strcpy(target_font_identifier, fms);
  strcat(target_font_identifier, "_font_");
  strcat(target_font_identifier, fi[i].name);
  strcat(target_font_identifier, "_");
  strcat(target_font_identifier, bms);
  strcat(target_font_identifier, mms);
    
  cb(i, fm,fms,bm,bms,mm,mms);
}

void map_font(int i, int fm, char *fms, int bm, char *bms, cbfn_t cb)
{
  if ( (fi[i].map_mode & MM_F) != 0 )
    gen_font(i, fm, fms, bm, bms, MM_F, "f", cb);
  if ( (fi[i].map_mode & MM_R) != 0 )
    gen_font(i, fm, fms, bm, bms, MM_R, "r", cb);
  if ( (fi[i].map_mode & MM_N) != 0 )
    gen_font(i, fm, fms, bm, bms, MM_N, "n", cb);
  if ( (fi[i].map_mode & MM_C) != 0 )
    gen_font(i, fm, fms, bm, bms, MM_C, fi[i].map_custom_postfix, cb);
}

void build_font(int i, int fm, char *fms, cbfn_t cb)
{
  if ( (fi[i].build_mode & BM_T) != 0 )
    map_font(i, fm, fms, BM_T, "t", cb);
  if ( (fi[i].build_mode & BM_H) != 0 )
    map_font(i, fm, fms, BM_H, "h", cb);
  if ( (fi[i].build_mode & BM_M) != 0 )
    map_font(i, fm, fms, BM_M, "m", cb);
  if ( (fi[i].build_mode & BM_8) != 0 )
  {
    if ( fm == FM_8 ) 
      map_font(i, fm, fms, BM_8, "", cb);
    else
      map_font(i, fm, fms, BM_8, "8", cb);
  }
}

void process_font(int i, cbfn_t cb)
{
  FILE *fp;
  static char s[1024];
  
  strcpy(s,bdf_path);
  strcat(s, fi[i].filename);
  fp = fopen(s, "r");
  if ( fp == NULL )
  {
    printf("font %s missing\n",s );
    return;
  }
  fclose(fp);
  
  if ( (fi[i].font_mode & FM_C) != 0 )
    build_font(i, FM_C, "u8g2", cb);
  if ( (fi[i].font_mode & FM_8) != 0 )
    build_font(i, FM_8, "u8x8", cb);
}

void do_font_loop(cbfn_t cb)
{
  int i, cnt;
  cnt = sizeof(fi)/sizeof(*fi);
  for( i = 0; i < cnt; i++ )
  {
    process_font(i, cb);
  }
}

int main(void)
{
  //unlink(u8x8_fonts_filename);
  //unlink(u8g2_fonts_filename);

#ifndef BUILD2
  
  if ( file_copy("u8x8_fonts.pre", u8x8_fonts_filename) == 0 )
    return 0;
  if ( file_copy("u8g2_fonts.pre", u8g2_fonts_filename) == 0 )
    return 0;
  
  
  do_font_loop(bdfconv);
  
  u8g2_font_list_fp = fopen("u8g2_font_list.c", "w");
  u8x8_font_list_fp  = fopen("u8x8_font_list.c", "w");
  fprintf(u8g2_font_list_fp, "/* u8g2_font_list.c */\n");
  fprintf(u8x8_font_list_fp, "/* u8x8_font_list.c */\n");
  fprintf(u8g2_font_list_fp, "#include \"u8g2.h\"\n");
  fprintf(u8x8_font_list_fp, "#include \"u8g2.h\"\n");
  fprintf(u8g2_font_list_fp, "const uint8_t *u8g2_font_list[] = {\n");
  fprintf(u8x8_font_list_fp, "const uint8_t *u8x8_font_list[] = {\n");
  do_font_loop(fontlist_identifier);
  fprintf(u8g2_font_list_fp, "  NULL\n};\n");
  fprintf(u8x8_font_list_fp, "  NULL\n};\n");

  fprintf(u8g2_font_list_fp, "char *u8g2_font_names[] = {\n");
  fprintf(u8x8_font_list_fp, "char *u8x8_font_names[] = {\n");
  do_font_loop(fontlist_name);
  fprintf(u8g2_font_list_fp, "  NULL\n};\n");
  fprintf(u8x8_font_list_fp, "  NULL\n};\n");
  
  fclose(u8g2_font_list_fp);
  fclose(u8x8_font_list_fp);

  printf("update u8g2.h\n");
  insert_into_file("../../../csrc/u8g2.h", u8g2_prototypes, "/* start font list */", "/* end font list */");
  printf("update u8x8.h\n");
  insert_into_file("../../../csrc/u8x8.h", u8x8_prototypes, "/* start font list */", "/* end font list */");

  unlink("font.c");
#endif

#ifdef BUILD2
  u8g2_fnt_cnt = 0;
  do_font_loop(overviewpic);
#endif

  return 0;
}
