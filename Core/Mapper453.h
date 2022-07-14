#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// Realtec 8042

class Mapper453 : public BaseMapper
{
private:
	uint8_t _reg;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_reg = 0;
		WriteRegister(0x8000, 0);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_reg);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if (_reg & 0xE0) {
			_reg &= 0xE0;
			_reg |= value & ~0xE0;
		} else {
			_reg = value;
		}

		SelectCHRPage(0, 0);
		if (_reg & 0x40) {
			SelectPrgPage2x(0, (((_reg >> 3) & 0x18) | (_reg & 0x07)) << 1);
			SetMirroringType((_reg & 0x10) ? MirroringType::ScreenAOnly : MirroringType::ScreenBOnly);
		} else {
			SelectPRGPage(0, ((_reg >> 2) & 0x38) | (_reg & 0x07));
			SelectPRGPage(1, ((_reg >> 2) & 0x38) | 0x07);
			SetMirroringType((_reg & 0x10) ? MirroringType::Horizontal : MirroringType::Vertical);
		}
	}
};
