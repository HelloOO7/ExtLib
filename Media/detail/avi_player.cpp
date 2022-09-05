/*
 * Bermuda Syndrome engine rewrite
 * Copyright (C) 2007-2011 Gregory Montoir
 */

#include "avi_player.h"
#include "IO/exl_Stream.h"
#include "Heap/exl_Allocator.h"
#include "exl_DebugPrint.h"

#include "string.h"

INLINE uint16_t READ_LE_UINT16(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[1] << 8) | b[0];
}

INLINE uint32_t READ_LE_UINT32(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
}

AVI_Demuxer::AVI_Demuxer(exl::heap::Allocator* allocator) {
    _allocator = allocator;
}

bool AVI_Demuxer::open(exl::io::Stream *f) {
	_f = f;
	_recordsListSize = 0;
	_chunkData = 0;
	_chunkDataSize = 0;
	return _f != 0 && readHeader();
}

void AVI_Demuxer::close() {
	_f = 0;
	exl::heap::Allocator::FreeStatic(_chunkData);
	_chunkData = 0;
}

bool AVI_Demuxer::readHeader_avih() {
	uint8_t hdr[kSizeOfChunk_avih];
	_f->Read(hdr, sizeof(char), kSizeOfChunk_avih);
	_frameRate = 1000000 / READ_LE_UINT32(hdr);
	_frames = READ_LE_UINT32(hdr + 16);
	_streams = READ_LE_UINT32(hdr + 24);
	_width = READ_LE_UINT32(hdr + 32);
	_height = READ_LE_UINT32(hdr + 36);
	EXL_DEBUG_PRINTF("AviHdr streams %d, width %d, height %d, framerate %d, frames %d\n", _streams, _width, _height, _frameRate, _frames);
	return _streams == 2;
}

bool AVI_Demuxer::readHeader_strh() {
	uint8_t hdr[kSizeOfChunk_strh];
	_f->Read(hdr, sizeof(char), kSizeOfChunk_strh);
	if (memcmp(hdr, "auds", 4) == 0) {
		if (READ_LE_UINT32(hdr + 4) == 0) {
			_audioBufferSize = READ_LE_UINT32(hdr + 36);
		}
		else {
			_audioBufferSize = 0;
		}
		return true;
	}
	if (memcmp(hdr, "vids", 4) == 0 && memcmp(hdr + 4, "cvid", 4) == 0) {
		_videoBufferSize = READ_LE_UINT32(hdr + 36);
		return true;
	}
	return false;
}

bool AVI_Demuxer::readHeader_strf_auds() {
	uint8_t hdr[kSizeOfChunk_waveformat];
	_f->Read(hdr, sizeof(char), kSizeOfChunk_waveformat);
	int formatTag = READ_LE_UINT16(hdr);
	int channels = READ_LE_UINT16(hdr + 2);
	int sampleRate  = READ_LE_UINT32(hdr + 4);
	int bitsPerSample = READ_LE_UINT16(hdr + 14);
	return formatTag == 1 && channels == 1 && sampleRate == 44100 && bitsPerSample == 8;
}

bool AVI_Demuxer::readHeader_strf_vids() {
	uint8_t hdr[kSizeOfChunk_bitmapinfo];
	_f->Read(hdr, sizeof(char), kSizeOfChunk_bitmapinfo);
	int width = READ_LE_UINT32(hdr + 4);
	int height = READ_LE_UINT32(hdr + 8);
	int planes = READ_LE_UINT16(hdr + 12);
	int bitDepth = READ_LE_UINT16(hdr + 14);
	EXL_DEBUG_PRINTF("vids strf: w: %d, h: %d, planes: %d, bitDepth :%d\n", width, height, planes, bitDepth);
	return width == _width && height == _height && planes == 1 && bitDepth == 24;
}

