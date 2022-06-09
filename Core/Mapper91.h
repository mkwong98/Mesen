#pragma once
#include "stdafx.h"
#include "MMC3.h"

class Mapper91 : public MMC3
{
protected:
	virtual uint16_t RegisterStartAddress() override { return 0x6000; }
	virtual uint16_t RegisterEndAddress() override { return 0x9FFF; }

	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
	virtual uint16_t GetCHRPageSize() override { return 0x800; }

	uint8_t _prgBanks[2];
	uint8_t _chrBanks[4];
	uint8_t _outerBank;

	// for Submapper 1
	bool _verticalMirroring;

	void InitMapper() override
	{
		UpdateState();
	}

	void UpdateState() override
	{
		SelectPRGPage(0, ((_outerBank << 3) & 0x30) | (_prgBanks[0] & 0x0F));
		SelectPRGPage(1, ((_outerBank << 3) & 0x30) | (_prgBanks[1] & 0x0F));
		SelectPRGPage(2, ((_outerBank << 3) & 0x30) | 0xE);
		SelectPRGPage(3, ((_outerBank << 3) & 0x30) | 0xF);

		SelectCHRPage(0, ((_outerBank << 8) & 0x100) | _chrBanks[0]);
		SelectCHRPage(1, ((_outerBank << 8) & 0x100) | _chrBanks[1]);
		SelectCHRPage(2, ((_outerBank << 8) & 0x100) | _chrBanks[2]);
		SelectCHRPage(3, ((_outerBank << 8) & 0x100) | _chrBanks[3]);

		if(_romInfo.SubMapperID == 1) {
			SetMirroringType(_verticalMirroring ? MirroringType::Vertical : MirroringType::Horizontal);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr <= 0x6FFF) {
			if(_romInfo.SubMapperID == 1) {
				switch(addr & 0x07) {
					case 0x00:
					case 0x01:
					case 0x02:
					case 0x03:
						_chrBanks[addr & 0x03] = value;
						UpdateState();
						return;

					case 0x04:
					case 0x05:
						_verticalMirroring = (addr & 0x01) == 0x01;
						UpdateState();
						return;
					
					case 0x06:
						return;

					case 0x07:
						return;
					
					default:
						return;
				}
			} else {
				_chrBanks[addr & 0x03] = value;
				UpdateState();
				return;
			}
		}

		if(addr <= 0x7FFF) {
			switch(addr & 0x03) {
				case 0x00:
				case 0x01:
					_prgBanks[addr & 0x01] = value;
					UpdateState();
					return;

				case 0x02:
					MMC3::WriteRegister(0xE000, value);
					return;

				case 0x03:
					MMC3::WriteRegister(0xC000, 0x07); 
					MMC3::WriteRegister(0xC001, value);
					MMC3::WriteRegister(0xE001, value);
					return;
				
				default:
					return;

			}
		}

		_outerBank = (uint8_t)addr;
		UpdateState();
	}
};