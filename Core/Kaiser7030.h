#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Kaiser7030 : public BaseMapper
{
private:
	uint8_t _prgRegs[2];
	bool _old_mask_rom;

protected:
	uint16_t GetPRGPageSize() override { return 0x0400; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint32_t GetWorkRamSize() override { return 0x2000; }
	uint32_t GetWorkRamPageSize() override { return 0x0400; }
	uint16_t RegisterStartAddress() override { return 0x8000; }
	uint16_t RegisterEndAddress() override { return 0x9FFF; }

	void InitMapper() override
	{
		// last 32 KiB mapped to CPU $8000-$FFFF
		SetCpuMemoryMapping(0x8000, 0xFFFF, 0x60, PrgMemoryType::PrgRom); // rom-offset $18000

		_prgRegs[0] = 0x0F;
		_prgRegs[1] = 0x0F;
		_old_mask_rom = false;

		if(_romInfo.Hash.PrgCrc32 == 0xFA4DAC91) {
			_old_mask_rom = true;
		}

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_prgRegs[0], _prgRegs[1], _old_mask_rom);
	}

	void UpdateState()
	{
		if(_old_mask_rom) {
			// As the FCEUX source code comment indicates, the actual bank order in the 128 KiB mask ROM was unknown until July 2020. Emulators expected the ROM image to be laid out like this:
			// * the first 32 KiB to contain the eight banks selected by register $8000 mapped to $7000-$7FFF;
			// * the next 64 KiB to contain the sixteen banks selected by register $9000, with the first 1 KiB mapped to CPU $6C00-$6FFF and the second 3 KiB mapped to CPU $C000-$CBFF;
			// * the final 32 KiB mapped to CPU $8000-$FFFF except where replaced by RAM and the switchable PRG-ROM bank.
			SetCpuMemoryMapping(0x6000, 0x6BFF, 0, PrgMemoryType::WorkRam);
			SetCpuMemoryMapping(0x6C00, 0x6FFF, ((_prgRegs[1] & 0x0F) << 2) + 0x20, PrgMemoryType::PrgRom, MemoryAccessType::ReadWrite); // rom-offset $8000
			SetCpuMemoryMapping(0x7000, 0x7FFF, ((_prgRegs[0] & 0x07) << 2) + 0x00, PrgMemoryType::PrgRom, MemoryAccessType::ReadWrite); // rom-offset $0
			SetCpuMemoryMapping(0xB800, 0xBFFF, 3, PrgMemoryType::WorkRam);
			SetCpuMemoryMapping(0xC000, 0xCBFF, ((_prgRegs[1] & 0x0F) << 2) + 0x21, PrgMemoryType::PrgRom, MemoryAccessType::ReadWrite); // rom-offset $8400
			SetCpuMemoryMapping(0xCC00, 0xD7FF, 5, PrgMemoryType::WorkRam);
		} else {
			// The actual mask ROM layout is as follows:
			// * the first 64 KiB contain the sixteen banks selected by register $9000, with the first 3 KiB mapped to CPU $C000-$CBFF and the second 1 KiB mapped to CPU $6C00-$6FFF;
			// * the next 32 KiB contain the eight banks selected by register $8000 mapped to $7000-$7FFF;
			// * the final 32 KiB mapped to CPU $8000-$FFFF except where replaced by RAM and the switchable PRG-ROM bank.
			SetCpuMemoryMapping(0x6000, 0x6BFF, 0, PrgMemoryType::WorkRam);
			SetCpuMemoryMapping(0x6C00, 0x6FFF, ((_prgRegs[1] & 0x0F) << 2) + 0x03, PrgMemoryType::PrgRom, MemoryAccessType::ReadWrite); // rom-offset $0C00
			SetCpuMemoryMapping(0x7000, 0x7FFF, ((_prgRegs[0] & 0x07) << 2) + 0x40, PrgMemoryType::PrgRom, MemoryAccessType::ReadWrite); // rom-offset $10000
			SetCpuMemoryMapping(0xB800, 0xBFFF, 3, PrgMemoryType::WorkRam);
			SetCpuMemoryMapping(0xC000, 0xCBFF, ((_prgRegs[1] & 0x0F) << 2) + 0x00, PrgMemoryType::PrgRom, MemoryAccessType::ReadWrite); // rom-offset $0000
			SetCpuMemoryMapping(0xCC00, 0xD7FF, 5, PrgMemoryType::WorkRam);
		}

		SetMirroringType(_prgRegs[0] & 0x08 ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_prgRegs[(addr >> 12) & 0x01] = addr & 0x0F;

		UpdateState();
	}
};