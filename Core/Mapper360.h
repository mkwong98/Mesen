#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// 31-in-1 multicart using dipswitch

class Mapper360 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint32_t GetDipSwitchCount() override { return 5; }

	void InitMapper() override
	{
		RemoveRegisterRange(0x8000, 0xFFFF);

		uint32_t _dipsw = GetDipSwitches();

		if (_dipsw < 2)
			SelectPrgPage2x(0, _dipsw & 0xFE);
		else {
			SelectPRGPage(0, _dipsw);
			SelectPRGPage(1, _dipsw);
		}

		SelectCHRPage(0, _dipsw);

		SetMirroringType((_dipsw & 0x10) ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
