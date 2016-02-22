#include <Windows.h>

namespace GE{
	namespace Utility{
		class Clock{
			LARGE_INTEGER ticksPerSecond;
			LARGE_INTEGER tick;   // A point in time
			LARGE_INTEGER tock;   // For converting tick into real time
		public:
			Clock(){
				QueryPerformanceFrequency(&ticksPerSecond);
			}

			void Tick(){
				QueryPerformanceCounter(&tick);
			}

			double Tock(){
				QueryPerformanceCounter(&tock);
				return (double)(tock.QuadPart - tick.QuadPart) / (double)(ticksPerSecond.QuadPart);
			}

		};
	}
}