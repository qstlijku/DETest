#include "SplineLoft.h"

void SplineLoftHiRes::open(IBinaryArchive& fp) {
	uint32_t magic = 1397508178;
	fp.serialize(magic);
	SDL_assert_release(magic == 1397508178);

	uint32_t version = 7;
	fp.serialize(version);
	SDL_assert_release(version == 7);

	fp.serialize(unk1);

	if (fp.isReading()) {
		struct UnkStr {
			std::array<uint8_t, 32> unk;
			void read(IBinaryArchive& fp) {
				fp.memBlock(unk.data(), 1, unk.size());
			}
		};
		Vector<UnkStr> verticies;
		fp.serializeNdVectorExternal(verticies);
		vertexBuffer = createVertexBuffer(verticies.data(), verticies.size() * sizeof(UnkStr), VertexBufferOptions::BUFFER_STATIC);

		Vector<uint16_t> indicies;
		fp.serializeNdVectorExternal_pod(indicies);
		indexBuffer = createVertexBuffer(indicies.data(), indicies.size() * sizeof(uint16_t), VertexBufferOptions::BUFFER_STATIC, GL_ELEMENT_ARRAY_BUFFER);
	} else {
		//TODO
		SDL_assert_release(false);
	}
	
	//TODO
}

void SplineLoftHiRes::draw() {
	vertexBuffer->bind();
	indexBuffer->bind();

	glEnableVertexAttribArray(0);
	CHECK_GL_ERROR();

	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,  // type
		GL_FALSE,           // normalized?
		32,  // stride
		(void*)0            // array buffer offset
	);
	CHECK_GL_ERROR();

	glDrawElements(GL_POINTS, indexBuffer->size / sizeof(uint16_t), GL_UNSIGNED_SHORT, 0);
	CHECK_GL_ERROR();

	glDisableVertexAttribArray(0);
	CHECK_GL_ERROR();
}

void LoftShape::open(IBinaryArchive& fp) {
	uint32_t magic = 1280263764;
	fp.serialize(magic);
	SDL_assert_release(magic == 1280263764);

	uint32_t version = 24;
	fp.serialize(version);
	SDL_assert_release(version == 24);
}
