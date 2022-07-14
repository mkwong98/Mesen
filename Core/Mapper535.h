#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// Whirlwind Manu's cartridge conversion (cartrige code LH53) of the Famicom Disk System Game 謎の村雨城 (Nazo no Murasamejō).
// Its UNIF board name is UNL-LH53.

class Mapper535 : public BaseMapper
{
private:
	bool _irqEnabled;
	uint16_t _irqCounter;

protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	uint32_t GetWorkRamSize() override { return 0x2000; }
	uint32_t GetWorkRamPageSize() override { return 0x2000; }
	bool ForceWorkRamSize() override { return true; }

	void InitMapper() override
	{
		SelectPrgPage4x(0, 3 << 2);
		SelectCHRPage(0, 0);
		SetCpuMemoryMapping(0x6000, 0x7FFF, 0, PrgMemoryType::PrgRom);
		SetCpuMemoryMapping(0xB800, 0xD7FF, 0, PrgMemoryType::WorkRam);

		RemoveRegisterRange(0x8000, 0xDFFF);
	}

	void ProcessCpuClock() override
	{
		if (_irqEnabled) {
			_irqCounter++;
			if(_irqCounter == 7560) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF000) {
			case 0xE000:
				_irqEnabled = (value & 0x02) != 0;
				_irqCounter = 0;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;
			
			case 0xF000:
				SetCpuMemoryMapping(0x6000, 0x7FFF, value, PrgMemoryType::PrgRom);
				break;
		}
	}
};
