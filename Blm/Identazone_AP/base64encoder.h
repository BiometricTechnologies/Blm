#ifndef __base64encoder_h__
#define __base64encoder_h__
#include <vector>
typedef unsigned char BYTE;
namespace blm_utils{

	std::vector<BYTE> base64_decode(std::string const& encoded_string);

}


#endif // __base64encoder_h__
