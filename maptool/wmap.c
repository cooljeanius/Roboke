#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_BSD
#include <bsd/stdio.h>
#define READALINE line = fgetln(inf, &len);
#else
#define READALINE line = fgets(buffer, 8192, inf); if (line) {len=strlen(line);} else {len=0;}
#endif

typedef struct {
	int w;
	int h;
	char ***d;
} WMAP;

#define OMF(X,Y,TYPE) strcpy(out.d[(Y)][(X)], TYPE)
#define OMFX(X,Y,TYPE) strcpy(out.d[(Y)+70-26][(X)+46-28], TYPE)

void freemap (WMAP *map) {
	int x,y;
	for (y=0;y<map->h;++y) {
		for (x=0;x<map->w;++x) {
			free(map->d[y][x]);
		}
		free(map->d[y]);
	}
	free(map->d);
	map->w=0; map->h=0;
}

void allocmap (WMAP *map, int w, int h) {
	map->w=w; map->h=h;
	int x,y;
	map->d= (char***) calloc(map->h, sizeof (char*));
	for (y=0;y<map->h;++y) {
		map->d[y]=(char**) calloc(map->w, sizeof (char*));
		for (x=0;x<map->w;++x) {
			map->d[y][x]=(char*) calloc(13, sizeof (char*));
		}
	}
}

void initmap (WMAP *map, const char *terrain) {
	int x,y;
	for (y=0;y<map->h;++y) {
		for (x=0;x<map->w;++x) {
			snprintf(map->d[y][x], 13, "%-12s", terrain);
		}
	}
}

void fillarea (WMAP *map, int x0, int x1, int y0, int y1, const char *terrain) {
	int x,y;
	for (y=y0;y<map->h && y<=y1;++y) {
		for (x=x0;x<map->w && x<=x1;++x) {
			snprintf(map->d[y][x], 13, "%12s", terrain);
		}
	}
}

void fillareaX (WMAP *map, int x0, int x1, int y0, int y1, const char *terrain) {
	fillarea(map, x0+46-28, x1+46-28, y0+70-26, y1+70-26, terrain);
}

/**
 * map I/O
 */
void writemap (FILE *outf, WMAP *map) {
	int x,y;
	if (1) fprintf(stderr, "INFO: write map w:%i h:%i\n", map->w, map->h);
	fprintf(outf, "border_size=1\nusage=map\n\n");
	for (y=0;y<map->h;++y) {
		for (x=0;x<map->w;++x) {
			if (x!=0) fprintf(outf, ", ");
			fprintf(outf, "%-12s",map->d[y][x]);
		}
		fprintf(outf, "\n");
	}
}

int readmap (FILE *inf, WMAP *map) {
	rewind(inf);
	char *line = NULL;
	char buffer[8192] = "";
	char *tmp;
	size_t len;
	off_t pos;
	while (!feof(inf) && (!line || line[0]!='\n')) {
		READALINE // header
	}
	if (feof(inf)) {
		// error parsing header
		return -1;
	}
	// remember this file offset.
	pos = ftell(inf);
	READALINE // first map line
	if ( len < 2) {
		return -2;
	}
	map->w = 1;
	tmp=line;
	while(tmp && *tmp && (tmp=strchr(tmp, ','))) {
		map->w++;
		tmp++;
	}

	map->h=0;
	while (!feof(inf) && line && line[0]!='\0' && line [0]!='\n') {
		READALINE
		map->h++;
	}

	if (0) fprintf(stderr, "INFO: read map w:%i h:%i\n", map->w, map->h);

	map->d= (char***) calloc(map->h, sizeof (char*));
	fseek(inf, pos, SEEK_SET);
	int x,y;
	for (y=0;y<map->h;++y) {
		map->d[y]=(char**) calloc(map->w, sizeof (char*));
		READALINE
		tmp = line;
		int len;
		for (x=0; x < map->w; ++x) {
			if (!line) {
				fprintf(stderr, "invalid end of line\n");
				return -3;
			}
			tmp = strchr(line, ',');
			if (tmp) {len = tmp - line;}
			else len = strlen(line) - 1;
			map->d[y][x]=(char*) calloc(13, sizeof (char*));
			strncpy(map->d[y][x], line, len);
			if (tmp) {
				line = tmp + 2;
			} else {
				line = NULL;
			}
		}
	}
	while (!feof(inf)) {
		READALINE
		if (line) fprintf(stderr, "WARNING: excess line: '%s'\n", line);
	}
	return (0);
}

