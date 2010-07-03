#include "UpdateStackMember.h"

UpdateStackMember::UpdateStackMember(int nType, int nFrame)
{
	type = nType;
	frame = nFrame;
}

int UpdateStackMember::getFrame()
{
	return frame;
}

int UpdateStackMember::getType()
{
	return type;
}