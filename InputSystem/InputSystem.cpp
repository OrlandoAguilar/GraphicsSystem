//http://www.cambiaresearch.com/articles/15/javascript-char-codes-key-codes

#include "InputSystem.h"
#include "Debug.h"

namespace GE{
		InputSystem & InputSystem::GetInstance()
		{
			static InputSystem input;
			return input;
		}

		InputSystem::InputSystem() :
			System(INPUT_SYSTEM),
			keyboard(new KeyBoard()),
			mouse(new Mouse())
		{
			inputSlot.Attach(this, &GE::InputSystem::Input);
			inputFunctions[0].Attach(keyboard, &GE::InputSystem::KeyBoard::RegisterButtonEvent);
			inputFunctions[1].Attach(mouse, &GE::InputSystem::Mouse::SetButtonState);
			inputFunctions[2].Attach(mouse, &GE::InputSystem::Mouse::SetNewPosition);
			inputFunctions[3].Attach(mouse, &GE::InputSystem::Mouse::SetDeltaWheel);
			restart.Attach(this, &GE::InputSystem::Restart);

		}

		void InputSystem::Restart(){
			mouse->Restart();
			keyboard->Restart();
		}

		InputSystem::~InputSystem()
		{
			delete mouse;
			delete keyboard;
		}

		void InputSystem::Update(double){
		}

		void InputSystem::Input(int type, int p1, int p2){
			inputFunctions[type](p1, p2);
		}

		bool InputSystem::KeyBoard::GetButtonState(int b){
				return keys[b] == 1;
		}

		InputSystem::KeyBoard::KeyBoard(){
			for (int z = 0; z < 256; ++z)
				keys[z] = 0;
		}

		void InputSystem::KeyBoard::RegisterButtonEvent(int b, int state){
			keys[b] = state;
			if (state == 1){
				KeyDown(b);
			}
			else{
				KeyUp(b);
			}
		}

		InputSystem::KeyBoard::~KeyBoard(){}


		void InputSystem::KeyBoard::Restart(){
			for (int z = sizeof(keys) / sizeof(keys[0]) - 1; z >= 0; --z)
				keys[z] = 0;
		}

		bool InputSystem::Mouse::GetButtonState(int b){
			return button[b] == 1;
		}

		void InputSystem::Mouse::GetPosition(glm::vec2&v) const{
			v = pos;
		}

		glm::vec2 InputSystem::Mouse::GetPosition() const{
			return pos;
		}

		float InputSystem::Mouse::GetWheel()const{
			return wheelPos;
		}

		InputSystem::Mouse::Mouse() : pos(0), left(0), right(0), wheelPos(0.0f)
		{
		}

		void InputSystem::Mouse::SetNewPosition(int x, int y){
			pos.x = static_cast<float>(x);
			pos.y = static_cast<float>(y);
			MouseMove(x, y);
		}

		void InputSystem::Mouse::SetButtonState(int b, int state){
			button[b] = state;
			if (state == 1){
				KeyDown(b);
			}
			else{
				KeyUp(b);
			}
		}
		void InputSystem::Mouse::SetDeltaWheel(int, int delta){
			wheelPos += delta;
		}

		void InputSystem::Mouse::Restart(){
			left = 0;
			right = 0;
		}
}
