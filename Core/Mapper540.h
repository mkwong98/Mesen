#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper540 : public Mapper359
{
	virtual uint16_t GetCHRPageSize() override { return 0x0800; }

	virtual void UpdateChrMapping() override
	{
		if(HasChrRom()) {
			SelectCHRPage(0, _chrBanks[0]);
			SelectCHRPage(1, _chrBanks[1]);
			SelectCHRPage(2, _chrBanks[6]);
			SelectCHRPage(3, _chrBanks[7]);
		}
	}
};