/**
 * map modify fns
 */
int copymap (WMAP *dst, WMAP *src, int xoff, int yoff) {
	int x,y;
	/* destination needs to be at least as large as src */
	if ( (src->w+xoff > dst->w) || (src->h+yoff > dst->h)) {
		return -1;
	}
	if ( (dst->w+xoff < 0) || (dst->h+yoff < 0)) {
		return -2;
	}
	for (y=0;y<src->h;++y) {
		for (x=0;x<src->w;++x) {
			strncpy(dst->d[y+yoff][x+xoff], src->d[y][x], 13);
		}
	}
	return 0;
}

void rotatemap(WMAP *m, int ang) {
	WMAP tmp;
	int x,y,w,h;
	allocmap(&tmp, m->w, m->h);
	copymap(&tmp, m, 0, 0);

	switch (ang) {
		case -2: /* mirror Y */
		case -1: /* mirrot X */
		case 180:
			w=tmp.w; h=tmp.h;
			break;
		case 90:
		case 270:
			w=tmp.h; h=tmp.w;
			freemap(m);
			allocmap(m, w, h);
			break;
		default:
			freemap(&tmp);
			return;
			break;
	}

	for (y=0;y<h;++y) {
		for (x=0;x<w;++x) {
			if      (ang==180) strncpy(m->d[y][x], tmp.d[(h-1)-y][(w-1)-x], 13); // 180
			else if (ang== -1) strncpy(m->d[y][x], tmp.d[      y][(w-1)-x], 13); // mirror X
			else if (ang== -2) strncpy(m->d[y][x], tmp.d[(h-1)-y][      x], 13); // mirror Y
		/*else if (ang==  0) strncpy(m->d[y][x], tmp.d[      y][      x], 13); // NOOP - copy */

		/*else if (ang==XXX) strncpy(m->d[y][x], tmp.d[(w-1)-x][(h-1)-y], 13); // 270 + mirror Y */
			else if (ang==270) strncpy(m->d[y][x], tmp.d[      x][(h-1)-y], 13); // 270
			else if (ang== 90) strncpy(m->d[y][x], tmp.d[(w-1)-x][      y], 13); // 90
		/*else if (ang==XXX) strncpy(m->d[y][x], tmp.d[      x][      y], 13); // 90  + mirror X */
		}
	}
	freemap(&tmp);
}

int copymaparea (WMAP *dst, WMAP *src, int w, int h, int dx, int dy, int sx, int sy, char *transparent) {
	int x,y;
	for (y=0;y<h;++y) {
		if ( (y+sy < 0 || y+sy >= src->h) || (y+dy < 0 || y+dy >= dst->h) )
			continue;
		for (x=0;x<w;++x) {
			if ( (x+sx < 0 || x+sx >= src->w) || (x+dx < 0 || x+dx >= dst->w) )
				continue;
			if (transparent && strncmp(transparent, src->d[y+sy][x+sx], strlen(transparent)) == 0)
				continue;
			if (transparent && transparent[0] == '!' && strncmp(&transparent[1], src->d[y+sy][x+sx], strlen(transparent)-1) != 0)
				continue;
			strncpy(dst->d[y+dy][x+dx], src->d[y+sy][x+sx], 13);
		}
	}
	return 0;
}


/**
 * high level fn
 */
int read_mapfile (const char *fn, WMAP *dst) {
	int err =0;
	FILE *f = fopen(fn, "r");
	if (!f) {
		fprintf(stderr, "ERROR: opening file: '%s.\n", fn);
		return -1;
	}
	if ((err=readmap(f, dst))) {
		fprintf(stderr, "ERROR: %i reading map: '%s'\n",err, fn);
	}
	fclose(f);
	return err;
}

