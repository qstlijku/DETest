#pragma once

class IBinaryArchive;

#include <SDL_rwops.h>
#include "glm/glm.hpp"
#include <string>
#include <memory>
#include "Vector.h"
#include "CStringID.h"

class IBinaryArchive {
public:
	IBinaryArchive();
	virtual ~IBinaryArchive() {}

	void serialize(bool& value);
	void serialize(uint8_t& value);
	void serialize(char& value);
	void serialize(uint16_t& value);
	void serialize(int16_t& value);
	void serialize(uint32_t& value);
	void serialize(int32_t& value);
	void serialize(uint64_t& value);
	void serialize(int64_t& value);
	void serialize(float& value);
	void serialize(double& value);
	void serialize(glm::vec2& value);
	void serialize(glm::ivec2& value);
	void serialize(glm::vec3& value);
	void serialize(glm::vec4& value);
	void serialize(glm::mat4& value);
	void serialize(std::string& value);

	template<typename T>
	void serializeInPlace(std::vector<T> &data, uint32_t count, uint32_t padding = sizeof(T));

	void PreAllocateSizeOfType(CStringID type, uint32_t count);
	void PreAllocatePointers(uint32_t count);
	void PreAllocateDynamicType(CStringID type);
	void PreAllocateMemory(uint32_t unk1, uint32_t unk2);

	void PauseInPlace();
	void ResumeInPlace();
	int numPausedInPlace = 0;

	virtual bool isReading() const = 0;
	void pad(size_t padding);
	void padInPlace(size_t padding);
	size_t size();
	size_t tell();
	virtual void memBlock(void* ptr, size_t objSize, size_t objCount) = 0;
	virtual void memBlockInPlace(void* ptr, size_t objSize, size_t objCount) = 0;

	template<typename T>
	void serializeConstant(const T &value);

	template<typename T>
	void serializeNdVector(Vector<T>& vec);

	template<typename T>
	void serializeNdVectorExternal_TOREMOVE(Vector<T>& vec, uint32_t typeId, uint32_t& unk);

	template<typename T>
	void serializeNdVectorExternal(Vector<T>& vec, CStringID typeId, bool alwaysPreAllocate = false);

	template<typename T>
	void serializeNdVectorExternal(Vector<std::unique_ptr<T>>& vec, CStringID typeId);

	template<typename T>
	void serializeNdVectorInPlace(Vector<T>& vec, uint32_t padding = sizeof(T));

	template<typename T>
	void serialize(T &value);

	SDL_RWops* fp;
	bool bigEndian = false;
	enum PaddingType { PADDING_NONE, PADDING_IBINARYARCHIVE, PADDING_GEAR
	};
	PaddingType padding = PADDING_IBINARYARCHIVE;

	size_t getPadSize(size_t padding);

	virtual void markHeader() = 0;
	virtual void markInPlaceOffset(size_t offset) = 0;
	virtual void finish() = 0;

	struct Header {
		uint32_t unk1 = 0;//head1 = *(uint *)&param_1->dataSize;
		uint32_t unk2 = 0;//head2 = *(uint *)&param_1->dataSize + pad to 16;
		uint32_t unk3 = 0;//head3 = *(uint*)&param_1->InPlaceDataSize;
		uint32_t unk4 = 0;//head4
		uint32_t unk5 = 0;//head5 = *(uint *)&param_1->dataSize; without PreAllocations
		uint32_t unk6 = 0;//head6
		uint32_t unk7 = 0;//head7 = head6 * 0x10 + head5;
		uint32_t unk8 = 0;//head8
		uint32_t unk9 = 0;//head9
		void read(IBinaryArchive& fp);
	};
	IBinaryArchive::Header header;

	struct Unk {
		CStringID type;
		uint32_t numInstances = 0;
		uint32_t unk3;
		uint32_t unk4;
		void read(IBinaryArchive& fp);
	};
	std::vector<Unk> head6;
	std::vector<Unk> head8;

	uint32_t beginOffset = 0;
	Sint64 inPlaceRead = 0;

protected:
	Sint64 offset = 0;//Usefull for debugging
};

bool operator==(const IBinaryArchive::Header& lhs, const IBinaryArchive::Header& rhs);

class CBinaryArchiveReader : public IBinaryArchive {
public:
	CBinaryArchiveReader(SDL_RWops* _fp);
	~CBinaryArchiveReader() {}

