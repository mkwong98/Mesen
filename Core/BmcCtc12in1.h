#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class BmcCtc12in1 : public BaseMapper
{
private:
	uint8_t _reg;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectCHRPage(0, 0);
	}

	void Reset(bool softReset) override
	{
		if(!softReset) {
			_reg = 0;
		}
		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_reg);
	}

	void UpdateState() {
		SetCpuMemoryMapping(0x6000, 0x7FFF, PrgMemoryType::PrgRom, 0x2000, MemoryAccessType::Read);
		SetPpuMemoryMapping(0, 0x1FFF, 0, ChrMemoryType::Default, (_reg & 0x80) ? MemoryAccessType::ReadWrite : MemoryAccessType::Read);

		switch((_reg & 0xC0) >> 6) {
			case 0:		// NROM-128
				SelectPRGPage(0, _reg & 0x1F);
				SelectPRGPage(1, _reg & 0x1F);
				break;

			case 1:		// NROM-256
				SelectPrgPage2x(0, _reg & 0x1E);
				break;

			default:	// UNROM
				SelectPRGPage(0, _reg & 0x3F);
				SelectPRGPage(1, (_reg & 0x3F) | 7);
				break;
		}
		SetMirroringType(_reg & 0x20 ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0xC000) {
			_reg &= 0x07;
			_reg |= value & ~0x07;
		} else {
			_reg &= _reg & ~0x07;
			_reg |= value & 0x07;
		}
		UpdateState();
	}
};