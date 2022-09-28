#ifndef AVI_PLAYER_H__
#define AVI_PLAYER_H__

#include "IO/exl_Stream.h"
#include "Heap/exl_Allocator.h"

/*
 * Bermuda Syndrome engine rewrite
 * Copyright (C) 2007-2011 Gregory Montoir
 */

#define AVI_OWN_BUFFERING

#ifdef AVI_OWN_BUFFERING
#define CINEDATA_TYPE uint8_t*
#else
#define CINEDATA_TYPE exl::io::Stream*
#endif

enum AVI_ChunkType {
	kChunkNullType,
	kChunkAudioType,
	kChunkVideoType,
	kChunkTermType
};

struct AVI_Chunk {
	AVI_ChunkType type;
	CINEDATA_TYPE data;
	int dataSize;
};

struct AVI_Demuxer {
	enum {
		kSizeOfChunk_avih = 56,
		kSizeOfChunk_strh = 56,
		kSizeOfChunk_waveformat = 16,
		kSizeOfChunk_bitmapinfo = 40
	};

    AVI_Demuxer(exl::heap::Allocator* allocator);

	bool open(exl::io::Stream *f);
	void close();

	bool readHeader();
	bool readHeader_avih();
	bool readHeader_strh();
	bool readHeader_strf_auds();
	bool readHeader_strf_vids();

	bool readNextChunk(AVI_Chunk &chunk);
	void prepareChunkDecode(AVI_Chunk& chunk);

    exl::heap::Allocator* _allocator;

	int _frames;
	int _width, _height;
	int _streams;
	int _frameRate;

	int _audioNChannels;
	int _audioRate;
	int _audioBps;

	exl::io::Stream *_f;
	int _mediaBeginOfs;
	int _recordsListSize;
	uint8_t *_chunkData;
	int _chunkDataSize;
	int _audioBufferSize;
	int _videoBufferSize;
};

struct Cinepak_YUV_Vector {
	uint8_t y[4];
	uint8_t u, v;
};

struct Cinepak_Codebook {
	Cinepak_YUV_Vector** stripVectors;
};

enum {
	kCinepakV1 = 0,
	kCinepakV4 = 1
};

struct Cinepak_Decoder {
	enum {
		MAX_STRIPS = 2,
		MAX_VECTORS = 256
	};

	#ifdef AVI_OWN_BUFFERING
	INLINE uint8_t readByte() {
        uint8_t r = *_data;
		_data++;
		return r;
	}

	INLINE uint16_t readWord() {
        uint16_t r = _data[1] | (_data[0] << 8);
		_data += 2;
		return r;
	}

	INLINE uint32_t read24() {
        uint32_t r = _data[2] | (_data[1] << 8) | (_data[0] << 16);
		_data += 3;
		return r;
	}

	INLINE uint32_t readLong() {
        uint32_t r = _data[3] | (_data[2] << 8) | (_data[1] << 16) | (_data[0] << 24);
		_data += 4;
		return r;
	}

	INLINE void skipBytes(int amount) {
		_data += amount;
	}
	#else
	uint8_t readByte() {
        uint8_t b;
        _data->Read(&b, sizeof(uint8_t), 1);
        return b;
	}

	uint16_t readWord() {
		uint8_t b[2];
		_data->Read(b, sizeof(uint8_t), 2);
        return b[1] | (b[0] << 8);
	}

	uint32_t read24() {
		uint8_t b[3];
		_data->Read(b, sizeof(uint8_t), 3);
        return b[2] | (b[1] << 8) | (b[0] << 16);
	}

	uint32_t readLong() {
		uint8_t b[4];
		_data->Read(b, sizeof(uint8_t), 4);
        return b[3] | (b[2] << 8) | (b[1] << 16) | (b[0] << 24);
	}

	void skipBytes(int amount) {
		_data->SeekCur(amount);
	}
	#endif

	Cinepak_Decoder(exl::heap::Allocator* allocator);

	void allocYUV(int width, int height);
	void resizeCodebook(Cinepak_Codebook* codebook, int newStripCount);
	void decodeFrameV4(Cinepak_YUV_Vector *v0, Cinepak_YUV_Vector *v1, Cinepak_YUV_Vector *v2, Cinepak_YUV_Vector *v3);
	void decodeFrameV1(Cinepak_YUV_Vector *v);
	void decodeVector(Cinepak_YUV_Vector *v);
	void decode(CINEDATA_TYPE data, int dataSize);

	exl::heap::Allocator* _allocator;
	CINEDATA_TYPE _data;
	Cinepak_Codebook codebookV1;
	Cinepak_Codebook codebookV4;
	int stripVectorCount;
	int _w, _h;
	int _xPos, _yPos, _yMax;

	uint8_t *_yuvFrame;
	int _yuvPitch;
};

struct AVI_SoundBufferQueue {
	uint8_t *buffer;
	int size;
	int offset;
	AVI_SoundBufferQueue *next;
};

struct AVI_Player {
	#define AVI_DECODE_VIDEO (1 << 0)
	#define AVI_DECODE_AUDIO (1 << 1)

	AVI_Player(exl::heap::Allocator* allocator);
	~AVI_Player();

public:
	typedef void(* VideoCallback)(void* supervisor, uint8_t* yuv);
	typedef void(* AudioCallback)(void* supervisor, AVI_SoundBufferQueue* sbq);

	bool openStream(exl::io::Stream* f, int decodeFlags);
	INLINE void setSupervisor(void* supervisor) {
		_supervisor = supervisor;
	}
	INLINE void setVideoCallback(VideoCallback callback) {
		_vidCb = callback;
	}
	INLINE void setAudioCallback(AudioCallback callback) {
		_audCb = callback;
	}
	bool skipFrame();
	bool readNextFrame();
	void close();

	void play(exl::io::Stream *f, int decodeFlags);
	void restart();

	INLINE int getFrame() {
		return _nowFrame;
	}

	INLINE void getVideoDimensions(int* pW, int* pH) {
		*pW = _demux._width;
		*pH = _demux._height;
	}

	INLINE void getAudioInfo(int* pNChannels, int* pSampleRate, int* pBps) {
		*pNChannels = _demux._audioNChannels;
		*pSampleRate = _demux._audioRate;
		*pBps = _demux._audioBps;
	}

	uint32_t getAvailableSamples();
	void getSamples(void *buf, int samples, int channel);
	void discardSamples(int samples);
private:
	void freeSoundQueue();
	void decodeAudioChunk(AVI_Chunk &c);
	void decodeVideoChunk(AVI_Chunk &c);
	void skipChunk(AVI_Chunk &c);
	void mix(int16_t *buf, int samples);

	exl::heap::Allocator* _allocator;
	void* _supervisor;
	int _decodeFlags;
	int _nowFrame;
	AVI_Demuxer _demux;
	VideoCallback _vidCb;
	AudioCallback _audCb;
	AVI_SoundBufferQueue *_soundQueue, *_soundTailQueue;
	int _soundQueuePreloadSize;
	Cinepak_Decoder _cinepak;
	AVI_Chunk _currentChunk;
};

#endif // AVI_PLAYER_H__
