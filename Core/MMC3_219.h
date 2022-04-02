#pragma once
#include "MMC3.h"

class MMC3_219 : public MMC3
{
private:
	uint8_t _exRegs[3];

protected:
	void InitMapper() override
	{
		MMC3::InitMapper();
		AddRegisterRange(0x5000, 0x5FFF);
	}

	void Reset(bool softreset) override
	{
		_exRegs[0] = _exRegs[2] = 0x00;
		_exRegs[1] = 0x03;
		MMC3::UpdateState();
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		MMC3::SelectCHRPage(slot, (_exRegs[1] << 7) | (page & 0x7F));
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		MMC3::SelectPRGPage(slot, (_exRegs[1] << 4) | (page & 0x0F));
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			switch(addr & 0x01) {
				case 0: _exRegs[1] = (_exRegs[1] & ~0x01) | ((value >> 3) & 0x01); break;
				case 1: _exRegs[1] = (_exRegs[1] & ~0x02) | ((value >> 4) & 0x02); break;
			}
			MMC3::UpdatePrgMapping();
			MMC3::UpdateChrMapping();
		} else if(addr < 0xA000) {
			switch(addr & 0xE003) {
			case 0x8000:
				MMC3::WriteRegister(addr, value);
				break;
			
			case 0x8002:
				_exRegs[0] = value;
				MMC3::WriteRegister(addr, value);
				break;

			case 0x8001:
				if(_exRegs[0] & 0x20) { // Extended Mode
					uint8_t bankRegister = MMC3::GetState().Reg8000;
					if((bankRegister >= 0x25) && (bankRegister <= 0x26)) {
						uint8_t prgBank = ((value & 0x20) >> 5) | ((value & 0x10) >> 3) | ((value & 0x08) >> 1) | ((value & 0x04) << 1);
						MMC3::_registers[6 | (bankRegister & 0x01)] = prgBank;
						MMC3::UpdatePrgMapping();
					} else if((bankRegister >= 0x08) && (bankRegister <= 0x1F)) {
						switch(bankRegister) {
							case 0x08: case 0x0A: case 0x0E: case 0x12: case 0x16: case 0x1A: case 0x1E: 
								_exRegs[2] = value << 4; 
								break;

							case 0x09: MMC3::_registers[0] = _exRegs[2] | (value >> 1 & 0x0E); break;
							case 0x0B: MMC3::_registers[0] = _exRegs[2] | (value >> 1 | 0x1); break;
							case 0x0C:
							case 0x0D: MMC3::_registers[1] = _exRegs[2] | (value >> 1 & 0xE); break;
							case 0x0F: MMC3::_registers[1] = _exRegs[2] | (value >> 1 | 0x1); break;
							case 0x10:
							case 0x11: MMC3::_registers[2] = _exRegs[2] | (value >> 1 & 0xF); break;
							case 0x14:
							case 0x15: MMC3::_registers[3] = _exRegs[2] | (value >> 1 & 0xF); break;
							case 0x18:
							case 0x19: MMC3::_registers[4] = _exRegs[2] | (value >> 1 & 0xF); break;
							case 0x1C:
							case 0x1D: MMC3::_registers[5] = _exRegs[2] | (value >> 1 & 0xF); break;
						}
						MMC3::UpdateChrMapping();
					}
				} else { // Normal MMC3
					MMC3::WriteRegister(addr, value);
				}
				break;
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};