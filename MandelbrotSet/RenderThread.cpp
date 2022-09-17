#include "RenderThread.h"

int RenderThread::RenderMain()
{
	while (true)
	{
		WaitForSingleObject(renderEvent, INFINITE);
		switch (currentOperation)
		{
		case OPERATION_NONE:
			break;
		case OPERATION_DRAW:
			break;
		case OPERATION_UPDATE_TEX:
			break;
		case OPERATION_UPDATE_SEG:
			break;
		default:
			OutputDebugString(L"Unexpected operation in render main");
			break;
		}
		currentOperation = 0;
	}
	return 0;
}

void RenderThread::Draw()
{

}

void RenderThread::TexUpdate()
{

}

void RenderThread::TexSegUpdate()
{

}

int RenderThread::TimingMain()
{
	return 0;
}

RenderThread::RenderThread()
{
	InitThread();
}

void RenderThread::InitThread()
{
	renderEvent = CreateEvent(
		NULL,
		false,
		false,
		TEXT("RenderEvent")
	);
	if (renderEvent == NULL) {
		assert(false && "Render Event Creation Failed");
	}

	timingEvent = CreateEvent(
		NULL,
		false,
		false,
		TEXT("TimingEvent")
	);
	if (renderEvent == NULL) {
		assert(false && "Timing Event Creation Failed");
	}

	renderThread = new std::thread(&RenderThread::RenderMain, NULL);
	timingThread = new std::thread(&RenderThread::TimingMain, NULL);
}

void RenderThread::ReleaseThread()
{
	// Test this, this might crash the program
	renderThread->~thread();
	timingThread->~thread();
}

void RenderThread::DispatchDraw()
{
	currentOperation = OPERATION_DRAW;
	SetEvent(renderEvent);
}

void RenderThread::DispatchTexUpdate()
{
	currentOperation = OPERATION_UPDATE_TEX;
	SetEvent(renderEvent);
}

void RenderThread::DispatchTexSegUpdate()
{
	currentOperation = OPERATION_UPDATE_SEG;
	SetEvent(renderEvent);
}