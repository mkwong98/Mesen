#include "stdafx.h"
#include <random>
#include "HdPpu.h"
#include "CPU.h"
#include "Console.h"
#include "HdNesPack.h"
#include "VideoDecoder.h"
#include "RewindManager.h"
#include "HdPackConditions.h"
#include "NotificationManager.h"
#include "BaseMapper.h"
#include "MemoryManager.h"

void HdPpu::WriteRAM(uint16_t addr, uint8_t value)
{
	if (addr >= 0x2000 && addr < 0x3000)
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<> dist(0, 1);
		_nameTableRandVal[addr & 0xFFF] = dist(mt);
	}
	PPU::WriteRAM(addr, value);
}


void HdPpu::DrawPixel()
{
	uint16_t bufferOffset = (_scanline << 8) + _cycle - 1;
	uint16_t &pixel = _currentOutputBuffer[bufferOffset];
	_lastSprite = nullptr;

	if (bufferOffset == 0) {
		//init sprite frame ranges at screen start
		uint8_t spriteCnt = (_flags.LargeSprites ? 128 : 64);
		for (int i = 0; i < spriteCnt; i++) {
			_spriteFrameRanges[i].lastUpdated = _spriteFrameRanges[i].updated;
			if (_spriteFrameRanges[i].updated) {
				//copy last frame sprite data
				memcpy(&(_spriteFrameRanges[i].last), &(_spriteFrameRanges[i].current), sizeof(_spriteFrameRanges[i].current));
				_spriteFrameRanges[i].updated = false;
				_spriteFrameRanges[i].lastStartFrameNumber = _spriteFrameRanges[i].startFrameNumber;
			}
		}

		//init sprite addition counter
		if (_hdData->Additions.size() > 0) {
			memset(_info->AdditionCount, 0, PPU::PixelCount);
		}
	}
	
	if(IsRenderingEnabled() || ((_state.VideoRamAddr & 0x3F00) != 0x3F00)) {
		bool isChrRam = !_console->GetMapper()->HasChrRom();
		BaseMapper *mapper = _console->GetMapper();

		uint32_t color = GetPixelColor();
		pixel = (_paletteRAM[color & 0x03 ? color : 0] & _paletteRamMask) | _intensifyColorBits;

		TileInfo* lastTile = &((_state.XScroll + ((_cycle - 1) & 0x07) < 8) ? _previousTile : _currentTile);
		uint32_t backgroundColor = 0;
		if(_flags.BackgroundEnabled && _cycle > _minimumDrawBgCycle) {
			backgroundColor = (((_state.LowBitShift << _state.XScroll) & 0x8000) >> 15) | (((_state.HighBitShift << _state.XScroll) & 0x8000) >> 14);
		}

		HdPpuPixelInfo &tileInfo = _info->ScreenTiles[bufferOffset];

		tileInfo.Grayscale = _paletteRamMask == 0x30;
		tileInfo.EmphasisBits = _intensifyColorBits >> 6;
		tileInfo.Tile.PpuBackgroundColor = ReadPaletteRAM(0);
		tileInfo.Tile.BgColorIndex = backgroundColor;
		tileInfo.Tile.nameTableIdx = _state.VideoRamAddr;

		if(backgroundColor == 0) {
			tileInfo.Tile.BgColor = tileInfo.Tile.PpuBackgroundColor;
		} else {
			tileInfo.Tile.BgColor = ReadPaletteRAM(lastTile->PaletteOffset + backgroundColor);
		}
		tileInfo.XScroll = _state.XScroll;
		tileInfo.TmpVideoRamAddr = _state.TmpVideoRamAddr;

		if(_lastSprite && _flags.SpritesEnabled) {
			int j = 0;
			for(uint8_t i = 0; i < _spriteCount; i++) {
				int32_t shift = (int32_t)_cycle - _spriteTiles[i].SpriteX - 1;
				SpriteInfo& sprite = _spriteTiles[i];
				if(shift >= 0 && shift < 8) {
					tileInfo.Sprite[j].TileIndex = sprite.AbsoluteTileAddr / 16;
					if(isChrRam) {
						mapper->CopyChrTile(sprite.AbsoluteTileAddr & 0xFFFFFFF0, tileInfo.Sprite[j].TileData);
					}
					if(_version >= 100) {
						tileInfo.Sprite[j].PaletteColors = 0xFF000000 | _paletteRAM[sprite.PaletteOffset + 3] | (_paletteRAM[sprite.PaletteOffset + 2] << 8) | (_paletteRAM[sprite.PaletteOffset + 1] << 16);
					} else {
						tileInfo.Sprite[j].PaletteColors = _paletteRAM[sprite.PaletteOffset + 3] | (_paletteRAM[sprite.PaletteOffset + 2] << 8) | (_paletteRAM[sprite.PaletteOffset + 1] << 16);
					}
					if(sprite.OffsetY >= 8) {
						tileInfo.Sprite[j].OffsetY = sprite.OffsetY - 8;
					} else {
						tileInfo.Sprite[j].OffsetY = sprite.OffsetY;
					}

					tileInfo.Sprite[j].OffsetX = shift;
					tileInfo.Sprite[j].HorizontalMirroring = sprite.HorizontalMirror;
					tileInfo.Sprite[j].VerticalMirroring = sprite.VerticalMirror;
					tileInfo.Sprite[j].BackgroundPriority = sprite.BackgroundPriority;

					int32_t shift = (int32_t)_cycle - sprite.SpriteX - 1;
					if(sprite.HorizontalMirror) {
						tileInfo.Sprite[j].SpriteColorIndex = ((sprite.LowByte >> shift) & 0x01) | ((sprite.HighByte >> shift) & 0x01) << 1;
					} else {
						tileInfo.Sprite[j].SpriteColorIndex = ((sprite.LowByte << shift) & 0x80) >> 7 | ((sprite.HighByte << shift) & 0x80) >> 6;
					}

					if(tileInfo.Sprite[j].SpriteColorIndex == 0) {
						tileInfo.Sprite[j].SpriteColor = ReadPaletteRAM(0);
					} else {
						tileInfo.Sprite[j].SpriteColor = ReadPaletteRAM(sprite.PaletteOffset + tileInfo.Sprite[j].SpriteColorIndex);
					}
					tileInfo.Sprite[j].PaletteOffset = sprite.PaletteOffset;

					tileInfo.Sprite[j].PpuBackgroundColor = tileInfo.Tile.PpuBackgroundColor;
					tileInfo.Sprite[j].BgColorIndex = tileInfo.Tile.BgColorIndex;
					tileInfo.Sprite[j].OAMIndex = (sprite.OffsetY >= 8 ? sprite.OAMIndex + 64 : sprite.OAMIndex);
					
					if (!_spriteFrameRanges[tileInfo.Sprite[j].OAMIndex].updated) {
						//fill the current frame sprite
						memcpy(_spriteFrameRanges[tileInfo.Sprite[j].OAMIndex].current.TileData, tileInfo.Sprite[j].TileData, 16);
						_spriteFrameRanges[tileInfo.Sprite[j].OAMIndex].current.TileIndex = tileInfo.Sprite[j].TileIndex;
						_spriteFrameRanges[tileInfo.Sprite[j].OAMIndex].current.PaletteColors = tileInfo.Sprite[j].PaletteColors;
						_spriteFrameRanges[tileInfo.Sprite[j].OAMIndex].current.HorizontalMirroring = tileInfo.Sprite[j].HorizontalMirroring;
						_spriteFrameRanges[tileInfo.Sprite[j].OAMIndex].current.VerticalMirroring = tileInfo.Sprite[j].VerticalMirroring;
						_spriteFrameRanges[tileInfo.Sprite[j].OAMIndex].current.BackgroundPriority = tileInfo.Sprite[j].BackgroundPriority;
						_spriteFrameRanges[tileInfo.Sprite[j].OAMIndex].current.ScreenX = sprite.SpriteX;
						_spriteFrameRanges[tileInfo.Sprite[j].OAMIndex].current.ScreenY = _scanline - (tileInfo.Sprite[j].VerticalMirroring ? 7 - tileInfo.Sprite[j].OffsetY : tileInfo.Sprite[j].OffsetY);
						_spriteFrameRanges[tileInfo.Sprite[j].OAMIndex].updated = true;
					}

					j++;
					if(j >= 4) {
						break;
					}
				}
			}
			tileInfo.SpriteCount = j;
		} else {
			tileInfo.SpriteCount = 0;
		}

		if(_flags.BackgroundEnabled && _cycle > _minimumDrawBgCycle) {
			tileInfo.Tile.TileIndex = lastTile->AbsoluteTileAddr / 16;
			if(isChrRam) {
				mapper->CopyChrTile(lastTile->AbsoluteTileAddr & 0xFFFFFFF0, tileInfo.Tile.TileData);
			}
			if(_version >= 100) {
				tileInfo.Tile.PaletteColors = _paletteRAM[lastTile->PaletteOffset + 3] | (_paletteRAM[lastTile->PaletteOffset + 2] << 8) | (_paletteRAM[lastTile->PaletteOffset + 1] << 16) | (_paletteRAM[0] << 24);
			} else {
				tileInfo.Tile.PaletteColors = _paletteRAM[lastTile->PaletteOffset + 3] | (_paletteRAM[lastTile->PaletteOffset + 2] << 8) | (_paletteRAM[lastTile->PaletteOffset + 1] << 16);
			}
			tileInfo.Tile.OffsetY = lastTile->OffsetY;
			tileInfo.Tile.OffsetX = (_state.XScroll + ((_cycle - 1) & 0x07)) & 0x07;
		} else {
			tileInfo.Tile.TileIndex = HdPpuTileInfo::NoTile;
		}
		
		if (bufferOffset == PixelCount - 1) {
			//match sprite frame range at screen end
			uint16_t distance;
			uint16_t newDistance;
			uint16_t newDistanceX;
			uint16_t newDistanceY;
			uint8_t spriteCnt = (_flags.LargeSprites ? 128 : 64);

			for (int i = 0; i < spriteCnt; i++) {
				if (_spriteFrameRanges[i].updated) {
					distance = 13;
					_spriteFrameRanges[i].startFrameNumber = _frameCount;
					for (int j = 0; j < spriteCnt; j++) {
						if (_spriteFrameRanges[j].lastUpdated) {
							newDistanceX = abs(_spriteFrameRanges[i].current.ScreenX - _spriteFrameRanges[j].last.ScreenX);
							newDistanceY = abs(_spriteFrameRanges[i].current.ScreenY - _spriteFrameRanges[j].last.ScreenY);
							newDistance = newDistanceX + newDistanceY;
							if (newDistance < distance && newDistanceX <= 6 && newDistanceY <= 6) {
								//check for matches
								bool compareResult = false;
								if (_spriteFrameRanges[i].current.BackgroundPriority == _spriteFrameRanges[j].last.BackgroundPriority
									&& _spriteFrameRanges[i].current.HorizontalMirroring == _spriteFrameRanges[j].last.HorizontalMirroring
									&& _spriteFrameRanges[i].current.VerticalMirroring == _spriteFrameRanges[j].last.VerticalMirroring
									&& _spriteFrameRanges[i].current.PaletteColors == _spriteFrameRanges[j].last.PaletteColors
									) {
									if (isChrRam) {
										compareResult = (memcmp(_spriteFrameRanges[i].current.TileData, _spriteFrameRanges[j].last.TileData, 16) == 0);
									}
									else {
										compareResult = (_spriteFrameRanges[i].current.TileIndex == _spriteFrameRanges[j].last.TileIndex);
									}
								}
								if (compareResult) {
									_spriteFrameRanges[i].startFrameNumber = _spriteFrameRanges[j].lastStartFrameNumber;
								}
								distance = newDistance;
							}							
						}
					}
				}
			}

			//check for valid addition
			for (uint8_t j = 0; j < spriteCnt; j++) {
				uint8_t i = (_flags.LargeSprites ? (j / 2) + ((j % 2) ? 64 : 0) : j);
				if (_spriteFrameRanges[i].updated) {
					for (uint8_t k = 0; k < _hdData->Additions.size(); k++) {
						if (*(_hdData->Additions[k]) == _spriteFrameRanges[i].current) {
							int16_t addX = _spriteFrameRanges[i].current.ScreenX + (_spriteFrameRanges[i].current.HorizontalMirroring ? -_hdData->Additions[k]->OffsetX : _hdData->Additions[k]->OffsetX);
							int16_t addY = _spriteFrameRanges[i].current.ScreenY + (_spriteFrameRanges[i].current.VerticalMirroring ? -_hdData->Additions[k]->OffsetY : _hdData->Additions[k]->OffsetY);

							HdPpuTileInfo addSp;
							addSp.HorizontalMirroring = _spriteFrameRanges[i].current.HorizontalMirroring;
							addSp.VerticalMirroring = _spriteFrameRanges[i].current.VerticalMirroring;
							addSp.BackgroundPriority = _spriteFrameRanges[i].current.BackgroundPriority;
							addSp.PaletteColors = _hdData->Additions[k]->additionSpr.PaletteColors;
							addSp.IsChrRamTile = isChrRam;
							memcpy(addSp.TileData, _hdData->Additions[k]->additionSpr.TileData, 16);
							addSp.TileIndex = _hdData->Additions[k]->additionSpr.TileIndex;
							addSp.OAMIndex = i;

							for (uint8_t px = 0; px < 8; px++) {
								for (uint8_t py = 0; py < 8; py++) {
									int16_t rx = addX + px;
									int16_t ry = addY + py;
									if (rx >= 0 && rx < 256 && ry >= 0 && ry < 240) {
										uint16_t addBufferOffset = (ry << 8) + rx;
										if (_info->AdditionCount[addBufferOffset] < 4) {
											HdPpuPixelInfo& addTileInfo = _info->ScreenTiles[addBufferOffset];
											addTileInfo.SprAddition[_info->AdditionCount[addBufferOffset]] = addSp;
											addTileInfo.SprAddition[_info->AdditionCount[addBufferOffset]].OffsetX = px;
											addTileInfo.SprAddition[_info->AdditionCount[addBufferOffset]].OffsetY = (_spriteFrameRanges[i].current.VerticalMirroring ? 7-py : py);
											_info->AdditionCount[addBufferOffset]++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	} else {
		//"If the current VRAM address points in the range $3F00-$3FFF during forced blanking, the color indicated by this palette location will be shown on screen instead of the backdrop color."
		pixel = ReadPaletteRAM(_state.VideoRamAddr) | _intensifyColorBits;
		_info->ScreenTiles[bufferOffset].Tile.TileIndex = HdPpuTileInfo::NoTile;
		_info->ScreenTiles[bufferOffset].SpriteCount = 0;
	}
}

HdPpu::HdPpu(shared_ptr<Console> console, HdPackData * hdData) : PPU(console)
{
	_hdData = hdData;

	if(_hdData) {
		_version = _hdData->Version;

		bool isChrRamGame = !console->GetMapper()->HasChrRom();
		_screenInfo[0] = new HdScreenInfo(isChrRamGame);
		_screenInfo[1] = new HdScreenInfo(isChrRamGame);
		_info = _screenInfo[0];
	}
}

HdPpu::~HdPpu()
{
	if(_hdData) {
		delete _screenInfo[0];
		delete _screenInfo[1];
	}
}

void HdPpu::SendFrame()
{
	_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone, _currentOutputBuffer);

	_info->FrameNumber = _frameCount;
	_info->spriteFrameRanges = _spriteFrameRanges;
	_info->nameTableRandVal = _nameTableRandVal;
	_info->WatchedAddressValues.clear();
	for(uint32_t address : _hdData->WatchedMemoryAddresses) {
		if(address & HdPackBaseMemoryCondition::PpuMemoryMarker) {
			if((address & 0x3FFF) >= 0x3F00) {
				_info->WatchedAddressValues[address] = ReadPaletteRAM(address);
			} else {
				_info->WatchedAddressValues[address] = _console->GetMapper()->DebugReadVRAM(address & 0x3FFF, true);
			}
		} else {
			_info->WatchedAddressValues[address] = _console->GetMemoryManager()->DebugRead(address);
		}
	}

#ifdef  LIBRETRO
	_console->GetVideoDecoder()->UpdateFrameSync(_currentOutputBuffer, _info);
#else
	if(_console->GetRewindManager()->IsRewinding()) {
		_console->GetVideoDecoder()->UpdateFrameSync(_currentOutputBuffer, _info);
	} else {
		_console->GetVideoDecoder()->UpdateFrame(_currentOutputBuffer, _info);
	}
	_currentOutputBuffer = (_currentOutputBuffer == _outputBuffers[0]) ? _outputBuffers[1] : _outputBuffers[0];
	_info = (_info == _screenInfo[0]) ? _screenInfo[1] : _screenInfo[0];
#endif
}
