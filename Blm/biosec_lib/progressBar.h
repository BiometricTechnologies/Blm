#ifndef __progressBar_h__
#define __progressBar_h__

#include <cstdint>
#include <tuple>
#include <functional>
#include <string>
#include "boost/noncopyable.hpp"
#include "common.h"
//#include "../CollectorInterface/CollectorInterface_i.h"

namespace blm_utils{


	class ProgressBarWindow : private boost::noncopyable{
	public:
		ProgressBarWindow(const std::function<void(const wchar_t*, const wchar_t*, uint32_t, uint32_t, uint32_t, uint32_t)>& updateProgressFn);
		~ProgressBarWindow();

		void updateProgressFn(std::function<void(const wchar_t*, const wchar_t*, uint32_t, uint32_t, uint32_t, uint32_t)> val) { updateProgressFn_ = val; }
		void resetBarParams(std::int32_t barIndex, std::int32_t maxVal, std::int32_t curVal = 0) /* throw() */;
		void setCaption(const std::wstring& caption) /* throw() */;
		void setBarText(std::int32_t barIndex, const std::wstring& text) /* throw() */;
		void setBarCurrentValue(std::int32_t barIndex, std::int32_t currentValue) /* throw() */;
		void addBarCurrentValue(std::int32_t barIndex, std::int32_t val) /* throw() */;
		
	private:

		typedef enum{
			CURRENT = 0,
			MAX,
			CAPTION
		};

		static const std::uint32_t kUpperBarIndex = 0;
		static const std::uint32_t kLowerBarIndex = 1;
		static const std::uint32_t kBarsCount_ = 2;

		std::function<void(const wchar_t*, const wchar_t*, uint32_t, uint32_t, uint32_t, uint32_t)> updateProgressFn_;
		
		std::vector<std::tuple<std::int32_t, std::int32_t, std::wstring>> barsParams_; // current value - max value - caption
		
		void checkBarIndexParam(std::int32_t barIndex);
		void checkBarMaxValue(std::int32_t value);
		void checkBarValue(std::int32_t barIndex, std::int32_t value);
		void updatePbWindow();
	};



}

#endif // __progressBar_h__