	Sint64 inPlaceOffset = -1;

	bool isReading() const;
	void memBlock(void* ptr, size_t objSize, size_t objCount);
	void memBlockInPlace(void* ptr, size_t objSize, size_t objCount);
	void markHeader();
	void markInPlaceOffset(size_t offset);
	void finish();
};

class CBinaryArchiveWriter : public IBinaryArchive {
public:
	CBinaryArchiveWriter(SDL_RWops* _fp);
	~CBinaryArchiveWriter();

	int64_t headerOffset = -1;
	std::vector<uint8_t> inPlaceData;

	bool isReading() const;
	void memBlock(void* ptr, size_t objSize, size_t objCount);
	void memBlockInPlace(void* ptr, size_t objSize, size_t objCount);
	void markHeader();
	void markInPlaceOffset(size_t offset);
	void finish();
};

template<typename T>
inline void IBinaryArchive::serializeInPlace(std::vector<T>& data, uint32_t count, uint32_t padding) {
	//Align to Padding
	padInPlace(padding);

	data.resize(count);
	memBlockInPlace(data.data(), sizeof(T), count);

	SDL_assert(!bigEndian);
}

template<typename T>
inline void IBinaryArchive::serializeConstant(const T &value) {
	T v = value;
	serialize(v);
	SDL_assert_release(v == value);
}

template<typename T>
inline void IBinaryArchive::serializeNdVector(Vector<T>& vec) {
	uint32_t count = (uint32_t)vec.size();
	serialize(count);
	vec.resize(count);
	for (uint32_t i = 0; i < count; ++i)
		serialize(vec[i]);
}

template<>
inline void IBinaryArchive::serializeNdVector(Vector<uint8_t>& vec) {
	uint32_t count = (uint32_t) vec.size();
	serialize(count);
	vec.resize(count);
	memBlock(vec.data(), 1, count);
}

template<typename T>
inline void IBinaryArchive::serializeNdVectorExternal_TOREMOVE(Vector<T>& vec, uint32_t typeId, uint32_t &unk) {
	//PreAllocateSizeOfType behavior
	uint32_t counter = (uint32_t)vec.size(), counter2 = (uint32_t)vec.size();
	serialize(counter);

	serializeConstant(typeId);

	serialize(unk);

	//Vector Count
	serialize(counter2);
	SDL_assert_release(counter == counter2);

	vec.resize(counter);
	for (uint32_t i = 0; i < counter; ++i)
		vec[i].read(*this);
}

template<typename T>
inline void IBinaryArchive::serializeNdVectorExternal(Vector<T>& vec, CStringID typeId, bool alwaysPreAllocate) {
	//PreAllocateSizeOfType behavior
	uint32_t counter = (uint32_t)vec.size();
	serialize(counter);
	vec.resize(counter);

	if (counter || alwaysPreAllocate)
		PreAllocateSizeOfType(typeId, counter);

	for (uint32_t i = 0; i < counter; ++i)
		vec[i].read(*this);
}

template<typename T>
inline void IBinaryArchive::serializeNdVectorExternal(Vector<std::unique_ptr<T>>& vec, CStringID typeId) {
	//PreAllocateSizeOfType behavior
	uint32_t counter = (uint32_t)vec.size();
	serialize(counter);
	vec.resize(counter);

	if (counter) {
		PreAllocatePointers(counter);

		uint32_t unk1 = 0;//I think this is the count of set pointers
		for (uint32_t i = 0; i < counter; ++i)
			unk1 += vec[i].get() ? 1 : 0;
		serialize(unk1);

		if(unk1)
			PreAllocateSizeOfType(typeId, unk1);

		for (uint32_t i = 0; i < counter; ++i) {
			bool has = vec[i].get();
			serialize(has);

			if (isReading()) {
				if (has) {
					vec[i] = std::make_unique<T>();
					vec[i]->read(*this);
				} else {
					vec[i].reset();
				}
			} else {
				if(has)
					vec[i]->read(*this);
			}

		}
	}
}

template<typename T>
inline void IBinaryArchive::serializeNdVectorInPlace(Vector<T>& vec, uint32_t padding) {
	uint32_t count = vec.size();
	serialize(count);
	
	serializeInPlace(vec, count, padding);
}

template<typename T>
__forceinline void IBinaryArchive::serialize(T & value) {
	value.read(*this);
}
