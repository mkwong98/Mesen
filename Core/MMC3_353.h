#pragma once
#include "MMC3.h"

class MMC3_353 : public MMC3
{
protected:
	uint16_t GetChrRamPageSize() override { return 0x2000; }
	uint32_t GetChrRamSize() override { return 0x2000; }

	uint8_t _exReg;

	void InitMapper() override
	{
		_exReg = 0;
		MMC3::InitMapper();
	}

	void Reset(bool softreset) override
	{
		_exReg = 0;
		MMC3::UpdateState();
	}

	void UpdateMirroring() override
	{
		if(!(_exReg == 0)) {
			MMC3::UpdateMirroring();
		}
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		if((_exReg == 2) && (MMC3::_registers[0] & 0x80)) {
			MMC3::SelectChrPage8x(0, 0, ChrMemoryType::ChrRam);
		} else {
			MMC3::SelectCHRPage(slot, (_exReg << 7) | (page & 0x7F), memoryType);
		}
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(_exReg == 2) {
			page &= 0x0F;
			page |= ((MMC3::_registers[0] >> 3) & 0x10);
			page |= (_exReg << 5);
		} else {
			page &= 0x1F;
			page |= (_exReg << 5);
		}

		MMC3::SelectPRGPage(slot, page);

		if((_exReg == 3) && !(MMC3::_registers[0] & 0x80)) {
			MMC3::SelectPRGPage(2, MMC3::_registers[6] | 0x70);
			MMC3::SelectPRGPage(3, MMC3::_registers[7] | 0x70);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if((addr & 0x80) == 0x80) {
			_exReg = (addr >> 13) & 0x03;
			UpdatePrgMapping();
			UpdateChrMapping();
		} else {
			if((_exReg == 0) && (addr & 0xE001) == 0x8001) {
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