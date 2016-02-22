#pragma once
#include "System.h"
#include "Delegate.h"
#include <assert.h> 
#include <GLM\glm.hpp>
#include "GESignal.h"

#define MLEFT_BUT 0
#define MRIGHT_BUT 1

enum{ L_SHIFT =16,
	NUM0 =96,
	NUM2 = 98,
	NUM4 = 100,
	NUM5 = 101,
	NUM6 = 102,
	NUM8 = 104,
	NUM7 = 103,
	NUM1 = 97,
	NUM_PLUS = 107,
	NUM_MINUS = 109,
	NUM_POINT = 110,

};


namespace GE{

		class InputSystem :
			public System
		{
		public:

			class KeyBoard;
			class Mouse;

			static InputSystem& GetInstance();

			
			void Update(double);
			siglot::Slot<int, int, int> inputSlot;

			KeyBoard* const keyboard;
			Mouse * const mouse;

			virtual void Input(int, int, int);

			siglot::Slot<> restart;

		private:
			siglot::Slot<int, int> inputFunctions[4];
			void Restart();

			InputSystem();
			~InputSystem();

		public:

			class KeyBoard
			{
				friend class InputSystem;
			public:
				~KeyBoard();

				bool GetButtonState(int b);

				siglot::Signal<int> KeyDown;
				siglot::Signal<int> KeyUp;

			private:
				int	keys[256];
				KeyBoard();
				void RegisterButtonEvent(int b, int state);
				void Restart();
			};

			class Mouse
			{
				friend class InputSystem;
			public:
				~Mouse(){}

				siglot::Signal<int> KeyDown;
				siglot::Signal<int> KeyUp;
				siglot::Signal<int, int> MouseMove;

				bool GetButtonState(int b);
				void GetPosition(glm::vec2&v) const;
				float GetWheel()const;
				glm::vec2 GetPosition() const;

				enum{ LEFT=0, RIGHT=1};
			private:
				glm::vec2 pos;
				float wheelPos;
				union {
					struct {
						int left, right;
					};
					int button[2];
				};

				Mouse();
				void SetNewPosition(int x, int y);
				void SetButtonState(int b, int state);
				void SetDeltaWheel(int , int delta);
				void Restart();

			};


		};
}

