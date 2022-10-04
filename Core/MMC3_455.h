#pragma once

#include "stdafx.h"
#include "MMC3.h"

class MMC3_455: public MMC3
{
private:
	uint8_t _exRegs[2];

protected:
	void InitMapper() override
	{
		_exRegs[0] = 0;
		_exRegs[1] = 1;

		AddRegisterRange(0x4100, 0x5FFF, MemoryOperation::Write);

		MMC3::InitMapper();
		MMC3::WriteRegister(0xA001, 0x80);
	}

	void Reset(bool softReset) override
	{
		_exRegs[0] = 0;
		_exRegs[1] = 1;

		MMC3::UpdateState();
	}


	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		uint16_t base = (((_exRegs[1] >> 2) & 0x10) | ((_exRegs[0] << 1) & 0x08) | ((_exRegs[1] >> 2) & 0x07)) << 4;
		uint16_t mask = (_exRegs[0] & 0x02) ? 0xFF : 0x7F;

		MMC3::SelectCHRPage(slot, (base & ~mask) | (page & mask));
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint16_t base = (((_exRegs[1] >> 2) & 0x10) | ((_exRegs[0] << 1) & 0x08) | ((_exRegs[1] >> 2) & 0x07)) << 1;
		uint16_t mask = (_exRegs[0] & 0x01) ? 0x1F : 0x0F;

		if(_exRegs[1] & 0x01) {
			uint8_t nrom = _exRegs[1] & 0x02;
			base = (slot & 0x02) ? (base | nrom) : (base & ~nrom);
			mask = 0x01;
			page = slot & 0x01;
		}

		MMC3::SelectPRGPage(slot, (base & ~mask) | (page & mask));
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if((GetState().RegA001 & 0x80) && (addr & 0x100)) {
				_exRegs[0] = (addr & 0xFF);
				_exRegs[1] = value;
				MMC3::UpdatePrgMapping();
				MMC3::UpdateChrMapping();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_exRegs[0], _exRegs[1]);
	}
};