
#include <windows.h> 
#include "Sphere.h"
#include <windows.h> 
#include "WindowSystem.h"
#include "OpenGLGraphicsSystem.h"

#include "Debug.h"
#include "Clock.h"
#include "InputSystem.h"

using namespace GE;
using namespace GE::Graphics;
using namespace GE::Utility;
bool run = true;

siglot::Slot<> closeSlot;

void Close(){
	run = false;
}

int main()
{

	EnableMemoryLeakChecking();
	closeSlot.Attach(&Close);

	WindowSystem& windowSystem = WindowSystem::GetInstance();
	windowSystem.Start("Skinning Animation Project", 800, 600);
	OpenGLGraphicsSystem & graphicsSystem = OpenGLGraphicsSystem::GetInstance();
	graphicsSystem.Start(windowSystem.getWindowHandler());
	//D3DGraphicsSystem* graphicsSystem = new D3DGraphicsSystem(windowSystem->getWindowHandler());

	InputSystem& inputSystem = InputSystem::GetInstance();

	windowSystem.resizeSignal.Connect(&graphicsSystem.resizeSlot);
	windowSystem.closeWindowSignal.Connect(&closeSlot);
	windowSystem.inputSignal.Connect(&inputSystem.inputSlot);
	windowSystem.outOfFocus.Connect(&inputSystem.restart);
	windowSystem.refresh.Connect(&graphicsSystem.refresh);
	windowSystem.uiEvent.Connect(&graphicsSystem.uiEvent);
	graphicsSystem.Initialize();

	Clock time;

	double dt = 1.0 / 60.0;
	double maxdt = 1.0 / 20.0;

	while(run){
		time.Tick();
		windowSystem.Update(dt);
		inputSystem.Update(dt);
		graphicsSystem.Update(dt);
		dt = time.Tock();
		if (dt > maxdt)
			dt = maxdt;
	}
	windowSystem.Release();
	graphicsSystem.Release();

	return 0;
}