bool AVI_Demuxer::readHeader() {
	_frames = 0;
	_width = _height = 0;
	_streams = 0;
	_frameRate = 0;
	_mediaBeginOfs = 0;

	char tag[5];
	tag[4] = 0;
	if (!_f->SeekSet(8)) {
		EXL_DEBUG_PRINTF("Could not seek to AVI header.\n");
		return false;
	} // skip RIFF header
	_f->Read(tag, sizeof(char), 4);
	if (memcmp(tag, "AVI ", 4) == 0) {
		bool readHdrLoop = true;
		while (readHdrLoop) {
        	_f->Read(tag, sizeof(char), 4);
			int len;
            _f->Read(&len, sizeof(int), 1);
			if (memcmp(tag, "LIST", 4) == 0) {
				_f->Read(tag, sizeof(char), 4);
				if (memcmp(tag, "movi", 4) == 0) {
					_chunkDataSize = ((_videoBufferSize > _audioBufferSize ? _videoBufferSize : _audioBufferSize) + 3) & 0xFFFFFFFC;
					_chunkData = static_cast<uint8_t*>(_allocator->Alloc(_chunkDataSize));
					if (!_chunkData) {
						//warning("Unable to allocate %d bytes", _chunkDataSize);
						return false;
					}
					EXL_DEBUG_PRINTF("Header reading done. Video buffer size 0x%x, audio 0x%x\n", _videoBufferSize, _audioBufferSize);
					_mediaBeginOfs = _f->Tell();
					return true;
				}
			} else if (memcmp(tag, "avih", 4) == 0 && len == kSizeOfChunk_avih) {
				readHdrLoop = readHeader_avih();
			} else if (memcmp(tag, "strh", 4) == 0 && len == kSizeOfChunk_strh) {
				readHdrLoop = readHeader_strh();
			} else if (memcmp(tag, "strf", 4) == 0 && len == kSizeOfChunk_waveformat) {
				readHdrLoop = readHeader_strf_auds();
			} else if (memcmp(tag, "strf", 4) == 0 && len == kSizeOfChunk_bitmapinfo) {
				readHdrLoop = readHeader_strf_vids();
			} else {
				_f->SeekCur(len);
			}
		}
	}
	else {
		EXL_DEBUG_PRINTF("AVI signature wrong! (got: %.4s)\n", tag);
	}
	return false;
}

bool AVI_Demuxer::readNextChunk(AVI_Chunk &chunk) {
	//EXL_DEBUG_PRINTF("Chunk begin stream pos %x\n", _f->Tell());
	char tag[8];
	//assert(_recordsListSize >= 0);
	/*if (_recordsListSize == 0) {
		_f->Read(tag, sizeof(char), 12); // 'LIST', size, 'rec '
		_recordsListSize = READ_LE_UINT32(tag + 4) - 4;
	}*/
	_f->Read(tag, sizeof(char), 8);
	int len = READ_LE_UINT32(tag + 4);
	len = (len + 1) & ~1;
	_recordsListSize -= len + 8;
	if (tag[2] == 'w' && tag[3] == 'b') {
		chunk.type = kChunkAudioType;
	} else if (tag[2] == 'd' && tag[3] == 'c') {
		chunk.type = kChunkVideoType;
	} else {
		EXL_DEBUG_PRINTF("Unknown chunk tag %.4s at stream pos %x\n", tag, _f->Tell() - 8);
		_f->SeekCur(len);
		return false;
	}
	//EXL_DEBUG_PRINTF("Chunk %.4s\n", tag);
	//assert(len <= _chunkDataSize);
	chunk.dataSize = len;
	return true;
}

Cinepak_Decoder::Cinepak_Decoder(exl::heap::Allocator* allocator) {
	_allocator = allocator;
	codebookV1.stripVectors = nullptr;
	codebookV4.stripVectors = nullptr;
	stripVectorCount = 0;
}

void Cinepak_Decoder::allocYUV(int width, int height) {
	_yuvPitch = width * 2;
	u32 allocSize = width * height * 2;
	_yuvFrame = static_cast<uint8_t*>(_allocator->Alloc(allocSize));
}

