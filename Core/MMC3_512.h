#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_512 : public MMC3
{
private:
	uint8_t _reg = 0;

protected:
	virtual uint32_t GetChrRamSize() override { return 0x2000; }
	virtual uint16_t GetChrRamPageSize() override { return 0x0400; }

	virtual uint32_t GetWorkRamSize() override { return 0x2000; }
	virtual uint32_t GetWorkRamPageSize() override { return 0x2000; }
	virtual bool ForceWorkRamSize() override { return true; }

	virtual void InitMapper() override
	{
		AddRegisterRange(0x4100, 0x4FFF, MemoryOperation::Write);
		MMC3::InitMapper();
	}

	virtual void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_reg);
	}

	virtual void UpdateMirroring() override
	{
		if(_reg == 0x01) {
			SetPpuMemoryMapping(0x2000, 0x2FFF, 1 << 2, ChrMemoryType::ChrRam, MemoryAccessType::ReadWrite);
		} else {
			MMC3::UpdateMirroring();
		}
	}

	virtual void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(_reg & 0x02) {
			memoryType = ChrMemoryType::ChrRam;
			page &= 0x03;
		}

		MMC3::SelectCHRPage(slot, page, memoryType);
	}

	virtual void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		MMC3::SelectPRGPage(slot, page & 0x3F, memoryType);
	}

	virtual void UpdateState() override
	{
		MMC3::UpdateState();

		// Always enable WRAM
		SetCpuMemoryMapping(0x6000, 0x7FFF, 0, HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam, MemoryAccessType::ReadWrite);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000){
			if(addr & 0x100) {
				_reg = value & 0x03;
				UpdateChrMapping();
				UpdateMirroring();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};
