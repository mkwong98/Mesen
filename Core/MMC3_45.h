#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_45 : public MMC3
{
private:
	uint8_t _regIndex;

protected:
	uint8_t _reg[4];

	virtual uint16_t RegisterStartAddress() override { return 0x6000; }
	virtual uint16_t RegisterEndAddress() override { return 0xFFFF; }

	virtual void InitMapper() override
	{
		MMC3::InitMapper();
	}

	virtual void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		ArrayInfo<uint8_t> reg = { _reg, 4 };
		Stream(_regIndex, reg);
	}

	virtual void Reset(bool softReset) override
	{
		_regIndex = 0;
		_reg[0] = 0x00;
		_reg[1] = 0x00;
		_reg[2] = 0x0F;
		_reg[3] = 0x00;
		UpdateState();
	}

	virtual void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		uint8_t mask = 0xFF >> (0x0F - (_reg[2] & 0x0F));
		if(!HasChrRam()) {
			page &= mask;
			page |= ((_reg[0] | ((_reg[2] & 0xF0) << 4)) & ~mask);
		}
		MMC3::SelectCHRPage(slot, page, memoryType);
	}

	virtual void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		page &= 0x3F ^ (_reg[3] & 0x3F);
		page |= ((_reg[2] << 2) & 0x300) | _reg[1];
		MMC3::SelectPRGPage(slot, page, memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			WritePrgRam(addr, value);
			if(!(_reg[3] & 0x40)) {
				_reg[_regIndex] = value;
				_regIndex = (_regIndex + 1) & 0x03;
				MMC3::UpdatePrgMapping();
				MMC3::UpdateChrMapping();
				MMC3::UpdateMirroring();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};