int overlay_mapfile (const char *fn, WMAP *dst, int xoff, int yoff) {
	WMAP map;
	int err =0;
	map.w=map.h=0;
	FILE *f = fopen(fn, "r");
	if (!f) {
		fprintf(stderr, "ERROR: opening file: '%s.\n", fn);
		return -1;
	}
	if ((err=readmap(f, &map))) {
		fprintf(stderr, "ERROR: %i reading map: '%s'\n",err, fn);
		fclose(f);
		return -2;
	}
	fclose(f);
	if (xoff%2) {
		fprintf(stderr, "WARNING: map-align mismatch file: '%s.\n", fn);
	}
	if ((err=copymap(dst, &map, xoff, yoff))) {
		fprintf(stderr, "ERROR: %i copying map: '%s'\n",err, fn);
	}
	freemap(&map);
	return err;
}
#ifdef __APPLE__XXX
#define MAPROOT "/Users/rgareus/Library/Application Support/Wesnoth_1.8/data/add-ons/Roboke/maps/"
#else
#define MAPROOT "./"
#endif


void dothecave (WMAP *out, int x0, int x1, int y0, int y1) {
	WMAP src;
	allocmap(out, x1-x0, y1-y0);
	read_mapfile(MAPROOT "island.map", &src);
	copymaparea(out, &src, src.w, src.h, 0, 0, x0, y0, NULL);
	freemap(&src);
	read_mapfile(MAPROOT "cave.map", &src);
	int cxo, cxd;
	int cyo, cyd;
	cxo = cxd = cyo = cyd = 0;
	if (x0 > 28) cxd = x0 - 28; else cxo = 28 - x0;
	if (y0 > 26) cyd = y0 - 26; else cyo = 26 - y0;
	copymaparea(out, &src, src.w, src.h, cxo ,cyo, cxd, cyd, "_off^_usr");
	freemap(&src);
}

