#include "mabFile.h"
#include "IBinaryArchive.h"
#include <DB.h>
#include <SDL_log.h>

void mabFile::open(IBinaryArchive & fp) {
	uint32_t magic;
	fp.serialize(magic);
	SDL_assert_release(magic == 0x329B || magic == 0x46B4);

	bool isWDL = (magic == 0x46B4);
	if (isWDL)
	{
		// skip padding (WDL)
		SDL_RWseek(fp.fp, 0x14, RW_SEEK_CUR);
	}
	else
		SDL_RWseek(fp.fp, 0x4, RW_SEEK_CUR);

	fp.serialize(header1);
	fp.serialize(header2);
	if (!isWDL)
	    fp.serialize(header3);

	fp.serialize(signature[0]);
	fp.serialize(signature[1]);
	fp.serialize(signature[2]);
	fp.serialize(flags);
	SDL_assert_release(signature[0] == 0x61);
	SDL_assert_release(signature[1] == 0x4E);
	SDL_assert_release(signature[2] == 0x69 || signature[2] == 0x70);

	fp.serialize(animationDataSize);
	fp.serialize(duration);

	if (isWDL)
	{
		fp.serialize(animFrameRate);
		SDL_assert_release(animFrameRate == 30.0f);
	}

	fp.serialize(numBonesInAnim);
	SDL_Log("numBonesInAnim: %d\n", numBonesInAnim);

	if (isWDL)
	{
		for (int i = 0; i < 10; i++)
			fp.serialize(dataCounts[i]);
		fp.serializeConstant((uint16_t)0);

		for (int i = 0; i < 12; i++)
			fp.serialize(offsets[i]);

		fp.serializeConstant(0); // paddingToMatchPS3
	}
	else
	{
		for (int i = 0; i < 7; i++)
			fp.serialize(dataCounts1[i]);
		for (int i = 0; i < 11; i++)
			fp.serialize(offsets1[i]);
	}

	fp.serialize(lastFrame);
	SDL_Log("lastFrame: %d\n", lastFrame);
	fp.pad(2);

	std::vector<std::string> bones;

	for (int i = 0; i < numBonesInAnim; i++)
	{
		uint32_t boneHash;
		fp.serialize(boneHash);
		std::string test = DB::instance().getStrFromCRC(boneHash);
		bones.push_back(test);
	}

	for (int i = 0; i < numBonesInAnim; i++)
	{
		uint32_t frame;
		fp.serialize(frame);
		frameArray.push_back(frame);
	}

	SDL_RWseek(fp.fp, offsets[6] + 0x10, RW_SEEK_SET);

	uint32_t numOffsets;
	fp.serialize(numOffsets);

	for (int i = 1; i < numOffsets / 4; i++)
	{
		uint32_t offset;
		fp.serialize(offset);
		jointRotOffsets.push_back(offset);
	}

	auto sectionSize = jointRotOffsets[0] - numOffsets;
	auto sectionSize2 = jointRotOffsets[1] - jointRotOffsets[0];

	for (int i = 0; i < sectionSize; i++)
	{
		uint8_t nextByte;
		fp.serialize(nextByte);
		bitstream.push_back(nextByte);
	}

	SDL_Log("sectionSize: %d\n", sectionSize);

	// Section 1: KeyTimes (offsets[0]), usually linear pretty straightforward
	// Section 2: TranslationTable (offsets[1], See FillBoneAddressingTable); seems to be dataCounts[6]
	// also FillTranslationCompressionTable
	// Section 3: DisplacementOrientations
	// Section 4: DisplacementTranslations
	// Section 5: JointConstantRotations (FF BF FF BF FF 3F repeating)
	// Section 6: JointConstantTranslations
	// Section 7: offsets[6], JointRotations (this is usually the longest!)
	// Section 8: offsets[7], JointTranslations
	// Section 9: see FC template (anchors and stuff)
	// Section 10: markup (ignore for now)
	// Section 11: empty?
}
