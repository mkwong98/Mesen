#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// bootleg cartridge conversion of the original FDS version of Kid Icarus (パルテナの鏡).

class Mapper539 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	uint32_t GetWorkRamSize() override { return 0x2000; }
	uint32_t GetWorkRamPageSize() override { return 0x0100; }
	bool ForceWorkRamSize() override { return true; }

	void InitMapper() override
	{
		RemoveRegisterRange(0x0000, 0xFFFF, MemoryOperation::Any);
		AddRegisterRange(0xA000, 0xAFFF, MemoryOperation::Write);
		AddRegisterRange(0xF000, 0xFFFF, MemoryOperation::Write);

		SelectPRGPage(0, 0xC);
		SelectPRGPage(1, 0);
		SelectPRGPage(2, 0xE);
		SelectPRGPage(3, 0xF);
		SelectCHRPage(0, 0);

		SetCpuMemoryMapping(0x6000, 0x7FFF, 0x0D, PrgMemoryType::PrgRom);

		// Certain ranges in the CPU address space are overlaid with portions of 8 KiB of PRG-RAM as follows:
		// TODO: This mapping seemed to work so whateva
		SetCpuMemoryMapping(0x6000, 0x60FF, 0x18, PrgMemoryType::WorkRam);
		SetCpuMemoryMapping(0x6200, 0x62FF, 0x1A, PrgMemoryType::WorkRam);
		SetCpuMemoryMapping(0x6400, 0x65FF, 0x1C, PrgMemoryType::WorkRam);
		SetCpuMemoryMapping(0x8200, 0x82FF, 0x12, PrgMemoryType::WorkRam);
		SetCpuMemoryMapping(0xC000, 0xD1FF, 0x00, PrgMemoryType::WorkRam);
		SetCpuMemoryMapping(0xDF00, 0xDFFF, 0x1F, PrgMemoryType::WorkRam);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF000) {
			case 0xA000:
				SelectPRGPage(1, value);
				break;

			case 0xF000:
				SetMirroringType((value & 8) ? MirroringType::Horizontal : MirroringType::Vertical);
				break;
		}
	}
};
