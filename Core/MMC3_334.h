#pragma once
#include "MMC3.h"

class MMC3_334 : public MMC3
{
protected:
    uint32_t GetDipSwitchCount() override { return 1; }
    bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		MMC3::InitMapper();
		SelectPrgPage4x(0, -4);
		SelectChrPage8x(0, 0);
        AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Any);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
	}

	void UpdatePrgMapping() override
	{
	}

    uint8_t ReadRegister(uint16_t addr) override
    {
        uint8_t value = _console->GetMemoryManager()->GetOpenBus();

        if(addr & 0x02) {
            value &= ~0x01;
            value |= GetDipSwitches() & 0x01;
        }

        return value;
    }

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
            if(!(addr & 0x01)) {
				SelectPrgPage4x(0, (value & 0xFE) << 1);
            }
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};