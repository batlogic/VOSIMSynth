#include "UIUnitSelector.h"
#include "UnitFactory.h"
#include "Theme.h"

syn::UIComponent* syn::UIUnitSelector::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	int row = 0;
	int group = 0;
	set<string> gNames = m_unitFactory->getGroupNames();
	for (string gName : gNames) {
		vector<string> pNames = m_unitFactory->getPrototypeNames(gName);
		if (highlightedRow() == row) {
			if (m_currGroup == group)
				m_currGroup = -1;
			else {
				m_currGroup = group;
				m_currPrototype = -1;
				m_currPrototypeName = "";
			}
			return this;
		}
		if (group == m_currGroup) {
			for (int j = 0; j < pNames.size(); j++) {
				row++;
				if (highlightedRow() == row) {
					m_currPrototype = j;
					m_currPrototypeName = pNames[j];
					return this;
				}
			}
		}
		row++;
		group++;
	}
	m_currGroup = -1;
	m_currPrototype = -1;
	m_currPrototypeName = "";
	return nullptr;
}

bool syn::UIUnitSelector::onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	int row = 0;
	int group = 0;
	set<string> gNames = m_unitFactory->getGroupNames();
	Vector2i cursor = a_relCursor - m_pos;
	int groupFontSize = theme()->mUnitSelectorGroupFontSize;
	int protoFontSize = theme()->mUnitSelectorProtoFontSize;
	int rowPix = 0;
	for (string gName : gNames) {
		vector<string> pNames = m_unitFactory->getPrototypeNames(gName);
		rowPix += groupFontSize;
		if (cursor.y() < rowPix) {
			m_highlightedRow = row;
			return true;
		}
		if (group == m_currGroup) {
			for (int j = 0; j < pNames.size(); j++) {
				row++;
				rowPix += protoFontSize;
				if (cursor.y() < rowPix) {
					m_highlightedRow = row;
					return true;
				}
			}
		}
		row++;
		group++;
	}
	return false;
}

void syn::UIUnitSelector::draw(NVGcontext* a_nvg) {
	int groupFontSize = theme()->mUnitSelectorGroupFontSize;
	int protoFontSize = theme()->mUnitSelectorProtoFontSize;

	nvgTextAlign(a_nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

	nvgFontBlur(a_nvg, 0.1f);

	int row = 0;
	int group = 0;
	set<string> gNames = m_unitFactory->getGroupNames();
	m_autoHeight = 0;
	m_autoWidth = 0;
	for (string gName : gNames) {
		vector<string> pNames = m_unitFactory->getPrototypeNames(gName);
		nvgFontFace(a_nvg, "sans-bold");
		if (highlightedRow() == row)
			nvgFillColor(a_nvg, theme()->mHighlightedTextColor);
		else
			nvgFillColor(a_nvg, theme()->mTextColor);
		nvgFontSize(a_nvg, groupFontSize);
		m_autoWidth = MAX(m_autoWidth, 5 + nvgText(a_nvg, 0, m_autoHeight, gName.c_str(), nullptr));
		m_autoHeight += groupFontSize;
		if (group == m_currGroup) {
			nvgFontSize(a_nvg, protoFontSize);
			nvgFontFace(a_nvg, "sans");
			for (int j = 0; j < pNames.size(); j++) {
				row++;
				if (highlightedRow() == row || m_currPrototype == j)
					nvgFillColor(a_nvg, theme()->mHighlightedTextColor);
				else
					nvgFillColor(a_nvg, theme()->mTextColor);
				m_autoWidth = MAX(m_autoWidth, 5 + nvgText(a_nvg, m_indentAmt, m_autoHeight, pNames[j].c_str(), nullptr));
				m_autoHeight += protoFontSize;
			}
		}
		row++;
		group++;
	}
	setMinSize_(Vector2i{m_autoWidth, m_autoHeight});
	setSize(Vector2i{m_autoWidth, m_autoHeight});
}