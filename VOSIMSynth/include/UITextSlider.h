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
*  \file UITextSlider.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UITEXTSLIDER__
#define __UITEXTSLIDER__
#include "UIComponent.h"
#include <VoiceManager.h>
#include "UITextBox.h"

namespace syn
{
	class UITextSlider : public UIComponent
	{
	public:
		UITextSlider(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, int a_paramId);

		bool onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		UIComponent* onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;
		bool onMouseScroll(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) override;
		void setValueFromString(const string& a_str);
	protected:
		void draw(NVGcontext* a_nvg) override;
	private:
		void _onResize() override;
		void _updateValue();
		void _updateMinSize();
	private:
		double m_value;
		string m_valueStr, m_paramName;
		VoiceManager* m_vm;
		UITextBox* m_textBox;
		int m_unitId;
		int m_paramId;

		bool m_isValueDirty = false;
	};
}
#endif