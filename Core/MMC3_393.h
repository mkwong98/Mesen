#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_393 : public MMC3
{
private:
	uint8_t _exReg[2];

protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	uint16_t GetChrRamPageSize() override { return 0x400; }
	uint32_t GetChrRamSize() override { return 0x2000; }

	void InitMapper() override
	{
		MMC3::InitMapper();
	}

	void Reset(bool softreset) override
	{
		_exReg[0] = _exReg[1] = 0;
		MMC3::UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_exReg[0], _exReg[1]);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(_exReg[0] & 0x08) {
			MMC3::SelectChrPage8x(0, 0, ChrMemoryType::ChrRam);
		} else {
			MMC3::SelectCHRPage(slot, (_exReg[0] << 8) | page, memoryType);
		}
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(_exReg[0] & 0x20) {
			if(_exReg[0] & 0x10) {
				MMC3::SelectPrgPage2x(0, ((_exReg[0] << 3) | (_exReg[1] & 0x07)) << 1);
				MMC3::SelectPrgPage2x(1, ((_exReg[0] << 3) | 0x07) << 1);
			} else {
				page = (MMC3::_registers[6] >> 2) & 3;
				SelectPrgPage4x(0, ((_exReg[0] << 2) | page) << 2);
			}
		} else {
			MMC3::SelectPRGPage(slot, (_exReg[0] << 4) | (page & 0x0F));
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_exReg[0] = (uint8_t)addr;
			MMC3::UpdatePrgMapping();
			MMC3::UpdateChrMapping();
		} else {
			MMC3::WriteRegister(addr, value);
			if(_exReg[1] != value) {
				_exReg[1] = value;
				MMC3::UpdatePrgMapping();
			}
		}
	}
};