/*
This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject.  If not, see <http://www.gnu.org/licenses/>.

Copyright 2016, Austen Satterlee
*/

/**
*  \file VOSIMWindow.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __MAINWINDOW__
#define __MAINWINDOW__

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <sftools/Chronometer.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/policies.hpp>
#include <list>

#include "nanovg.h"
#include "UI.h"
#include "perf.h"

#define MAX_GUI_MSG_QUEUE_SIZE 64

using sf::Event;
using boost::lockfree::spsc_queue;
using boost::lockfree::capacity;

namespace syn
{
	template <typename T>
	struct Timestamped
	{
		T data;
		sf::Time time;
	};

	class VOSIMWindow;

	struct GUIMessage
	{
		void(*action)(VOSIMWindow*, ByteChunk*);
		ByteChunk data;
	};

	class VoiceManager;
	class UnitFactory;

	class VOSIMWindow
	{
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		VOSIMWindow(int a_width, int a_height, VoiceManager* a_vm, UnitFactory* a_unitFactory);
		virtual ~VOSIMWindow();

		sf::RenderWindow* GetWindow() const { return m_sfmlWindow; }
		bool isOpen() const { return m_isOpen; }

		Vector2i getSize() const { return m_size; }

		Vector2i cursorPos() const;

		Vector2i diffCursorPos() const;

		Vector2i size() const;

		const Timestamped<Event::MouseButtonEvent>& lastClickEvent() const { return m_lastClick; }

		const Timestamped<Event::MouseWheelScrollEvent>& lastScrollEvent() const { return m_lastScroll; }

		shared_ptr<Theme> theme() const { return m_theme; }
		double fps() const { return m_fps; }

		UICircuitPanel* getCircuitPanel() const { return m_circuitPanel; }

		UnitFactory* getUnitFactory() const { return m_unitFactory; }

		NVGcontext* getContext() const { return m_vg; }

		void setFocus(UIComponent* a_comp);

		UIComponent* getFocused() const { return m_focusPath.empty() ? nullptr : m_focusPath.front(); }

		void clearFocus() { setFocus(nullptr); }

		void forfeitFocus(UIComponent* a_comp);

		bool OpenWindow(sf::WindowHandle a_system_window);
		void CloseWindow();

		void save(ByteChunk* a_data) const;
		int load(ByteChunk* a_data, int startPos);

		void reset();

		template <typename UnitType>
		void registerUnitControl(function<UIUnitControl*(VOSIMWindow*, VoiceManager*, int)> a_unitControlConstructor);

		UIUnitControl* createUnitControl(unsigned a_classId, int a_unitId);

		void queueExternalMessage(GUIMessage* a_msg);

#ifdef _WIN32
		void setHInstance(HINSTANCE a_newHInstance) { m_HInstance = a_newHInstance; }
		HINSTANCE getHInstance() const { return m_HInstance; }
		static LRESULT CALLBACK drawFunc(HWND Handle, UINT Message, WPARAM WParam, LPARAM LParam);
#endif
	private:
		/**
		 * Attach the sfml window to the given parent window.
		 */
		void _OpenWindowImplem(sf::WindowHandle a_system_window);
		void _CloseWindowImplem();
		void _initialize();
		void _runLoop();
		void _handleEvents();

		void _updateCursorPos(const Vector2i& newPos);

		void _queueInternalMessage(GUIMessage* a_msg);
		void _flushMessageQueues();
		void _processMessage(GUIMessage* a_msg);
	private:
#ifdef _WIN32
		HINSTANCE m_HInstance;
#endif
		sf::WindowHandle m_timerWindow;
		sf::RenderWindow* m_sfmlWindow;
		bool m_isOpen;

		NVGcontext* m_vg;

		Vector2i m_size;
		Vector2i m_cursor;
		Vector2i m_dCursor;
		Timestamped<Event::MouseButtonEvent> m_lastClick;
		Timestamped<Event::MouseWheelScrollEvent> m_lastScroll;

		UIComponent* m_draggingComponent;
		list<UIComponent*> m_focusPath;
		VoiceManager* m_vm;
		UnitFactory* m_unitFactory;

		UIComponent* m_root;
		UICircuitPanel* m_circuitPanel;
		shared_ptr<Theme> m_theme;

		PerfGraph m_fpsGraph;
		sftools::Chronometer m_timer;
		unsigned m_frameCount;
		double m_fps;

		map<unsigned, function<UIUnitControl*(VOSIMWindow*, VoiceManager*, int)>> m_unitControlMap;

		spsc_queue<GUIMessage*> m_guiInternalMsgQueue;
		spsc_queue<GUIMessage*> m_guiExternalMsgQueue;
	};

	template <typename UnitType>
	void VOSIMWindow::registerUnitControl(function<UIUnitControl*(VOSIMWindow*, VoiceManager*, int)> a_unitControlConstructor) {
		unsigned unitClassId = UnitType("").getClassIdentifier();
		m_unitControlMap[unitClassId] = a_unitControlConstructor;
	}
}
#endif
