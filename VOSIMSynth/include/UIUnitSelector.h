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
*  \file UIUnitSelector.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UIUNITSELECTOR__
#define __UIUNITSELECTOR__
#include "nanovg.h"
#include "UIComponent.h"
#include "UnitFactory.h"

namespace syn
{
	class UIUnitSelector : public UIComponent
	{
	public:
		UIUnitSelector(VOSIMWindow* a_window, UnitFactory* a_unitFactory)
			: UIComponent{a_window}, m_autoWidth(0), m_autoHeight(0), m_currGroup(-1), m_currPrototype(-1), m_highlightedRow(-1), m_unitFactory(a_unitFactory) {}

		Vector2i calcAutoSize() const override;
		bool onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		int selectedGroup() { return m_currGroup; }
		int selectedPrototype() { return m_currPrototype; }
		void setSelectedGroup(int a_group) { m_currGroup = a_group; }
		void setSelectedPrototype(int a_proto) { m_currPrototype = a_proto; }
	protected:
		void draw(NVGcontext* a_nvg) override;
	private:
		int m_autoWidth, m_autoHeight;
		int m_currGroup, m_currPrototype;
		int m_highlightedRow;

		float m_fontSize = 14;
		UnitFactory* m_unitFactory;
	};
}
#endif
