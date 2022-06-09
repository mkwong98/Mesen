#pragma once
#include "MMC3.h"

class MMC3_370 : public MMC3
{
protected:
	uint32_t GetDipSwitchCount() override { return 1; }
	bool AllowRegisterRead() override { return true; }

	uint8_t _exReg;

	void InitMapper() override
	{
		_exReg = 0;
		MMC3::InitMapper();
		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Any);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
	}

	void Reset(bool softreset) override
	{
		_exReg = 0;
		MMC3::UpdateState();
	}

	void UpdateMirroring() override
	{
		if((_exReg & 0x07) != 0x01) {
			MMC3::UpdateMirroring();
		}
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		uint8_t mask = (_exReg & 0x04) ? 0xFF : 0x7F;
		MMC3::SelectCHRPage(slot, ((_exReg << 7) & 0x380) | (page & mask));
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint8_t mask = (_exReg & 0x20) ? 0x0F : 0x1F;
		MMC3::SelectPRGPage(slot, ((_exReg << 1) & 0x70) | (page & mask));
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		uint8_t value = _console->GetMemoryManager()->GetOpenBus();

		value &= ~0x80;
		value |= ((GetDipSwitches() << 7) & 0x80);

		return value;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x5FFF) {
			_exReg = addr & 0x3F;
			UpdatePrgMapping();
			UpdateChrMapping();
		} else {
			if(((_exReg & 0x07) == 0x01) && (addr & 0xE001) == 0x8001) {
				uint8_t nametable = value >> 7;
				
				if(GetChrMode() == 0) {
					switch(GetCurrentRegister()) {
						case 0:
							SetNametable(0, nametable);
							SetNametable(1, nametable);
							break;

						case 1:
							SetNametable(2, nametable);
							SetNametable(3, nametable);
							break;
					}
				} else {
					switch(GetCurrentRegister()) {
						case 2: SetNametable(0, nametable); break;
						case 3: SetNametable(1, nametable); break;
						case 4: SetNametable(2, nametable); break;
						case 5: SetNametable(3, nametable); break;
					}
				}
			}

			MMC3::WriteRegister(addr, value);
		}
	}
};