static void SET_YUV_V4(uint8_t *dst, uint8_t y1, uint8_t y2, uint8_t u, uint8_t v) {
	dst[0] = u;
	dst[1] = y1;
	dst[2] = v;
	dst[3] = y2;
}

void Cinepak_Decoder::decodeFrameV4(Cinepak_YUV_Vector *v0, Cinepak_YUV_Vector *v1, Cinepak_YUV_Vector *v2, Cinepak_YUV_Vector *v3) {
	uint8_t *p = _yuvFrame + _yPos * _yuvPitch + _xPos * 2;

	SET_YUV_V4(&p[0], v0->y[0], v0->y[1], v0->u, v0->v);
	SET_YUV_V4(&p[4], v1->y[0], v1->y[1], v1->u, v1->v);
	p += _yuvPitch;
	SET_YUV_V4(&p[0], v0->y[2], v0->y[3], v0->u, v0->v);
	SET_YUV_V4(&p[4], v1->y[2], v1->y[3], v1->u, v1->v);
	p += _yuvPitch;
	SET_YUV_V4(&p[0], v2->y[0], v2->y[1], v2->u, v2->v);
	SET_YUV_V4(&p[4], v3->y[0], v3->y[1], v3->u, v3->v);
	p += _yuvPitch;
	SET_YUV_V4(&p[0], v2->y[2], v2->y[3], v2->u, v2->v);
	SET_YUV_V4(&p[4], v3->y[2], v3->y[3], v3->u, v3->v);
}

static void SET_YUV_V1(uint8_t *dst, uint8_t y, uint8_t u, uint8_t v) {
	dst[0] = u;
	dst[1] = y;
	dst[2] = v;
	dst[3] = y;
}

void Cinepak_Decoder::decodeFrameV1(Cinepak_YUV_Vector *v) {
	uint8_t *p = _yuvFrame + _yPos * _yuvPitch + _xPos * 2;

	SET_YUV_V1(&p[0], v->y[0], v->u, v->v);
	SET_YUV_V1(&p[4], v->y[1], v->u, v->v);
	p += _yuvPitch;
	SET_YUV_V1(&p[0], v->y[0], v->u, v->v);
	SET_YUV_V1(&p[4], v->y[1], v->u, v->v);
	p += _yuvPitch;
	SET_YUV_V1(&p[0], v->y[2], v->u, v->v);
	SET_YUV_V1(&p[4], v->y[3], v->u, v->v);
	p += _yuvPitch;
	SET_YUV_V1(&p[0], v->y[2], v->u, v->v);
	SET_YUV_V1(&p[4], v->y[3], v->u, v->v);
}

void Cinepak_Decoder::decodeVector(Cinepak_YUV_Vector *v) {
	for (int i = 0; i < 4; ++i) {
		v->y[i] = readByte();
	}
	v->u = 128 + readByte();
	v->v = 128 + readByte();
}

void Cinepak_Decoder::resizeCodebook(Cinepak_Codebook* codebook, int newStripCount) {
	Cinepak_YUV_Vector** old = codebook->stripVectors;
	codebook->stripVectors = new(_allocator) Cinepak_YUV_Vector*[newStripCount]();
	if (old) {
		memcpy(codebook->stripVectors, old, sizeof(Cinepak_YUV_Vector*) * stripVectorCount);
		delete old;
	}
}

