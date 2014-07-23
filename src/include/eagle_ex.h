#ifndef EAGLE_EX_H
#define EAGLE_EX_H
#include <cerrno>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include <execinfo.h>

namespace eagle
{
class EagleException : public std::exception
{
public:
	
	explicit EagleException(const std::string &buffer);

	EagleException(const std::string &buffer, int err);

	virtual ~EagleException() throw();
	
	virtual const char* what() const throw();

	int getErrCode() { return int_code_; }


private:
	
	std::string str_buffer_;

	int int_code_;
};

}

#endif
