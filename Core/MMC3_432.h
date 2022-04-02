#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_432 : public MMC3
{
private:
	uint8_t _exRegs[2];

protected:
	uint32_t GetDipSwitchCount() override { return 2; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		RemoveRegisterRange(0x6000, 0x7FFF, MemoryOperation::Read);
		MMC3::InitMapper();
	}

	void Reset(bool softreset) override
	{
		_exRegs[0] = 0;
		_exRegs[1] = 0;
		MMC3::UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_exRegs[0], _exRegs[1]);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		uint8_t mask = (_exRegs[1] & 0x04) ? 0x7F : 0xFF;
		page &= mask;
		page |= (((_exRegs[1] << 5) & 0x100) | ((_exRegs[1] << 7) & 0x80)) & ~mask;
		MMC3::SelectCHRPage(slot, page, memoryType);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint8_t mask = (_exRegs[1] & 0x02) ? 0x0F : 0x1F;
		uint8_t outerBank = ((_exRegs[1] << 1) & 0x20) | ((_exRegs[1] << 4) & 0x10);
		if(_exRegs[1] & 0x40) {
			MMC3::SelectPRGPage(0, (outerBank & ~mask) | (MMC3::_registers[6] & mask), memoryType);
			MMC3::SelectPRGPage(1, (outerBank & ~mask) | (MMC3::_registers[7] & mask), memoryType);
			MMC3::SelectPRGPage(2, (outerBank & ~mask) | (MMC3::_registers[6] & mask), memoryType);
			MMC3::SelectPRGPage(3, (outerBank & ~mask) | (MMC3::_registers[7] & mask), memoryType);
		} else {
			MMC3::SelectPRGPage(slot, (outerBank & ~mask) | (page & mask), memoryType);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if(_exRegs[0] & 0x01) {
			return GetDipSwitches();
		}
		return InternalReadRam(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(MMC3::CanWriteToWorkRam()) {
				_exRegs[addr & 0x01] = value;
				MMC3::UpdatePrgMapping();
				MMC3::UpdateChrMapping();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};