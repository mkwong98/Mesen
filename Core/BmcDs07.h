#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class BmcDs07 : public BaseMapper
{
private:
	uint8_t _regs[2];
	uint8_t _latch;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		_regs[0] = _regs[1] = -1;
		_latch = 0;
		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_regs[0], _regs[1], _latch);
	}

	void UpdateState()
	{
		uint8_t base = (_regs[0] & 0xF0) >> 1;
		SelectPRGPage(0, base | (_latch & 0x07));
		SelectPRGPage(1, base | 0x07);
		SetMirroringType((_latch & 0x80) ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_regs[addr & 0x01] = value;
		} else {
			if((_regs[0] & 0x80) == 0) {
				_latch = (_latch & 0xF8) | (value & 0x07);
			} else {
				_latch = value;
			}
		}
		UpdateState();
	}
};