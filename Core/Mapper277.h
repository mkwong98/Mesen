#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper277 : public BaseMapper
{
private:
	bool _locked;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0, 0x08);
	}

	void Reset(bool softReset) override
	{
		_locked = false;
		WriteRegister(0, 0x08);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(!_locked) {
			uint8_t prgBank = value & 0x0F;

			_locked = (value & 0x20) == 0x20;

			if(value & 0x08) {
				if(value & 0x01) {
					SelectPRGPage(0, prgBank);
					SelectPRGPage(1, prgBank);
				} else {
					SelectPrgPage2x(0, prgBank & 0xFE);
				}
			} else {
				SelectPRGPage(0, prgBank);
				SelectPRGPage(1, prgBank | 0x07);
			}

			SelectCHRPage(0, 0);
			SetMirroringType(value & 0x10 ? MirroringType::Horizontal : MirroringType::Vertical);
		}
	}
};

