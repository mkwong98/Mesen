#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// NES Mapper 226
// UNIF BMC-Ghostbusters63in1

class Mapper226 : public BaseMapper
{
protected:
	uint8_t _registers[2];

	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_registers[0] = 0;
		_registers[1] = 0;
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_registers[0], _registers[1]);
	}

	void Reset(bool softReset) override
	{
		_registers[0] = 0;
		_registers[1] = 0;

		UpdateState();
	}

	uint8_t GetPrgPage()
	{
		const uint8_t banks[4] = { 0, 0, 1, 2 };
		uint8_t base = ((_registers[0] & 0x80) >> 7) | ((_registers[1] & 0x01) << 1);

		if(_prgSize == (1536 * 1024)) { // for 1536 KB PRG roms / BMC-Ghostbusters63in1
			base = banks[base];
		}

		return (_registers[0] & 0x1F) | (base << 5);
	}

	void UpdateState()
	{
		uint8_t prgPage = GetPrgPage();

		if(_registers[0] & 0x20) {
			SelectPRGPage(0, prgPage);
			SelectPRGPage(1, prgPage);
		} else {
			SelectPRGPage(0, prgPage & 0xFE);
			SelectPRGPage(1, (prgPage & 0xFE) + 1);
		}

		SelectCHRPage(0, 0);

		SetMirroringType(_registers[0] & 0x40 ? MirroringType::Vertical : MirroringType::Horizontal);
	
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_registers[addr & 0x01] = value;

		UpdateState();
	}
};
