#pragma once
#ifndef _SERVICE_H
#define _SERVICE_H
#include "headers.h"
#include "utils.h"

enum ServiceState
{
	ACTIVE, INACTIVE
};
class Service {
public:
	MPEString serviceName;
	ServiceState serviceState;
private:
};
#endif // !_SERVICE_H
