#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_430 : public MMC3
{
private:
	uint8_t _reg;

protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		MMC3::InitMapper();
	}

	void Reset(bool softreset) override
	{
		_reg = 0;
		MMC3::UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_reg);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(_reg & 0x04) {
			MMC3::SelectCHRPage(slot, ((_reg << 6) & 0x180) | (page & 0x7F), memoryType);
		} else {
			MMC3::SelectCHRPage(slot, ((_reg << 6) & 0x100) | (page & 0xFF), memoryType);
		}
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(_reg & 0x08) {
			MMC3::SelectPRGPage(0, (_reg << 4) | ((MMC3::_registers[6] & ~0x02) & 0x0F));
			MMC3::SelectPRGPage(1, (_reg << 4) | ((MMC3::_registers[7] & ~0x02) & 0x0F));
			MMC3::SelectPRGPage(2, (_reg << 4) | ((MMC3::_registers[6] |  0x02) & 0x0F));
			MMC3::SelectPRGPage(3, (_reg << 4) | ((MMC3::_registers[7] |  0x02) & 0x0F));
		} else {
			MMC3::SelectPRGPage(slot, (_reg << 4) | (page & 0x0F));
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(MMC3::CanWriteToWorkRam()) {
				_reg = (uint8_t)addr;
				MMC3::UpdatePrgMapping();
				MMC3::UpdateChrMapping();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};