int main (int argc, char**argv) {
	WMAP out;
	WMAP source;
#if 0
	allocmap(&out, 160, 100);
	initmap(&out, "_f");
#endif

	int area = -1;
	if (argc > 1) area = atoi(argv[1]);

	if (area==-1) {
		// copy cave-part onto island map.
		int x0=0; int y0=0;
		int x1=160; int y1=100;
		dothecave(&out, x0, x1, y0, y1);

		OMF(67,39, "1 Khw"); // north crater cave keep
		OMF(68,38, "2 Chw");
	} else if (area==-2) {
		read_mapfile(MAPROOT "ship.map", &out);
		strcpy(out.d[31][12], "1 Ket");
		strcpy(out.d[26][23], "2 Wo");
	} else if (area==-3) {
		allocmap(&out, 33, 43);
		initmap(&out, "Wo");
		read_mapfile(MAPROOT "ship.map", &source);
		copymaparea(&out, &source, source.w, source.h, 4, 1, 0, 0, NULL);
		strcpy(out.d[31][16], "1 Ket");
		strcpy(out.d[1][1],   "2 Wo");
		strcpy(out.d[40][1],  "3 Wo");
	} else if (area==-4) {
		read_mapfile(MAPROOT "riddle.map", &out);
		OMF( 1,10, "1 Wwt");
	} else if (area==-5) {
		read_mapfile(MAPROOT "testing.map", &out);
#if 0
	} else if (area==-99) {
		read_mapfile(MAPROOT "island.map", &source);
		read_mapfile(MAPROOT "cave.map", &out);
		copymaparea(&out, &source, 18, 26, 0, 0, 28, 26, "!M");
		copymaparea(&out, &source, 24,  9, 0, 0, 28, 26, "!M");
		copymaparea(&out, &source, 44,  4, 0, 0, 28, 26, "!M");
#endif
	} else {
		// extract part from the main map
		read_mapfile(MAPROOT "island.map", &source);
	}

	if (area==0) {
		/* East: */
		allocmap(&out, 32, 42);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 120, 32, NULL);
		strcpy(out.d[5][9], "2 Khw");
		strcpy(out.d[35][23], "1 Ke");
		strcpy(out.d[36][12], "3 Ss^Vm");
	} else if (area==1) {
		/* NorthWestWest: Land */
		allocmap(&out, 42, 42);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 10, 12, NULL);
		strcpy(out.d[3][36], "1 Kh");
		strcpy(out.d[36][10], "2 Kh");
		strcpy(out.d[12][25], "3 Kh");
		strcpy(out.d[19][19], "4 Kh");
	} else if (area==2) {
		/* North shore: */
		allocmap(&out, 48, 40);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 64, 7, NULL);
		strcpy(out.d[34][42], "1 Kh");
		strcpy(out.d[6][16], "2 Kh");
		strcpy(out.d[10][3], "3 Kh");
		strcpy(out.d[1][46], "4 Ww");
	} else if (area==3) {
		/* Center  -- unused */
		allocmap(&out, 56, 45);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 50, 40, NULL);
	} else if (area==4) {
		/* south east */
		int xoff=114;
		int yoff=19;
		int wcrp=4;
		int hcrp=15;
		allocmap(&out, 160 - xoff - wcrp , 100 - yoff - hcrp);
		copymaparea(&out, &source, source.w, source.h, 0, 0, xoff, yoff, NULL);
		OMF(32,28, "1 Ko");
		OMF(14,63, "2 Khs");
		OMF(15,18, "3 Khw");
		//OMF(22,21, "4 Khw"); // sunken in sea
		OMF(29,48, "Ce^Dr");  // original player's keep
		OMF(30,48, "Ss^Vhs");
		OMF(30,47, "Ss");

		OMF(29,50, "Ss");

		OMF(34,49, "Wwr");
		OMF(35,49, "Wwr");
		OMF(31,48, "Ss");
		OMF(31,49, "Ss");

		OMF(32,40, "Ss");
		OMF(32,41, "Ss");

	} else if (area==5) {
		/* South West */
		allocmap(&out, 60, 100-44);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 0, 44, NULL);
		strcpy(out.d[0][46], "Hh^Ft"); // hide entry house
		strcpy(out.d[26][31], "1 Kd");
		strcpy(out.d[4][20], "2 Kh");
		strcpy(out.d[42][21], "3 Khw");
		strcpy(out.d[38][30], "4 Kh");
		strcpy(out.d[49][38], "5 Kh");
	} else if (area==6) {
		/* UNDERGROUND BASE -- unused*/
		allocmap(&out, 90, 50);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 34, 30, NULL);
	} else if (area==7) {
		allocmap(&out, source.w, source.h);
		copymap(&out, &source, 0, 0);
		strcpy(out.d[65][69], "1 Khr");
		strcpy(out.d[60][113], "2 Kh");

	} else if (area==8) {
		/* North East (sc 03)*/
		allocmap(&out, 140-96, 17);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 96, 30, NULL);
		strcpy(out.d[37-30][129-96], "1 Khw");
		strcpy(out.d[41-30][106-96], "2 Kh");
		strcpy(out.d[2][12], "Gg");
	} else if (area==9) {
		/* North West (sc 05)*/
		allocmap(&out, 92-44, 21-4);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 44, 4, NULL);
		strcpy(out.d[17-4][73-44], "1 Ch");
		strcpy(out.d[17-4][67-44], "2 Kh");
		strcpy(out.d[ 8-4][67-44], "3 Kh");
		strcpy(out.d[14-4][81-44], "4 Kh");
	} else if (area==10) {
		/* South Shore */
		allocmap(&out, 110-24, 100-75);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 24, 75, NULL);
		strcpy(out.d[7][6], "1 Kh");
		strcpy(out.d[18][14], "2 Kh");

		strcpy(out.d[16][65], "Mm^Cov");
		strcpy(out.d[15][66], "3 Mm^Kov");
		strcpy(out.d[14][66], "Mm^Cov");

		strcpy(out.d[8][55], "4 Mm^Kov");
		strcpy(out.d[9][55], "Mm^Cov");

		strcpy(out.d[4][63], "5 Ww^Kov"); // CHs^Kov
		strcpy(out.d[5][63], "Wo^Cov");
		strcpy(out.d[4][64], "Wo^Cov");

		strcpy(out.d[22][61], "6 Wot^Kov"); // is 'Wwf^Es'
		strcpy(out.d[23][61], "Wo^Cov");
		strcpy(out.d[22][60], "Wo^Cov");
		strcpy(out.d[22][62], "Wo^Cov");
	} else if (area==14) {
		/* soutern cliffs part II - secret levers*/
		allocmap(&out, 70, 28);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 46, 70, NULL);
		strcpy(out.d[10][63], "1 Khw");

		OMF(43,21, "Mm^Cov");
		OMF(44,20, "2 Mm^Kov");
		OMF(44,19, "Mm^Cov");

		OMF(41,9,  "3 Ww^Kov"); // CHs^Kov
		OMF(41,10, "Wo^Cov");
		OMF(42,9,  "Wo^Cov");

		fillarea(&out, 16,39, 0,6, "Mm^Xm");
		fillarea(&out, 15,38, 0,1, "_s");
		fillarea(&out, 16,37, 0,3, "_s");
		fillarea(&out, 17,35, 0,4, "_s");
		fillarea(&out, 18,33, 0,5, "_s");
		OMF(27,6, "_s");
	} else if (area==11) {
		/* soutern caves */
		int x0= 46; int y0=70;
		int x1=116; int y1=98;
		dothecave(&out, x0, x1, y0, y1);

		fillarea(&out, 15,33, 0,5, "Xu");
		fillarea(&out, 17,33, 5,7, "Xu");
		fillarea(&out, 37,42, 0,3, "Xu");
		fillarea(&out, 34,36, 0,2, "Xu");
		fillarea(&out, 39,41, 4,4, "Xu");
		fillarea(&out, 21,25, 8,8, "Xu");
		OMF(43,3, "Xu");
		OMF(34,3, "Xu");

		OMF(33,6, "Xu");
		OMF(33,7, "Xu");
		OMF(32,7, "Xu");
		OMF(31,8, "Xu");

		OMF(18,7, "Xu");
		OMF(15,8, "Xue");
		OMF(16,7, "Xue");
		OMF(17,8, "Xue");
		OMF( 4,5, "Xue");

		fillarea(&out,  2,5, 0,2, "Xu");
		fillarea(&out,  6,7, 0,1, "Xu");
		// north-east connection
		fillarea(&out, 49,53, 0,4, "Xu");
		fillarea(&out, 54,58, 0,2, "Xu");
		fillarea(&out, 59,60, 0,0, "Xu");
		OMF(57,3, "Xu");
		OMF(59,1, "Xu");

		strcpy(out.d[10][63], "1 Khw");
		OMF(43,21, "Mm^Cov");
		OMF(44,20, "2 Mm^Kov");
		OMF(44,19, "Mm^Cov");

		OMF(41,9,  "3 Ww^Kov"); // CHs^Kov
		OMF(41,10, "Wo^Cov");
		OMF(42,9,  "Wo^Cov");

	} else if (area==12) {
		/* North West End */
		allocmap(&out, 34, 50);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 0, 0, NULL);
		strcpy(out.d[48][20], "1 Kh");
		strcpy(out.d[20][6], "2 Khw");
		strcpy(out.d[31][29], "3 Kh");
	} else if (area==13) {
		/* North shore - caves */
		allocmap(&out, 48, 40);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 64, 7, NULL);
		freemap(&source);
		read_mapfile(MAPROOT "cave.map", &source);
		copymaparea(&out, &source, source.w, source.h, 19,  0 + 26 - 7, 19 + 64 - 28,  0, "_off^_usr");
		copymaparea(&out, &source, source.w, 7       , 16, 10 + 26 - 7, 16 + 64 - 28, 10, "_off^_usr");

		strcpy(out.d[34][42], "1 Kh");
		strcpy(out.d[7][17], "2 Kh");
		strcpy(out.d[10][3], "3 Kh");
	} else if (area==15) {
		/* River Passage */
		allocmap(&out, 34, 38);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 80, 44, NULL);
		freemap(&source);
		// only bottom right keep
		read_mapfile(MAPROOT "cave.map", &source);
		copymaparea(&out, &source, source.w, source.h, 29, 30, 81, 48, "_off^_usr");

		OMF(30,30, "Ss");
		OMF(33,30, "Xu");
		OMF(31,32, "1 Kud");
		OMF(4,6,   "2 Khr");
		OMF(21,11, "3 Kh");
		OMF(12,28, "Ww"); // passage
	} else if (area==16) {
		/* south east caves */
		int x0= 90; int y0=50;
		int x1=130; int y1=81;
		dothecave(&out, x0, x1, y0, y1);

		OMF(23,10, "1 Kh");
		OMF(11, 5, "2 Khw");
		OMF(21,26, "3 Kud");
		OMF(26,27, "5 Kud");
		OMF( 7,24, "4 Kud");

		OMF( 2,21, "Ww^Xo"); // passage bridge
		OMF(19,25, "Wo^Xo"); // nobody leaves, not even skeletons :)
		OMF(20,24, "Wo^Xo");
		//OMF( 7, 5, "Wo^Xo");
		//OMF(13, 2, "Wo^Xo");
		OMF(11,10, "Wo^Bs/"); // scenery :)
	} else if (area==17) {
		/* prison camp */
		allocmap(&out, 48, 30);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 112, 0, NULL);
		OMF(33,19, "1 Ko");
		OMF(37, 7, "2 Khw");
		OMF(44, 6, "Ko^Xo");
	} else if (area==18) {
		/* noth-western island(s) -- unused */
		allocmap(&out, 50, 35);
		copymaparea(&out, &source, source.w, source.h, 0, 0, 0, 0, NULL);
		OMF( 6,20, "1 Khw");
		OMF(35,24, "2 Kh");
		OMF(29,31, "3 Kh");
		OMF(46,15, "4 Kh");
	} else if (area==19) {
		// crater island
		int x0=58; int y0=50;
		int x1=90; int y1=80;
		dothecave(&out, x0, x1, y0, y1);

		OMF(11,26, "1 Kud"); // lair
		OMF(13, 5, "2 Kud"); // dwarven keep
		OMF(11,15, "3 Khw"); // crater
		OMF(11, 9, "4 Ko");  // drake's island keep
		OMF(22,16, "5 Kud"); // skel mid-east
		fillarea(&out, 5,11, 0,1, "Xu");
		OMF(12, 0, "Xu");
		fillarea(&out, 0,1,  0, 2, "Xu");
		fillarea(&out, 0,0,  5,15, "Xu");
		fillarea(&out, 0,2,  6,10, "Xu");
		fillarea(&out, 0,1, 11,13, "Xu");
		fillarea(&out, 7,9,  2, 2, "Xu");
		OMF( 9, 3, "Xu");
		fillarea(&out, 3,3,  8,10, "Xu");
		fillarea(&out, 4,5,  7, 7, "Xu");
		fillarea(&out, 6,6,  5, 6, "Xu");

	} else if (area==20) {
		int x0=62;  int y0=46;
		int x1=102; int y1=80;
		dothecave(&out, x0, x1, y0, y1);
		//fillarea(&out, 0,10, 12,12, "_s");
		fillarea(&out, 0,13, 13,25, "_s");
		fillarea(&out,14,15, 15,24, "_s");
		OMF(35,28, "1 Kud");
		OMF(22, 4, "2 Khr"); // dwarf's landing
		OMF( 7,30, "3 Kud"); // lair - oceb
		OMF(18,20, "4 Kud"); // skel - mid east
		OMF(26, 5, "5 Kud"); // bats
		OMF( 9, 9, "6 Kud"); // unused - dwaven keep

		OMF( 7,11, "Wo^Xo"); // crater entrance N
		OMF( 7,28, "Ww^Xo"); // crater entrance S
		OMF(33, 8, "Uu"); // bat backdoor
	} else if (area==21) {
		int x0=34;  int y0=52;
		int x1=80; int y1=80;
		dothecave(&out, x0, x1, y0, y1);
		OMF(35,24, "1 Kud");
		OMF( 4,13, "2 Kud");
		OMF(22, 5, "3 Kud"); // main NMe
		OMF(15,19, "4 Kud");
		OMF(37, 3, "5 Kud"); // old dwarven hideout - tmp bats
		OMF(35, 7, "6 Ko");  // drake's island keep -- unused
		OMF(30, 2, "Ql"); // bat backdoor
		OMF(35, 5, "Wo^Xo");
		// low water
		OMF(8, 6, "Wwf");
		OMF(7, 5, "Ww");
		OMF(7, 6, "Ww");
		OMF(6, 4, "Ww");
		OMF(5, 4, "Ww");
		OMF(4, 3, "Ww");
	} else if (area==22) {
		int x0=38; int y0=28;
		int x1=93; int y1=64;
		allocmap(&out, x1-x0, y1-y0);
		copymaparea(&out, &source, source.w, source.h, 0, 0, x0, y0, NULL);
		// secret-passage draught
		OMF(10,22, "Ce"); fillarea(&out,8,9, 22,23, "Ce"); // here or in .cfg?
		OMF( 4,30, "Wwf");
		OMF( 3,30, "Ww");
		OMF( 3,29, "Ww");
		OMF( 2,28, "Ww");
		OMF( 1,28, "Ww");
		OMF( 0,27, "Ww");
		OMF(28, 1, "Mm"); // top-cave batNskel backdoor

		OMF( 9,22, "Ke");
		OMF(11,22, "Gg^Ve"); // elf's village 

	} else if (area==23) {
		int x0=16;  int y0=43;
		int x1=48; int y1=70;
		dothecave(&out, x0, x1, y0, y1);
		copymaparea(&out, &source, source.w, 10, 0, 0, x0, y0, NULL);
		OMF( 4, 5, "1 Kh");
		OMF(22,22, "2 Kud");
	} else if (area==24) {
		int x0=16;  int y0=43;
		int x1=48; int y1=70;
		allocmap(&out, x1-x0, y1-y0);
		copymaparea(&out, &source, source.w, source.h, 0, 0, x0, y0, NULL);
		OMF( 4, 5, "1 Kh");
		OMF(22,22, "2 Mm");
	} else if (area==25) {
		int x0=38; int y0=28;
		int x1=93; int y1=64;
		dothecave(&out, x0, x1, y0, y1);

		OMF(10,22, "Ce"); fillarea(&out,8,9, 22,23, "Ce"); // here or in .cfg?
		OMF(53,18, "Uh"); // caved in bat backdoor
		OMF(49, 9, "Uu^Uf"); // caved in bat backdoor
		OMF(28, 1, "Mm"); // top-cave batNskel backdoor

		OMF(25,28, "Uu^Dr"); // reddie's joke

		OMF(18,29, "1 Kud");
		OMF(33,27, "2 Kud"); // north crater cave keep
		//OMF(46,22, "2 Kud"); // old dwarven keep
		OMF( 9,22, "3 Ke");
		OMF(29,11, "4 Khw");
		OMF(19, 6, "5 Kud");
		OMF(13,10, "6 Kud");
		OMF(27,17, "7 Kud"); // bats
		OMF(40,15, "8 Kud"); // bats or skels
		OMF(50,23, "9 Kud"); // bats
		//OMF(31,31, "9 Ko"); // old drake keep

		OMF(11,22, "Gg^Ve"); // elf's village 

		// secret-passage draught
		OMF( 4,30, "Wwf");
		OMF( 3,30, "Ww");
		OMF( 3,29, "Ww");
		OMF( 2,28, "Ww");
		OMF( 1,28, "Ww");
		OMF( 0,27, "Ww");
	} else if (area==26) {
		/* NorthWestWest: Cave */
		int x0=10; int y0=12;
		int x1=52; int y1=54;
		allocmap(&out, x1-x0, y1-y0);
		copymaparea(&out, &source, source.w, source.h, 0, 0, x0, y0, NULL);
		freemap(&source);
		// top-right cave
		read_mapfile(MAPROOT "cave.map", &source);
		copymaparea(&out, &source, 35 -28+x0, 29 -26+y0, 28-x0, 26-y0, 0, 0, "_off^_usr");
		copymaparea(&out, &source, 2, 9,                 35,    16,  35+x0-28, 16+y0-26, "_off^_usr");
		copymaparea(&out, &source, 1, 2,                 37,    20,  37+x0-28, 20+y0-26, "_off^_usr");
	}

	writemap(stdout, &out);
	return 0;
}
