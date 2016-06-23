#include "UIPlot.h"
#include <UILabel.h>
#include <tables.h>


namespace syn {
	UIPlot::UIPlot(VOSIMWindow* a_window) :
		UIComponent(a_window),
		m_minBounds{ 0, -1 },
		m_maxBounds{ 1, 1 },
		m_autoAdjustSpeed(1.0),
		m_yBufPtr(nullptr),
		m_bufSize(0),
		m_nScreenPts(0),
		m_crosshairPos{0.0,0.0},
		m_statusLabel(nullptr),
		m_interpPolicy(LinInterp), 
		m_xScale(LinScale) {
		setMinSize({ 200,200 });
	}

	void UIPlot::setBufferPtr(const double* a_yBufPtr, int a_bufSize) {
		m_yBufPtr = a_yBufPtr;
		m_bufSize = a_bufSize;
	}

	void UIPlot::setXBounds(const Vector2f& a_xBounds) {
		m_minBounds[0] = a_xBounds[0];
		m_maxBounds[0] = a_xBounds[1];
	}

	void UIPlot::setYBounds(const Vector2f& a_yBounds) {
		m_minBounds[1] = a_yBounds[0];
		m_maxBounds[1] = a_yBounds[1];
	}

	void UIPlot::setInterpPolicy(InterpPolicy a_policy) {
		m_interpPolicy = a_policy;
	}

	bool UIPlot::onMouseMove(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
		m_crosshairPos = a_relCursor.localCoord(this);
		_updateStatusLabel();
		return true;
	}

	void UIPlot::setXUnits(const string& a_xUnits) {
		m_xUnits = a_xUnits;
		_updateStatusLabel();
	}

	void UIPlot::setYUnits(const string& a_yUnits) {
		m_yUnits = a_yUnits;
		_updateStatusLabel();
	}

	Vector2i UIPlot::toScreenCoords(const Vector2f& a_sampleCoords) const {
		Vector2f unitCoords;
		unitCoords[1] = INVLERP(m_minBounds.y(), m_maxBounds.y(), a_sampleCoords.y());
		switch (m_xScale) {
		case LogScale2:
			unitCoords[0] = INVLERP(log2(m_minBounds.x()), log2(m_maxBounds.x()), log2(a_sampleCoords.x()));
			break;
		case LogScale10:
			unitCoords[0] = INVLERP(log10(m_minBounds.x()), log10(m_maxBounds.x()), log10(a_sampleCoords.x()));
			break;
		default:
		case LinScale:
			unitCoords[0] = INVLERP(m_minBounds.x(), m_maxBounds.x(), a_sampleCoords.x());
			break;
		}

		Vector2i screenCoords;
		screenCoords[0] = unitCoords.x()*size().x();
		screenCoords[1] = size().y() - unitCoords.y()*size().y();
		return screenCoords;
	}

	Vector2f UIPlot::toSampleCoords(const Vector2i& a_screenCoords) const {
		Vector2f unitCoords = { a_screenCoords.x() * 1.0 / size().x(), 1.0 - a_screenCoords.y() * 1.0 / size().y() };
		Vector2f sampleCoords;
		sampleCoords[1] = LERP<double>(m_minBounds.y(), m_maxBounds.y(), unitCoords.y());
		switch (m_xScale) {
		case LogScale2:
			sampleCoords[0] = pow(2.0, LERP(log2(m_minBounds.x()), log2(m_maxBounds.x()), unitCoords.x()));
			break;
		case LogScale10:
			sampleCoords[0] = pow(10.0, LERP(log10(m_minBounds.x()), log10(m_maxBounds.x()), unitCoords.x()));
			break;
		default:
		case LinScale:
			sampleCoords[0] = LERP(m_minBounds.x(), m_maxBounds.x(), unitCoords.x());
			break;
		}
		return sampleCoords;
	}

	void UIPlot::setStatusLabel(UILabel* a_statusLabel) {
		m_statusLabel = a_statusLabel;
		_updateStatusLabel();
	}

