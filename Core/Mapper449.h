#pragma once
#include "BaseMapper.h"

class Mapper449 : public BaseMapper
{
protected:
	uint32_t GetDipSwitchCount() override { return 4; }
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	bool AllowRegisterRead() override { return true; }

	bool _mFlag;

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	void Reset(bool softreset) override
	{
		WriteRegister(0x8000, 0);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_mFlag);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if(_mFlag) {
			addr |= GetDipSwitches();
		}
		return InternalReadRam(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint16_t prgBank = ((addr >> 2) & 0x1F) | ((addr >> 3) & 0x20);
		bool sFlag = (addr & 0x01) == 0x01;
		bool prgMode = ((addr >> 7) & 0x01) == 0x01;

		_mFlag = ((addr >> 9) & 0x01) == 0x01;

		if(prgMode) {
			if(sFlag) {
				SelectPrgPage2x(0, prgBank & 0xFE);
			} else {
				SelectPRGPage(0, prgBank);
				SelectPRGPage(1, prgBank);
			}
		} else {
			SelectPRGPage(0, prgBank);
			SelectPRGPage(1, prgBank | 0x07);
		}

		// protect CHR-RAM on nrom modes
		SetPpuMemoryMapping(0, 0x1FFF, 0, ChrMemoryType::Default, (!HasBattery() && prgMode) ? MemoryAccessType::Read : MemoryAccessType::ReadWrite);

		SelectCHRPage(0, value);

		SetMirroringType((addr & 0x02) == 0x02 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};