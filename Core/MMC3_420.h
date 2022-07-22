#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_420 : public MMC3
{
private:
	uint8_t _exRegs[4];

protected:
	virtual void InitMapper() override
	{
		AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Write);
		MMC3::InitMapper();
	}

	virtual void Reset(bool softReset) override
	{
		memset(_exRegs, 0, sizeof(_exRegs));

		if(!softReset) {
			MMC3::ResetMmc3();
		}

		MMC3::UpdateState();
	}

	virtual void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_exRegs[0], _exRegs[1], _exRegs[2], _exRegs[3]);

		if (!saving) {
			MMC3::UpdateState();
		}
	}

	virtual void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		uint16_t mask = 0xFF >> ((_exRegs[1] & 0x80) >> 7);
		uint16_t base = ((_exRegs[1] << 1) & 0x100) | ((_exRegs[1] << 5) & 0x80);

		MMC3::SelectCHRPage(slot, base | (page & mask), memoryType);
	}

	virtual void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint16_t mask;
		uint16_t base;
		if(_exRegs[0] & 0x80) {
			base = (((_exRegs[0] >> 1) & 0x07) | ((_exRegs[3] >> 2) & 0x08)) << 2;
			page = slot;
			mask = 0x03;
		} else {
			mask = 0x3F >> (((_exRegs[3] & 0x20) >> 5) | ((_exRegs[0] & 0x20) >> 4));
			base = ((_exRegs[3] << 3) & 0x20);
		}

		MMC3::SelectPRGPage(slot, base | (page & mask), memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			WritePrgRam(addr, value);
			_exRegs[addr & 3] = value;
			MMC3::UpdatePrgMapping();
			MMC3::UpdateChrMapping();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};
