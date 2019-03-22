#pragma once


typedef unsigned short USH;
typedef unsigned char UCH;


static size_t WSIZE = 32 * 1024;
static size_t MIN_MATCH = 3;
static size_t MAX_MATCH = 255 + MIN_MATCH;

static size_t WMASK = WSIZE - 1;
