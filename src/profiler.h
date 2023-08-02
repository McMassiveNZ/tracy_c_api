#pragma once

#include <cinttypes>

void profiler_set_thread_name(const char* name);
void profiler_set_frame_mark();
void profiler_begin_frame_zone(const char* file, const char* function, uint32_t line, const char* name, uint32_t color);
void profiler_end_frame_zone();

#define ProfilerBeginFrameZone(NAME, COLOR) \
	profiler_begin_frame_zone(__FILE__, __FUNCTION__, __LINE__, NAME, COLOR)

#define ProfilerEndFrameZone \
	profiler_end_frame_zone()