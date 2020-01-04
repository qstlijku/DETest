#pragma once

#include <SDL_rwops.h>
#include "glm/glm.hpp"
#include <string>
#include "Vector.h"

class IBinaryArchive;

class IBinaryArchive {
public:
	IBinaryArchive();
	virtual ~IBinaryArchive() {}

	void serialize(bool& value);
	void serialize(uint8_t& value);
	void serialize(int8_t& value);
	void serialize(uint16_t& value);
	void serialize(int16_t& value);
	void serialize(uint32_t& value);
	void serialize(int32_t& value);
	void serialize(uint64_t& value);
	void serialize(int64_t& value);
	void serialize(float& value);
	void serialize(double& value);
	void serialize(glm::vec2& value);
	void serialize(glm::vec3& value);
	void serialize(glm::vec4& value);
	void serialize(glm::mat4& value);
	void serialize(std::string& value);

	void serializeInPlace(uint8_t& value);
	void serializeInPlace(uint16_t& value);
	void serializeInPlace(uint32_t& value);
	void serializeInPlace(uint64_t& value);
	void serializeInPlace(glm::vec4& value);

	virtual bool isReading() const = 0;
	virtual void pad(size_t padding) = 0;
	size_t size();
	size_t tell();
	virtual void memBlock(void* ptr, size_t objSize, size_t objCount) = 0;
	virtual void memBlockInPlace(void* ptr, size_t objSize, size_t objCount) = 0;

	template<typename T>
	void serializeNdVector(Vector<T>& vec);

	template<typename T>
	void serializeNdVector_pod(Vector<T>& vec);

	template<typename T>
	void serializeNdVectorExternal(Vector<T>& vec, uint32_t typeId, uint32_t& unk);

	template<typename T>
	void serialize(T &value);

	SDL_RWops* fp;
	bool bigEndian = false;
	enum PaddingType { PADDING_NONE, PADDING_IBINARYARCHIVE, PADDING_GEAR
	};
	PaddingType padding = PADDING_IBINARYARCHIVE;

	size_t getPadSize(size_t padding);

	virtual void markHeader() = 0;
protected:
	Sint64 offset = 0;//Usefull for debugging
	struct Header {
		uint32_t inPlaceOffset = 0;//Padded out to 16
		uint32_t unk2 = 0;//Unused.
		uint32_t unk3 = 0;//Used in SerailizeBasicTypeInPlace

		uint32_t unk4 = 0;
		uint32_t unk5 = 0;//Size of header + non-inplace serialized data
		uint32_t unk6 = 0;//Number of Loops, done on construction

		uint32_t unk7 = 0;//Size of header + non-inplace serialized data
		uint32_t unk8 = 0;//Unused?
		uint32_t unk9 = 0;
		void read(IBinaryArchive& fp);
	};
};


class CBinaryArchiveReader : public IBinaryArchive {
public:
	CBinaryArchiveReader(SDL_RWops* _fp);
	~CBinaryArchiveReader() {}

	IBinaryArchive::Header header;
	Sint64 inPlaceOffset = -1;

	bool isReading() const;
	void pad(size_t padding);
	void memBlock(void* ptr, size_t objSize, size_t objCount);
	void memBlockInPlace(void* ptr, size_t objSize, size_t objCount);
	void markHeader();
};

class CBinaryArchiveWriter : public IBinaryArchive {
public:
	CBinaryArchiveWriter(SDL_RWops* _fp);
	~CBinaryArchiveWriter();

	int64_t headerOffset = -1;
	int64_t dataSize = 0;
	std::vector<uint8_t> inPlaceData;

	bool isReading() const;
	void pad(size_t padding);
	void memBlock(void* ptr, size_t objSize, size_t objCount);
	void memBlockInPlace(void* ptr, size_t objSize, size_t objCount);
	void markHeader();
	void finish();
};

template<typename T>
inline void IBinaryArchive::serializeNdVector(Vector<T>& vec) {
	uint32_t count = (uint32_t)vec.size();
	serialize(count);
	vec.resize(count);
	for (uint32_t i = 0; i < count; ++i)
		serialize(vec[i]);
}

template<typename T>
inline void IBinaryArchive::serializeNdVector_pod(Vector<T>& vec) {
	serializeNdVector(vec);
}

template<>
inline void IBinaryArchive::serializeNdVector_pod(Vector<uint8_t>& vec) {
	uint32_t count = vec.size();
	serialize(count);
	vec.resize(count);
	memBlock(vec.data(), 1, count);
}

template<typename T>
inline void IBinaryArchive::serializeNdVectorExternal(Vector<T>& vec, uint32_t typeId, uint32_t &unk) {
	//PreAllocateSizeOfType behavior
	uint32_t counter = vec.size(), counter2 = vec.size();
	serialize(counter);

	uint32_t unknownTypeID = typeId;
	serialize(unknownTypeID);
	SDL_assert_release(unknownTypeID == typeId);

	serialize(unk);

	//Vector Count
	serialize(counter2);
	SDL_assert_release(counter == counter2);

	vec.resize(counter);
	for (uint32_t i = 0; i < counter; ++i)
		vec[i].read(*this);
}

template<typename T>
__forceinline void IBinaryArchive::serialize(T & value) {
	value.read(*this);
}
