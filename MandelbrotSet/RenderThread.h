#pragma once

#include "Resource.h"
#include "framework.h"
#include <Debugapi.h>
#include <assert.h>
#include <thread>

#define OPERATION_NONE 0
#define OPERATION_DRAW 1
#define OPERATION_UPDATE_TEX 2
#define OPERATION_UPDATE_SEG 3


class RenderThread
{
public:
	void InitThread();
	void ReleaseThread();

	void DispatchDraw();
	void DispatchTexUpdate();
	void DispatchTexSegUpdate();
private:

	int RenderMain();
	void Draw();
	void TexUpdate();
	void TexSegUpdate();
	int TimingMain();

	std::thread* renderThread;
	std::thread* timingThread;

	HANDLE renderEvent;
	HANDLE timingEvent;

	short currentOperation = 0;
	short targetSegment = 0;
};