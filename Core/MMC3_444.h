#pragma once
#include "stdafx.h"
#include "MMC3.h"

// NES 2.0 Mapper 444 denotes the NC7000M multicart circuit board.
// Its UNIF board name is BMC-BS-110, which is the name of a different PCB whose name was mixed-up.

class MMC3_444 : public MMC3
{
private:
	uint8_t _exRegs;

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
		_exRegs = 0;
		MMC3::UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_exRegs);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		uint8_t mask = (_romInfo.SubMapperID & 0x01) ? 0xFF : 0x7F;
		uint16_t outerBank = (_exRegs << 7) & ((_romInfo.SubMapperID & 0x01) ? 0x00 : 0x80); // A17
		outerBank |= ((_exRegs << ((_romInfo.SubMapperID & 0x02) ? 4 : 7)) & 0x100); // A18
		MMC3::SelectCHRPage(slot, (outerBank & ~mask) | (page & mask), memoryType);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(_exRegs & 0x04) {
			page = (_exRegs << 4) | (MMC3::_registers[6] & 0x0F);
			if(_exRegs & 0x08) {
				page &= 0xFE;
				MMC3::SelectPrgPage2x(0, page, memoryType);
				MMC3::SelectPrgPage2x(1, page, memoryType);
			} else {
				page &= 0xFC;
				MMC3::SelectPrgPage4x(0, page, memoryType);
			}
		} else {
			MMC3::SelectPRGPage(slot, (_exRegs << 4) | (page & 0x0F), memoryType);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if((_exRegs & 0x0C) == 0x08) {
			return GetDipSwitches();
		}
		return InternalReadRam(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			/*if(MMC3::CanWriteToWorkRam()) */{
				_exRegs = (uint8_t)addr;
				MMC3::UpdatePrgMapping();
				MMC3::UpdateChrMapping();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};