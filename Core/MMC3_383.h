#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_383 : public MMC3
{
private:
	uint8_t _A15;
	uint8_t _A16;
	uint8_t _A17A18;

protected:
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		MMC3::InitMapper();
		RemoveRegisterRange(0xC000, 0xFFFF, MemoryOperation::Read);
	}

	void Reset(bool softreset) override
	{
		_A15 = 0;
		_A16 = 0;
		_A17A18 = 0;
		MMC3::UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_A15, _A16, _A17A18);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		MMC3::SelectCHRPage(slot, (_A17A18 << 3) | (page & 0x7F), memoryType);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		switch(_A17A18) {
			case 0x00:
				page &= _A16 ? 0x07: 0x03;
				MMC3::SelectPRGPage(slot, _A17A18 | _A16 | (_A16 ? 0x00 : _A15) | page);
				break;
			case 0x10:
			case 0x20:
				MMC3::SelectPRGPage(slot, _A17A18 | (page & 0x0F));
				break;
			case 0x30:
				MMC3::SelectPRGPage(slot ^ 0x02, _A17A18 | (page & 0x0F));
				if(slot == 3) {
					SetCpuMemoryMapping(0x6000, 0x7FFF, _A17A18 | (page & 0x0B), PrgMemoryType::PrgRom);
				}
				break;
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if(_A17A18 == 0x00) {
			_A16 = MMC3::_registers[6 | ((addr >> 13) & 0x01)] & 0x08;
			MMC3::UpdatePrgMapping();
		}
		return InternalReadRam(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr & 0x100) {
			_A15 = (addr >> 11) & 0x04;
			_A17A18 = addr & 0x30;
			MMC3::UpdatePrgMapping();
			MMC3::UpdateChrMapping();
		}
		MMC3::WriteRegister(addr, value);
	}
};