void Cinepak_Decoder::decode(CINEDATA_TYPE data, int dataSize) {
	if (!dataSize) {
		return; //skip frame
	}
	_data = data;

	const uint8_t flags = readByte();
	skipBytes(3);
	_w = readWord();
	_h = readWord();
	const int strips = readWord();
	dataSize -= 10;
	//assert(_w == AVI_Player::kDefaultFrameWidth && _h == AVI_Player::kDefaultFrameHeight && strips == MAX_STRIPS);

	if (strips > stripVectorCount) {
		resizeCodebook(&codebookV1, strips);
		resizeCodebook(&codebookV4, strips);
		stripVectorCount = strips;
	}

	_xPos = _yPos = 0;
	int yMax = 0;

	for (int strip = 0; strip < strips; ++strip) {
		if (codebookV1.stripVectors[strip] == nullptr) {
			codebookV1.stripVectors[strip] = new(_allocator) Cinepak_YUV_Vector[256];
		}
		if (codebookV4.stripVectors[strip] == nullptr) {
			codebookV4.stripVectors[strip] = new(_allocator) Cinepak_YUV_Vector[256];
		}
		if (strip != 0 && (flags & 1) == 0) {
			memcpy(&codebookV1.stripVectors[strip][0], &codebookV1.stripVectors[strip - 1][0], sizeof(Cinepak_YUV_Vector) * MAX_VECTORS);
			memcpy(&codebookV4.stripVectors[strip][0], &codebookV4.stripVectors[strip - 1][0], sizeof(Cinepak_YUV_Vector) * MAX_VECTORS);
		}
		readByte();
		int size = read24();
		dataSize -= size;
		readWord();
		readWord();
		const int stripHeight = readWord();
		readWord();

		size -= 12;
		_xPos = 0;
		yMax += stripHeight;
		Cinepak_Codebook cb;
		int i;
		unsigned int vectorCount;
		while (size > 0) {
			int chunkType = readByte();
			unsigned int chunkSize = read24();
			size -= chunkSize;
			chunkSize -= 4;

			switch (chunkType) {
			case 0x20:
			case 0x22:
				cb = (chunkType == 0x22) ? codebookV1 : codebookV4;
				vectorCount = (((unsigned long long)chunkSize * 0x2AAAAAABuLL) >> 32uLL); //divide by 6 in fixed point
				for (i = 0; i < vectorCount; ++i) {
					decodeVector(&cb.stripVectors[strip][i]);
				}
				chunkSize = 0;
				break;
			case 0x21:
			case 0x23:
				cb = (chunkType == 0x23) ? codebookV1 : codebookV4;
				i = 0;
				while (chunkSize > 0) {
					const uint32_t mask = readLong();
					chunkSize -= 4;
					for (int bit = 0; bit < 32; ++bit) {
						if (mask & (1 << (31 - bit))) {
							decodeVector(&cb.stripVectors[strip][i]);
							chunkSize -= 6;
						}
						++i;
					}
				}
				break;
			case 0x30:
				while (chunkSize > 0 && _yPos < yMax) {
					uint32_t mask = readLong();
					chunkSize -= 4;
					for (int bit = 0; bit < 32 && _yPos < yMax; ++bit) {
						if (mask & (1 << (31 - bit))) {
							Cinepak_YUV_Vector *v0 = &codebookV4.stripVectors[strip][readByte()];
							Cinepak_YUV_Vector *v1 = &codebookV4.stripVectors[strip][readByte()];
							Cinepak_YUV_Vector *v2 = &codebookV4.stripVectors[strip][readByte()];
							Cinepak_YUV_Vector *v3 = &codebookV4.stripVectors[strip][readByte()];
							chunkSize -= 4;
							decodeFrameV4(v0, v1, v2, v3);
						} else {
							Cinepak_YUV_Vector *v0 = &codebookV1.stripVectors[strip][readByte()];
							--chunkSize;
							decodeFrameV1(v0);
						}
						_xPos += 4;
						if (_xPos >= _w) {
							_xPos = 0;
							_yPos += 4;
						}
					}
				}
				break;
			case 0x31:
				while (chunkSize > 0 && _yPos < yMax) {
					uint32_t mask = readLong();
					chunkSize -= 4;
					for (int bit = 0; bit < 32 && chunkSize >= 0 && _yPos < yMax; ) {
						if (mask & (1 << (31 - bit))) {
							++bit;
							if (bit == 32) {
								//assert(chunkSize >= 4);
								mask = readLong();
								chunkSize -= 4;
								bit = 0;
							}
							if (mask & (1 << (31 - bit))) {
								Cinepak_YUV_Vector *v0 = &codebookV4.stripVectors[strip][readByte()];
								Cinepak_YUV_Vector *v1 = &codebookV4.stripVectors[strip][readByte()];
								Cinepak_YUV_Vector *v2 = &codebookV4.stripVectors[strip][readByte()];
								Cinepak_YUV_Vector *v3 = &codebookV4.stripVectors[strip][readByte()];
								chunkSize -= 4;
								decodeFrameV4(v0, v1, v2, v3);
							} else {
								Cinepak_YUV_Vector *v0 = &codebookV1.stripVectors[strip][readByte()];
								--chunkSize;
								decodeFrameV1(v0);
							}
						}
						++bit;
						_xPos += 4;
						if (_xPos >= _w) {
							_xPos = 0;
							_yPos += 4;
						}
					}
				}
				break;
			case 0x32:
				while (chunkSize > 0 && _yPos < yMax) {
					Cinepak_YUV_Vector *v0 = &codebookV1.stripVectors[strip][readByte()];
					--chunkSize;
					decodeFrameV1(v0);
					_xPos += 4;
					if (_xPos >= _w) {
						_xPos = 0;
						_yPos += 4;
					}
				}
				break;
			default:
				EXL_DEBUG_PRINTF("Unknown Cinepak chunk type %x\n", chunkType);
				break;
			}
			skipBytes(chunkSize);
		}
	}
	if (dataSize) {
		skipBytes(dataSize);
	}
}

