#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper319 : public BaseMapper
{
private:
	uint8_t _regs[3];

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		_regs[0] = _regs[1] = _regs[2] = 0;
		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_regs[0], _regs[1], _regs[2]);
	}

	void UpdateState()
	{
        if(_regs[1] & 0x40) {
            SelectPrgPage2x(0, (_regs[1] >> 2) & 0xE);
        } else {
			uint8_t bank = ((_regs[1] >> 2) & 0x06) | ((_regs[1] >> 5) & 0x01);
            SelectPRGPage(0, bank);
		    SelectPRGPage(1, bank);
        }
		SelectCHRPage(0, ((_regs[0] >> 4) & ~((_regs[0] << 2) & 0x04)) | ((_regs[2] << 2) & ((_regs[0] << 2) & 0x04)));
		SetMirroringType(_regs[1] & 0x80 ? MirroringType::Vertical : MirroringType::Horizontal);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_regs[(addr & 0x04) >> 2] = value;
			UpdateState();
		} else {
			_regs[2] = value;
			UpdateState();
		}
	}
};