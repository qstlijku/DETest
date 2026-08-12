
#include <SDL_log.h>

#define DEBUG_DRAW_IMPLEMENTATION
#define DEBUG_DRAW_MAX_LINES (1024 * 1024 * 10)
#define DEBUG_DRAW_OVERFLOWED(message) SDL_Log("%s\n", message)
#define STB_IMAGE_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION

#include "stb_image.h"
#include "dr_wav.h"
#include "debug_draw.hpp"
// TODO: Find appropriate version
#include "stb_vorbis.h"

