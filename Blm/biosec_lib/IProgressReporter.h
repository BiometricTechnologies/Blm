#ifndef __IProgressReporter_h__
#define __IProgressReporter_h__

#include <cstdint>
#include <memory>

namespace blm_utils{

class IProgressObserver {
public:
	IProgressObserver(){}
	virtual ~IProgressObserver(){}
	virtual void reportProgress(void* reporter, std::int64_t done, std::int64_t total) = 0;	
};

class IProgressReporter{
public:
	IProgressReporter():progressReceiver_(nullptr){}
	virtual ~IProgressReporter() = 0 {}
	void setReceiver(IProgressObserver* reciever) {
		progressReceiver_ = reciever;
	};
protected:
	IProgressObserver* getprogressReceiver(){
		return progressReceiver_;
	}
	bool checkProgressReceiver(){
		return (nullptr == progressReceiver_);
	}
private:
	IProgressObserver* progressReceiver_;
};

}

#endif // __IProgressReporter_h__
