#include "CMoveResourceDataManager.h"

#include "IBinaryArchive.h"

template <typename T>
Vector<T> getVectorFromAttribute(Attribute* it) {
	SDL_assert_release(it->buffer.size() % sizeof(T) == 0);
	uint32_t head = *(uint32_t*)it->buffer.data();
	Vector<T> data;
	data.assign((T*)(it->buffer.data() + 4), (T*)(it->buffer.data() + it->buffer.size()));
	SDL_assert_release(data.size() == head);
	return data;
}

void CMoveResourceDataManager::open(IBinaryArchive& fp) {
	// CMoveResourceDataManager::LoadData((ndStringBase__tm__2_c const &))
	fp.padding = fp.PADDING_NONE;

	uint32_t magic = 12817;//Not Used by game
	fp.serialize(magic);

	uint32_t part1Size;
	fp.serialize(part1Size);

	uint32_t fcbSize;
	fp.serialize(fcbSize);

	SDL_assert_release(part1Size);

	//Read part1Size bytes
	SDL_RWseek(fp.fp, part1Size, RW_SEEK_CUR);

	//Read FCB
	readFCB(fp, root);

	SDL_assert_release(fp.tell() == (4 * 3) + part1Size + fcbSize);
	SDL_assert_release(fp.tell() == fp.size());

	Node *PerMoveResourceInfo = root.findFirstChild("PerMoveResourceInfo");
	Vector<uint32_t> sizes = getVectorFromAttribute<uint32_t>(PerMoveResourceInfo->getAttribute("sizes"));
	Vector<uint32_t> rootNodeIds = getVectorFromAttribute<uint32_t>(PerMoveResourceInfo->getAttribute("rootNodeIds"));
	SDL_assert_release(sizes.size() == rootNodeIds.size());

	//Sanity Check
	uint32_t sumOfSizes = 0;
	for (auto& n : sizes)
		sumOfSizes += n;
	SDL_assert_release(sumOfSizes == part1Size);

	//Seek Back to Part1 Data
	SDL_RWseek(fp.fp, 4 * 3, RW_SEEK_SET);
	
	for (size_t i = 0; i < sizes.size(); ++i) {
		size_t offset = fp.tell();
		uint32_t nodeID = rootNodeIds[i];
		uint32_t size = sizes[i];

		//CMoveResource::InitMoveResourceData(const ROC::Ptr<const SMoveResourceDataBlock> &, CStringID nodeID)
		//CActiveMoveHandle<CMoveDataNode_Game> CActiveMoveHandle<T1>::GetFirstNodeFromResourceBlock(const ROC::Ptr<const SMoveResourceDataBlock> &) [with T1=CMoveDataNode_Game]
		//CActiveMoveHandle<CMoveDataNode_Game> 8 bytes

		//SMoveResourceDataBlock 16 bytes
		Vector<uint8_t> &data = _f9706572[nodeID];
		data.resize(size);
		fp.memBlock(data.data(), 1, size);
	}
	SDL_assert_release(fp.tell() == (4 * 3) + part1Size);

	//First, Let's do CAnimParam
	{
		Node* ANIMPARAM_VALUES = root.findFirstChild("ANIMPARAM_VALUES");
		Node* ANIMPARAM_FIXUPS = root.findFirstChild("ANIMPARAM_FIXUPS");

		Vector<uint32_t> hashes = getVectorFromAttribute<uint32_t>(ANIMPARAM_FIXUPS->getAttribute("hashesArray"));
		Vector<uint32_t> offsets = getVectorFromAttribute<uint32_t>(ANIMPARAM_FIXUPS->getAttribute("offsetsArray"));
		SDL_assert_release(hashes.size() == offsets.size());

		for (size_t i = 0; i < hashes.size(); ++i) {
			uint32_t hash = hashes[i];//Hash corresponds to ANIMPARAM_VALUES.hash
			uint32_t offset = offsets[i];
			SDL_RWseek(fp.fp, (4 * 3) + offset, RW_SEEK_SET);


		}
	}
}
