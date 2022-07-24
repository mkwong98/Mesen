#pragma once
#include "MMC3.h"

class MMC3_458 : public MMC3
{
private:
	uint8_t _exReg;

protected:
	uint32_t GetDipSwitchCount() override { return 2; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		_exReg = 0;
		MMC3::InitMapper();

		AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Write);
	}

	void Reset(bool softReset) override
	{
		_exReg = 0;

		MMC3::UpdateState();
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		MMC3::SelectCHRPage(slot, ((_exReg << 4) & ~0x7F) | (page & 0x7F));
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint16_t base = _exReg & 0x0F;
		uint16_t nrom = (_romInfo.SubMapperID == 1) ? ((_exReg & 0x02) >> 3) : ((_exReg & 0x10) >> 4);

		page = (slot & 1);
		page |= ((slot & 0x02) ? (base | nrom) : (base & ~nrom)) << 1;

		MMC3::SelectPRGPage(slot, page);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		uint8_t dipValue = GetDipSwitches() & 0x03;

		if((_exReg & 0x20) && dipValue) {
			addr = (addr & ~0x03) | dipValue;
		}

		return InternalReadRam(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_exReg = addr & 0xFF;
			MMC3::UpdatePrgMapping();
			MMC3::UpdateChrMapping();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_exReg);
	}
};