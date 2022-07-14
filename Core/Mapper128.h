#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// 4-in-1 pirate multicart

class Mapper128 : public BaseMapper
{
private:
	uint8_t _reg;
	uint16_t _outerBank;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_outerBank = 0;
		WriteRegister(0x8000, 0);
	}

	void Reset(bool softReset) override
	{
		_outerBank = 0;
		WriteRegister(0x8000, 0);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_reg, _outerBank);
	}

	void UpdateState()
	{
		SelectPRGPage(0, ((_outerBank >> 2) & 0x18) | (_reg & 0x07));
		SelectPRGPage(1, ((_outerBank >> 2) & 0x18) | 0x07);
		SelectCHRPage(0, 0);
		SetMirroringType((_outerBank & 0x02) ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(_outerBank < 0xF000) {
			_outerBank = addr;
		}
		_reg = value;
		UpdateState();
	}
};