AVI_Player::AVI_Player(exl::heap::Allocator* allocator)
	: _soundQueue(0), _soundTailQueue(0), _demux(allocator), _cinepak(allocator) {
    _allocator = allocator;
	_vidCb = nullptr;
	_audCb = nullptr;
}

AVI_Player::~AVI_Player() {
	while (_soundQueue) {
		AVI_SoundBufferQueue *next = _soundQueue->next;
		exl::heap::Allocator::FreeStatic(_soundQueue->buffer);
		exl::heap::Allocator::FreeStatic(_soundQueue);
		_soundQueue = next;
	}
}

bool AVI_Player::openStream(exl::io::Stream* f, int decodeFlags) {
	_decodeFlags = decodeFlags;
	_soundQueue = 0;
	_soundQueuePreloadSize = 0;
	_cinepak._yuvFrame = nullptr;
	_nowFrame = 0;
	return _demux.open(f);
}

void AVI_Demuxer::prepareChunkDecode(AVI_Chunk& chunk) {
	#ifdef AVI_OWN_BUFFERING
	if (chunk.dataSize <= _chunkDataSize) {
		_f->Read(_chunkData, sizeof(char), chunk.dataSize);
		chunk.data = _chunkData;
	}
	else {
		EXL_DEBUG_PRINTF("Chunk data size 0x%x exceeds maximum size 0x%x!", chunk.dataSize, _chunkDataSize);
	}
	#else
	chunk.data = _f;
	#endif
}

bool AVI_Player::readNextFrame() {
	bool decodeVideo = ((_decodeFlags & AVI_DECODE_VIDEO) != 0) && (_vidCb != nullptr);
	bool decodeAudio = ((_decodeFlags & AVI_DECODE_AUDIO) != 0) && (_audCb != nullptr);
	if (decodeVideo) {
		if (!_cinepak._yuvFrame) {
			_cinepak.allocYUV(_demux._width, _demux._height);
			EXL_DEBUG_PRINTF("YUV buffer allocated\n");
		}
	}
	AVI_Chunk chunk;
	while (_demux.readNextChunk(chunk)) {
		switch (chunk.type) {
			case kChunkAudioType:
				if (decodeAudio) {
					_demux.prepareChunkDecode(chunk);
					decodeAudioChunk(chunk);
				}
				else {
					skipChunk(chunk);
				}
				break;
			case kChunkVideoType:
				if (decodeVideo) {
					_demux.prepareChunkDecode(chunk);
					decodeVideoChunk(chunk);
				}
				else {
					skipChunk(chunk);
				}
				break;
		}
		if (chunk.type == kChunkVideoType) {
			_nowFrame++;
			return true;
		}
	}
	return false;
}

