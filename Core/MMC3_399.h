#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_399 : public MMC3
{
private:
	uint8_t _regs[4];

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
	virtual uint16_t GetCHRPageSize() override { return 0x1000; }

	virtual void InitMapper() override
	{
		_regs[0] = _regs[2] = 0;
		_regs[1] = _regs[3] = 1;

		MMC3::InitMapper();
	}

	virtual void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_regs[0], _regs[1], _regs[2], _regs[3]);
	}

	void UpdatePrgMapping() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, _regs[0]);
		SelectPRGPage(2, _regs[1]);
		SelectPRGPage(3, -1);
	}

	void UpdateChrMapping() override
	{
		SelectCHRPage(0, _regs[2]);
		SelectCHRPage(1, _regs[3]);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if (addr < 0xA000) {
			if (addr & 1) {
				_regs[0 | (value >> 7)] = value;
			} else {
				_regs[2 | (value >> 7)] = value;
			}
			UpdateState();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};