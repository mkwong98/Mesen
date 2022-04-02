#pragma once
#include "BaseMapper.h"

class Mapper375 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	uint16_t latchea;
	uint8_t latched;

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	virtual void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(latchea, latched);
	}

	void UpdateState()
	{
		uint16_t prgBank = ((latchea >> 2) & 0x1F) | ((latchea >> 3) & 0x20) | ((latchea >> 4) & 0x40);
		bool sFlag = (latchea & 0x01) == 0x01;
		bool lFlag = ((latchea >> 9) & 0x01) == 0x01;
		bool prgMode = ((latchea >> 7) & 0x01) == 0x01;
		uint16_t p0 = prgBank;

		if((latchea & 0x800) == 0x800)
			p0 = (prgBank & ~0x07) | (latched & 0x07);

		if (prgMode) {
			if (sFlag) {
				SelectPrgPage2x(0, prgBank & 0xFE);
			} else {
				SelectPRGPage(0, p0);
				SelectPRGPage(1, prgBank);
			}
		} else {
			if (sFlag) {
				if (lFlag) {
					SelectPRGPage(0, p0 & 0x7E);
					SelectPRGPage(1, prgBank | 0x07);
				} else {
					SelectPRGPage(0, p0 & 0x7E);
					SelectPRGPage(1, prgBank & 0x78);
				}
			} else {
				if (lFlag) {
					SelectPRGPage(0, p0);
					SelectPRGPage(1, prgBank | 0x07);
				} else {
					SelectPRGPage(0, p0);
					SelectPRGPage(1, prgBank & 0x78);
				}
			}
		}

		// protect CHR-RAM on nrom modes
		SetPpuMemoryMapping(0, 0x1FFF, 0, ChrMemoryType::Default, (latchea & 0x80) ? MemoryAccessType::Read : MemoryAccessType::ReadWrite);

		SetMirroringType((latchea & 0x02) == 0x02 ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(latchea & 0x800) {
			latched = value;
		} else {
			latchea = addr;
			latched = value;
		}
		UpdateState();
	}
};