void AVI_Player::freeSoundQueue() {
	AVI_SoundBufferQueue* q = _soundQueue;
	while (q) {
		_allocator->Free(q->buffer);
		AVI_SoundBufferQueue* toDelete = q;
		q = q->next;
		delete q;
	}
}

void AVI_Player::close() {
	freeSoundQueue();
	if (_cinepak._yuvFrame) {
		_allocator->Free(_cinepak._yuvFrame);
	}
	_demux.close();
}

void AVI_Player::play(exl::io::Stream *f, int decodeFlags) {
	if (openStream(f, decodeFlags)) {
		while (readNextFrame()) {
			;
		}
		close();
	}
}

void AVI_Player::restart() {
	if (_demux._f && _demux._mediaBeginOfs != 0) {
		freeSoundQueue();
		_nowFrame = 0;
		_demux._f->SeekSet(_demux._mediaBeginOfs);
	}
}

void AVI_Player::decodeAudioChunk(AVI_Chunk &c) {
	AVI_SoundBufferQueue *sbq = new(_allocator) AVI_SoundBufferQueue();
	if (sbq) {
		sbq->buffer = (uint8_t *)_allocator->Alloc(c.dataSize);
		if (sbq->buffer) {
			#ifdef AVI_OWN_BUFFERING
			memcpy(sbq->buffer, c.data, c.dataSize);
			#else
            c.data->Read(sbq->buffer, sizeof(char), c.dataSize);
			#endif
			sbq->size = c.dataSize;
			sbq->offset = 0;
			sbq->next = 0;
		} else {
			_allocator->Free(sbq);
			sbq = 0;
		}
	}
	if (sbq) {
		if (!_soundQueue) {
			_soundQueue = sbq;
		} else {
			AVI_SoundBufferQueue *p = _soundTailQueue;
			//assert(!p->next);
			p->next = sbq;
		}
		_soundTailQueue = sbq;
		if (_soundQueuePreloadSize < kSoundPreloadSize) {
			++_soundQueuePreloadSize;
		}
		if (_audCb) {
			_audCb(_supervisor, _soundQueue);
		}
	}
}

void AVI_Player::decodeVideoChunk(AVI_Chunk &c) {
	if (_cinepak._yuvFrame) {
		_cinepak.decode(c.data, c.dataSize);
		if (_vidCb) {
			_vidCb(_supervisor, _cinepak._yuvFrame);
		}
	}
}

void AVI_Player::skipChunk(AVI_Chunk &c) {
	_demux._f->SeekCur(c.dataSize);
}

void AVI_Player::mix(int16_t *buf, int samples) {
	if (_soundQueuePreloadSize < kSoundPreloadSize) {
		return;
	}
	while (_soundQueue && samples > 0) {
		int sample = (_soundQueue->buffer[_soundQueue->offset] << 8) ^ 0x8000;
		*buf++ = (int16_t)sample;
		*buf++ = (int16_t)sample;
		_soundQueue->offset += 2; // skip every second sample (44Khz stream vs 22Khz mixer)
		if (_soundQueue->offset >= _soundQueue->size) {
			AVI_SoundBufferQueue *next = _soundQueue->next;
			exl::heap::Allocator::FreeStatic(_soundQueue->buffer);
			exl::heap::Allocator::FreeStatic(_soundQueue);
			_soundQueue = next;
		}
		--samples;
	}
	if (!_soundQueue) {
		_soundTailQueue = 0;
	}
	if (samples > 0) {
		//warning("AVI_Player::mix() soundQueue underrun %d", samples);
	}
}