	void UIPlot::draw(NVGcontext* a_nvg) {
		nvgIntersectScissor(a_nvg, 0.0f, 0.0f, size()[0], size()[1]);

		// Draw background
		nvgBeginPath(a_nvg);
		nvgFillColor(a_nvg, Color(0.1f, 0.75f));
		nvgRect(a_nvg, 1, 1, size()[0] - 1, size()[1] - 1);
		nvgFill(a_nvg);

		nvgBeginPath(a_nvg);
		nvgStrokeColor(a_nvg, Color(0.0f, 1.0f));
		nvgRect(a_nvg, 0, 0, size()[0], size()[1]);
		nvgStroke(a_nvg);

		// Draw ticks
		Color tickColor{ colorFromHSL(0.19f, 1.0f, 0.5f) }; tickColor.a() = 0.9f;
		Color tickLabelColor{colorFromHSL(0.19f, 0.7f, 0.5f)}; tickColor.a() = 0.25f;
		nvgTextAlign(a_nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);	
		nvgFontSize(a_nvg, 12.0f);
		nvgStrokeColor(a_nvg, tickColor);
		nvgFillColor(a_nvg, tickLabelColor);
		nvgBeginPath(a_nvg);
		// Zero Y tick
		double zeroPct = INVLERP<double>(m_minBounds.y(), m_maxBounds.y(), 0.0);
		Vector2i zeroScreenPt = toScreenCoords({ 0.0, 0.0 });
		nvgMoveTo(a_nvg, 0.0f, zeroScreenPt.y());
		nvgLineTo(a_nvg, size().x(), zeroScreenPt.y());

		// Other Y ticks
		const int numYTicks = 10;
		const double yTickStep = 1. / numYTicks;
		double yTickPos = zeroPct;
		double yTickNeg = zeroPct-yTickStep;
		char buf[16];
		while (yTickNeg>0.0 || yTickPos <= 1.0) {
			if (yTickNeg > 0.0) {
				double samplePtPos = LERP<double>(m_minBounds.y(), m_maxBounds.y(), yTickNeg);
				Vector2i screenPtPos = toScreenCoords({ 0.0, samplePtPos });
				nvgMoveTo(a_nvg, 0.0f, screenPtPos.y());
				nvgLineTo(a_nvg, 10.0f, screenPtPos.y());
				sprintf(buf, "%.2f", samplePtPos);
				nvgText(a_nvg, 0.0f, screenPtPos.y(), buf, nullptr);
			}
			if (yTickPos <= 1.0) {
				double samplePtPos = LERP<double>(m_minBounds.y(), m_maxBounds.y(), yTickPos);
				Vector2i screenPtPos = toScreenCoords({ 0.0, samplePtPos });
				nvgMoveTo(a_nvg, 0.0f, screenPtPos.y());
				nvgLineTo(a_nvg, 10.0f, screenPtPos.y());
				sprintf(buf, "%.2f", samplePtPos);
				nvgText(a_nvg, 0.0f, screenPtPos.y(), buf, nullptr);
			}
			yTickNeg -= yTickStep;
			yTickPos += yTickStep;
		}		
		nvgStroke(a_nvg);

		// Draw paths
		const double bufPhaseStep = 1.0 / (double)m_nScreenPts;
		const Color lineColor{ 1.0f,1.0f };
		nvgBeginPath(a_nvg);
		nvgStrokeColor(a_nvg, lineColor);

		double bufPhase = 0.0;
		Vector2f yExtrema = { 0,0 };
		for (int j = 0; j < m_nScreenPts; j++) {
			double sampleVal;
			switch (m_interpPolicy) {
			case SincInterp:
				sampleVal = getresampled_single(m_yBufPtr, m_bufSize, bufPhase, m_nScreenPts, lut_blimp_table_online);
				break;
			case LinInterp:
			default:
			{
				int currOffset = static_cast<int>(bufPhase * m_bufSize);
				int nextOffset = WRAP(currOffset + 1, m_bufSize);
				sampleVal = LERP(m_yBufPtr[currOffset], m_yBufPtr[nextOffset], bufPhase * m_bufSize - currOffset);
				break;
			}
			}
			Vector2i screenPt = toScreenCoords(Vector2f{ LERP<double>(m_minBounds.x(),m_maxBounds.x(),bufPhase), sampleVal });
			yExtrema[1] = MAX<double>(yExtrema[1], sampleVal);
			yExtrema[0] = MIN<double>(yExtrema[0], sampleVal);
			if (j == 0)
				nvgMoveTo(a_nvg, screenPt[0], screenPt[1]);
			else
				nvgLineTo(a_nvg, screenPt[0], screenPt[1]);
			bufPhase += bufPhaseStep;
		}
		nvgStroke(a_nvg);

		_updateYBounds(yExtrema);

		// Draw crosshair
		nvgBeginPath(a_nvg);
		nvgStrokeColor(a_nvg, Color(1.0f, 0.25f));
		nvgMoveTo(a_nvg, 0, m_crosshairPos.y());
		nvgLineTo(a_nvg, size().x(), m_crosshairPos.y());
		nvgMoveTo(a_nvg, m_crosshairPos.x(), 0);
		nvgLineTo(a_nvg, m_crosshairPos.x(), size().y());
		nvgStroke(a_nvg);
	}

	void UIPlot::_onResize() {
		m_nScreenPts = size().x() << 1;
		_updateStatusLabel();
	}

	void UIPlot::_updateYBounds(const Vector2f& a_yExtrema) {
		// update extrema
		double margin = 0.10*(a_yExtrema[1] - a_yExtrema[0]);
		double minSetPoint = a_yExtrema[0] - margin;
		double maxSetPoint = a_yExtrema[1] + margin;
		double coeff = m_autoAdjustSpeed / m_window->fps();
		m_minBounds[1] = m_minBounds.y() + coeff * (minSetPoint - m_minBounds.y());
		m_maxBounds[1] = m_maxBounds.y() + coeff * (maxSetPoint - m_maxBounds.y());
		_updateStatusLabel();
	}

	void UIPlot::_updateStatusLabel() const {
		if (m_statusLabel) {
			Vector2f sample_pt = toSampleCoords(m_crosshairPos);
			char buf[64];
			sprintf(buf, "%.1f %s, %.4f %s", sample_pt.x(), m_xUnits.c_str(), sample_pt.y(), m_yUnits.c_str());
			m_statusLabel->setText(buf);
		}
	}
}