#ifndef __biosecHeader_h__
#define __biosecHeader_h__

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>

namespace blm_utils{

class BiosecHeader {

public:
	typedef enum ProtectionMode {
		BIOMETRIC_ONLY,
		PASSWORD_ONLY,
		BIOMETRIC_AND_PASSWORD
	};
	typedef enum Result : int {
		SUCCESS = 0,
		FAIL = 1
	};
	ProtectionMode protectionMode() const {
		return protectionMode_;
	}
	void protectionMode(ProtectionMode val) {
		protectionMode_ = val;
	}

	std::string serialize() const {
		std::stringstream result;
		result << protectionMode() << ' ' << totalFiles() << ' ' << std::string(sampleData_.begin(), sampleData_.end());
		return result.str();
	}
	Result deserialize(const std::string &serialized) {
		std::stringstream input(serialized);
		int mode;
		int64_t total;
		input >> mode;
		if(!input.good() || (input.peek() != ' ')) {
			return FAIL;
		}
		input >> total;
		if(!input.good() || (input.peek() != ' ')) {
			return FAIL;
		}
		input.get();
		input.peek();
		if(!input.good()) {
			return FAIL;
		}
		protectionMode((ProtectionMode)mode);
		totalFiles(total);
		sampleData_.assign(serialized.begin() + input.tellg(), serialized.end());
		return SUCCESS;
	}
	std::int64_t totalFiles() const {
		return totalFiles_;
	}
	void totalFiles(std::int64_t val) {
		totalFiles_ = val;
	}
	std::vector<char> sampleData() const {
		return sampleData_;
	}

	BiosecHeader():
		protectionMode_(BIOMETRIC_AND_PASSWORD) {
			auto data = getDataForSample();
			sampleData_.assign(data, data + strlen(data));
	}
private:
	ProtectionMode protectionMode_;
	std::int64_t totalFiles_;
	std::vector<char> sampleData_;

	static const char* getDataForSample() {
		return "48d@kfg;s8-f0?mdhjs!de,7-38f0c*sdfs)dfk4dk";
	}

};

}

#endif // __biosecHeader_h__



