#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// NES 2.0 Mapper 319
// UNIF BMC-HP898F

class Mapper319 : public BaseMapper
{
private:
	uint8_t _regs[3];
	bool _unif_BankOrder;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		_regs[0] = _regs[1] = _regs[2] = 0;
		_unif_BankOrder = false;

		if (_romInfo.Hash.PrgCrc32 == 0xC25FD362) {
			_unif_BankOrder = true;
		}

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_regs[0], _regs[1], _regs[2]);
	}

	void UpdateState()
	{
		if(_unif_BankOrder) {
			// The publicly-available UNIF ROM file of Prima Soft 9999999-in-1 has the order of the 16 KiB PRG-ROM banks
			// slightly mixed up, so that the PRG A14 mode bit operates on A16 instead of A14.
			// To obtain the correct bank order,
			// use UNIF 16 KiB PRG banks 0, 4, 1, 5, 2, 6, 3, 7. 
			uint8_t prgReg = (_regs[1] >> 3) & 7;
			uint8_t prgMask = (_regs[1] >> 4) & 4;
			SelectCHRPage(0, (((_regs[0] >> 4) & 0x07) & ~(((_regs[0] & 0x01) << 2) | (_regs[0] & 0x02))));
			SelectPRGPage(0, prgReg & (~prgMask));
			SelectPRGPage(1, prgReg | prgMask);
		} else {
			if(_regs[1] & 0x40) {
				SelectPrgPage2x(0, (_regs[1] >> 2) & 0xE);
			} else {
				uint8_t bank = ((_regs[1] >> 2) & 0x06) | ((_regs[1] >> 5) & 0x01);
				SelectPRGPage(0, bank);
				SelectPRGPage(1, bank);
			}
			SelectCHRPage(0, ((_regs[0] >> 4) & ~((_regs[0] << 2) & 0x04)) | ((_regs[2] << 2) & ((_regs[0] << 2) & 0x04)));
		}

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