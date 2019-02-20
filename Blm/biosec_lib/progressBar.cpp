#include <algorithm>
#include "progressBar.h"

namespace blm_utils {
    std::int32_t getPercentRelationalValue(std::int32_t val, std::int32_t maxVal);

    std::int32_t getPercentRelationalValue(std::int32_t val, std::int32_t maxVal) {
        if(0 == maxVal) {
            if(val != 0) {
                throw std::exception("[getPercentRelationalValue] error division by 0");
            } else {
                return 0;
            }
        }
        return (val * 100 / maxVal);
    }


    ProgressBarWindow::ProgressBarWindow(const std::function<void(const wchar_t*, const wchar_t*, uint32_t, uint32_t, uint32_t, uint32_t)> &updateProgressFn)
        : updateProgressFn_(updateProgressFn) {
        barsParams_.resize(kBarsCount_);
        std::for_each(std::begin(barsParams_), std::end(barsParams_), [](std::tuple<std::int32_t, std::int32_t, std::wstring> &thisBarParams) {
            std::get<CURRENT>(thisBarParams) = 0;
            std::get<MAX>(thisBarParams) = 0;
            std::get<CAPTION>(thisBarParams) = L"";
        });
    }


    void ProgressBarWindow::checkBarIndexParam(std::int32_t barIndex) {
        if(barIndex < 0 || (barIndex + 1 > kBarsCount_)) {
            throw std::invalid_argument("[ProgressBarWindow::setBarParams] error: progressbarIndex");
        }
    }

    void ProgressBarWindow::resetBarParams(std::int32_t barIndex, std::int32_t maxVal, std::int32_t curVal /*= 0*/) {
        checkBarIndexParam(barIndex);
        checkBarMaxValue(maxVal);
        std::get<MAX>(barsParams_.at(barIndex)) = maxVal;
        checkBarValue(barIndex, curVal);
        std::get<CURRENT>(barsParams_.at(barIndex)) = curVal;
    }


    void ProgressBarWindow::setBarText(std::int32_t barIndex, const std::wstring &text) {
        checkBarIndexParam(barIndex);
        std::get<CAPTION>(barsParams_.at(barIndex)) = text;
    }


    void ProgressBarWindow::setCaption(const std::wstring &caption) {

    }


    void ProgressBarWindow::updatePbWindow() {

        updateProgressFn_(
            std::get<CAPTION>(barsParams_.at(kUpperBarIndex)).c_str(),
            std::get<CAPTION>(barsParams_.at(kLowerBarIndex)).c_str(),
            std::get<CURRENT>(barsParams_.at(kUpperBarIndex)),
            std::get<MAX>(barsParams_.at(kUpperBarIndex)),
            std::get<CURRENT>(barsParams_.at(kLowerBarIndex)),
            std::get<MAX>(barsParams_.at(kLowerBarIndex))
        );
    }


    ProgressBarWindow::~ProgressBarWindow() {

    }


    void ProgressBarWindow::setBarCurrentValue(std::int32_t barIndex, std::int32_t currentValue) { /* throw() */
        checkBarIndexParam(barIndex);
        checkBarValue(barIndex, currentValue);

        std::get<CURRENT>(barsParams_.at(barIndex)) = currentValue;
        updatePbWindow();
    }

    void ProgressBarWindow::checkBarMaxValue(std::int32_t value) {
        if(value < 0) {
            throw std::invalid_argument("[ProgressBarWindow::checkBarMaxValue] error: value");
        }
    }

    void ProgressBarWindow::checkBarValue(std::int32_t barIndex, std::int32_t value) {
        checkBarIndexParam(barIndex);
        if(value < 0 || value > std::get<MAX>(barsParams_.at(barIndex))) {
            //DLOG(ERROR) << " done value " << value << " greater than max value " << std::get<MAX>(barsParams_.at(barIndex));
            throw std::invalid_argument("[ProgressBarWindow::checkBarValue] error: value");
        }
    }

    void ProgressBarWindow::addBarCurrentValue(std::int32_t barIndex, std::int32_t val) { /* throw() */
        checkBarIndexParam(barIndex);
        checkBarValue(barIndex, val);
        std::get<CURRENT>(barsParams_.at(barIndex)) += val;
        updatePbWindow();
    }


}