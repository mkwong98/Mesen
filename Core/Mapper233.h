#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper233 : public BaseMapper
{
protected:
	uint8_t _reset;

	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_reset = 0;
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_reset);
	}

	void Reset(bool softReset) override
	{
		if(softReset) {
			_reset ^= 0x20;
		}

		WriteRegister(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(value & 0x20) {
			SelectPRGPage(0, _reset | (value & 0x1F));
			SelectPRGPage(1, _reset | (value & 0x1F));
		} else {
			SelectPRGPage(0, _reset | (value & 0x1E));
			SelectPRGPage(1, _reset | (value & 0x1E) + 1);
		}

		SelectCHRPage(0, 0);

		switch((value >> 6) & 0x03) {
			case 0: SetMirroringType(MirroringType::ScreenAOnly); break;
			case 1: SetMirroringType(MirroringType::Vertical); break;
			case 2: SetMirroringType(MirroringType::Horizontal); break;
			case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
		}
	}
};
