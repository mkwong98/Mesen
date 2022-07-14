#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Bmc830752C : public BaseMapper
{
private:
	uint8_t _regs[2];

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_regs[0] = _regs[1] = 0;
		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_regs[0], _regs[1]);
	}

	void UpdateState()
	{
		uint8_t bank = (_regs[1] & 0x0F) << 3;
		SelectPRGPage(0, bank | (_regs[0] & 0x07));
		SelectPRGPage(1, bank | 0x07);

		SelectCHRPage(0, 0);

		SetMirroringType((_regs[1] & 0x60) ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if ((addr >= 0xA000) && (addr < 0xC000)) {
			_regs[1] = value;
		} else {
			_regs[0] = value;
		}

		UpdateState();
	}
};