#include "profiler.h"
#include "tracy/TracyC.h"

#include <cassert>
#include <cstring>
#include <map>
#include <string_view>

struct TracyProfilerContextStack
{
	void Push(TracyCZoneCtx ctx)
	{
		assert(current_depth == 0 || (current_depth - 1) < s_stack_depth);
		data[current_depth++] = ctx;
	}

	TracyCZoneCtx Pop()
	{
		assert(current_depth > 0);
		return data[--current_depth];
	}

	const ___tracy_source_location_data* GetSourceLocationData(const char* file, const char* function, uint32_t line, const char* name, uint32_t color)
	{
		const std::string_view key_str(name);
		if (auto itr = sourceLocationData.find(key_str); itr != sourceLocationData.end())
		{
			return &itr->second;
		}

		___tracy_source_location_data& result = sourceLocationData[key_str];
		result.color = color;
		result.file = file;
		result.function = function;
		result.line = line;
		result.name = name;

		return &result;
	}

private:
	static constexpr uint32_t s_stack_depth = 64;
	TracyCZoneCtx data[s_stack_depth];
	std::map<std::string_view, ___tracy_source_location_data> sourceLocationData;
	uint32_t current_depth = 0;
};

static thread_local TracyProfilerContextStack s_tracy_context_stack;

void profiler_set_thread_name(const char* name)
{
	TracyCSetThreadName(name);
}

void profiler_set_frame_mark()
{
	TracyCFrameMark;
}

void profiler_begin_frame_zone(const char* file, const char* function, uint32_t line, const char* name, uint32_t color)
{
	const ___tracy_source_location_data* src_loc = s_tracy_context_stack.GetSourceLocationData(file, function, line, name, color);
	const TracyCZoneCtx current_ctx = ___tracy_emit_zone_begin(src_loc, true);
	s_tracy_context_stack.Push(current_ctx);
}

void profiler_end_frame_zone()
{
	const TracyCZoneCtx current_ctx = s_tracy_context_stack.Pop();
	___tracy_emit_zone_end(current_ctx);
}
