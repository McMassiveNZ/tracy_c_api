#include <cstdio>
#include <thread>
#include <vector>
#include <chrono>

#include "profiler.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

using namespace std::chrono_literals;

static bool s_shouldClose;

void do_work(int task_id)
{
	char buf[32] = {};
	sprintf_s(buf, "Worker_%d", task_id);
	profiler_set_thread_name(buf);

	while (!s_shouldClose)
	{
		ProfilerBeginFrameZone("do_work", 0xFFFF00);
		printf("Executing Task %d\n", task_id);
		std::this_thread::sleep_for(2ms);
		ProfilerEndFrameZone;
	}
}

static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps = {};
			HDC hdc = BeginPaint(window, &ps);
			FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
			EndPaint(window, &ps);
			return 0;
		}
	}
	return DefWindowProc(window, message, wParam, lParam);
}

auto main() -> int
{
 	const char className[] = "tracy-c-api";
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASSEX wc = {};

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = className;

	if (RegisterClassEx(&wc) == 0)
	{
		MessageBox(nullptr, "Call to RegisterClass failed", "Fatal Error", MB_OK);
		return 0;
	}

	HWND window = CreateWindowEx(
		0,
		className,
		"tracy-c-api-window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	ShowWindow(window, SW_SHOW);

	std::vector<std::jthread> threadPool;
	for (auto i = 0u; i < std::thread::hardware_concurrency(); i++)
		threadPool.emplace_back(do_work, i);

	auto last = std::chrono::high_resolution_clock::now();
	while (!s_shouldClose)
	{
		ProfilerBeginFrameZone("Main Thread", 0xFF00FF);

		MSG message = {};
		while (PeekMessage(& message, NULL, 0, 0, PM_REMOVE) == TRUE)
		{
			if (message.message == WM_QUIT)
				s_shouldClose = true;

			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		auto current = std::chrono::high_resolution_clock::now();
		auto dt = current - last;
		auto remaining = 16ms - std::chrono::duration_cast<std::chrono::milliseconds>(dt);
		last = current;

		std::this_thread::sleep_for(remaining);
		ProfilerEndFrameZone;
		profiler_set_frame_mark();
	}
}

