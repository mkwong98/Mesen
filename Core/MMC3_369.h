#pragma once
#include "MMC3.h"

class MMC3_369 : public MMC3
{
private:
	uint8_t _outerBankReg;
	uint8_t _smb2jBank;

	uint16_t _m2Counter;

protected:
	virtual uint32_t GetWorkRamSize() override { return 0x2000; }
	virtual uint32_t GetWorkRamPageSize() override { return 0x2000; }

	virtual bool ForceWorkRamSize() override { return false; }

	void InitMapper() override
	{
		_outerBankReg = 0;
		
		_m2Counter = 0;

		MMC3::InitMapper();

		AddRegisterRange(0x4100, 0x4FFF, MemoryOperation::Write);
	}

	void Reset(bool softReset) override
	{
		if(!softReset) {
			_outerBankReg = 0;
		}

		_m2Counter = 0;

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_outerBankReg, _smb2jBank, _m2Counter);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		switch (_outerBankReg) {
			case 0x00:
				MMC3::SelectChrPage8x(0, 0);
				break;

			case 0x01:
				MMC3::SelectChrPage8x(0, 0x01 << 3);
				break;

			case 0x13:
				MMC3::SelectChrPage8x(0, 0x03 << 3);
				break;

			case 0x37:
			case 0xFF: {
				uint16_t chrBase = (_outerBankReg == 0xFF) ? 0x100 : 0x80;

				MMC3::SelectCHRPage(slot, chrBase | (page & 0x7F));
				break;
			}
		}
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		switch (_outerBankReg) {
			case 0x00:
				MMC3::SelectPrgPage4x(0, 0 << 2);
				break;

			case 0x01:
				MMC3::SelectPrgPage4x(0, 1 << 2);
				break;

			case 0x13:
				SetCpuMemoryMapping(0x6000, 0x7FFF, 0x0E, PrgMemoryType::PrgRom);
				MMC3::SelectPRGPage (0, 0x0C);
				MMC3::SelectPRGPage (1, 0x0D);
				MMC3::SelectPRGPage (2,  8 | (_smb2jBank & 0x03));
				MMC3::SelectPRGPage (3, 0x0F);
				break;
			
			case 0x37:
			case 0xFF: {
				uint8_t prgMask = (_outerBankReg == 0xFF) ? 0x1F : 0x0F;
				uint8_t prgBase = (_outerBankReg == 0xFF) ? 0x20 : 0x10;

				SetCpuMemoryMapping(0x6000, 0x7FFF, 0, PrgMemoryType::WorkRam);
				MMC3::SelectPRGPage(slot, prgBase | (page & prgMask));
				break;
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if((addr & 0xC100) == 0x4100) {
				_outerBankReg = value;
				UpdateState();
			}
		} else {
			if(_outerBankReg == 0x13) {
				switch(addr & 0xF000) {
					case 0x8000:
					case 0x9000:
						_m2Counter = 0;
						_console->GetCpu()->ClearIrqSource(IRQSource::External);
						break;

					case 0xA000:
					case 0xB000:
						_m2Counter = 4096;
						break;

					case 0xE000:
					case 0xF000:
						_smb2jBank = value;
						UpdateState();
						break;
				}
			} else {
				MMC3::WriteRegister(addr, value);
			}
		}
	}

	void ProcessCpuClock() override
	{
		if(_outerBankReg == 0x13) {
			if(_m2Counter > 0) {
				_m2Counter--;
				if(_m2Counter == 0) {
					_console->GetCpu()->SetIrqSource(IRQSource::External);
				}
			}
		}
	}

	void NotifyVRAMAddressChange(uint16_t addr) override
	{
		if(_outerBankReg != 0x13) {
			MMC3::NotifyVRAMAddressChange(addr);
		}
	}
};
