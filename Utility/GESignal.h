#pragma once
#include "Callable.h"
#include <list>
#include "Delegate.h"
#include "PointerFunc.h"

namespace siglot{
	template<typename... args>
	class Slot;

	template<typename... args>
	class Signal{
		typedef Slot<args...> tSlot;
		friend class tSlot;
	public:
		void operator()(args... ar){
			for (auto slot : m_slots){
				(*slot)(ar...);
			}
		}

		void Connect(tSlot* slot){
			AppendSlot(slot);
			slot->AppendSignal(this);
		}

		void Disconnect(tSlot* slot){
			RemoveSlot(slot);
			slot->RemoveSignal(this);
		}
		~Signal(){
			for (tSlot* slot : m_slots){
				slot->RemoveSignal(this);
			}
		}
	private:
		void AppendSlot(tSlot* slot){
			m_slots.push_back(slot);
		}
		void RemoveSlot(tSlot* slot){
			m_slots.remove(slot);
		}
		std::list< tSlot*> m_slots;
	};

	template<typename... args>
	class Slot{
		typedef Callable<void, args...> tCallable;
		typedef Signal<args...> tSignal;
		friend class tSignal;
	public:
		void operator()(args... ar){
			for (auto callable : m_callable){
				(*callable)(ar...);
			}
		}

		template<typename tClass>
		void Attach(tClass * _oClass, void(tClass::* tMethod)(args...)){
			m_callable.push_back(new Delegate<tClass, void, args...>(_oClass, tMethod));
		}

		void Attach(void(*tPFunc)(args...)){
			m_callable.push_back(new PointerFunc<void, args...>(tPFunc));
		}

		template<typename tClass>
		void Detach(tClass * _oClass, void(tClass::* tMethod)(args...)){
			Delegate<tClass, void, args...> deleg(_oClass, tMethod);
			m_callable.remove_if([&deleg](tCallable* c){return (c->Equal(deleg) ? delete c, true : false); });
		}

		void Detach(void(*tPFunc)(args...)){
			PointerFunc<void, args...> pF(tPFunc);
			m_callable.remove_if([&pF](tCallable* c){return (c->Equal(pF) ? delete c, true : false); });
		}

		~Slot(){
			for (tSignal* signal : m_signals){
				signal->RemoveSlot(this);
			}
			for (tCallable* callable : m_callable){
				delete callable;
			}
		}
	private:

		void AppendSignal(tSignal* signal){
			m_signals.push_back(signal);
		}
		void RemoveSignal(tSignal* signal){
			m_signals.remove(signal);
		}

		std::list< tCallable*> m_callable;
		std::list< tSignal*> m_signals;
	};
}