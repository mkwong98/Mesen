#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper400 : public BaseMapper
{
private:
	uint8_t _regs[2];
	uint8_t _led;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		memset(_regs, 0x00, sizeof(_regs));

		_regs[0] = 0x80;
		
		AddRegisterRange(0x7800, 0x7FFF, MemoryOperation::Write);

		WriteRegister(0xC000, 0);
	}

	virtual void Reset(bool softReset) override
	{
		_regs[0] = 0x80;

		WriteRegister(0xC000, 0);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_regs[0], _regs[1], _led);
	}

	void UpdateState()
	{
		SelectPRGPage(0, (_regs[0] & 0xF8) | (_regs[1] & 0x07));
		SelectPRGPage(1, (_regs[0] & 0xF8) | 0x07);

		SelectCHRPage(0, _regs[1] >> 5);

		if(!(_regs[0] == 0x80)) {
			SetMirroringType((_regs[0] & 0x20) ? MirroringType::Horizontal : MirroringType::Vertical);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_regs[0] = value;
			UpdateState();
		} else {
			if(addr < 0xC000) {
				_led = value;
			} else {
				_regs[1] = value;
				UpdateState();
			}
		}
	}
};
