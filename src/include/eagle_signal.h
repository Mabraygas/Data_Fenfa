#ifndef EAGLE_SIGNAL_H
#define EAGLE_SIGNAL_H
#include <signal.h>
#include <iostream>

namespace eagle
{

typedef void (sig_func)(int);

class SignalHandler
{
public:

	SignalHandler()
	{}

	SignalHandler(SignalHandler *handler);

	virtual ~SignalHandler() {}

	sig_func *set_signo(int signo)
	{
		return Signal(signo,HandleEntry);
	}

	void set_handler(SignalHandler *handler)
	{handler_ = handler;}

	SignalHandler * get_handler() const
	{return handler_;}

protected:

	virtual void HandleSignal(int signo) {}

private:

	static void HandleEntry(int signo)
	{
		if(handler_)
			handler_->HandleSignal(signo);
	}

	static sig_func *Signal(int signo, sig_func *func);

private:
	
	static SignalHandler *handler_;
};

}

#endif

