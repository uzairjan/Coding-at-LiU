/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/tnm067lab2/processors/marchingtetrahedra.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

size_t MarchingTetrahedra::HashFunc::max = 1;

const ProcessorInfo MarchingTetrahedra::processorInfo_{
    "org.inviwo.MarchingTetrahedra",  // Class identifier
    "Marching Tetrahedra",            // Display name
    "TNM067",                         // Category
    CodeState::Experimental,          // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo MarchingTetrahedra::getProcessorInfo() const { return processorInfo_; }

MarchingTetrahedra::MarchingTetrahedra()
    : Processor()
    , volume_("volume")
    , mesh_("mesh")
    , isoValue_("isoValue", "ISO value", 0.5f, 0.0f, 1.0f) {

    addPort(volume_);
    addPort(mesh_);

    addProperty(isoValue_);

    isoValue_.setSerializationMode(PropertySerializationMode::All);

    volume_.onChange([&]() {
        if (!volume_.hasData()) {
            return;
        }
        NetworkLock lock(getNetwork());
        float iso = (isoValue_.get() - isoValue_.getMinValue()) /
                    (isoValue_.getMaxValue() - isoValue_.getMinValue());
        auto vr = volume_.getData()->dataMap_.valueRange;
        isoValue_.setMinValue(vr.x);
        isoValue_.setMaxValue(vr.y);
        isoValue_.setIncrement(glm::abs(vr.y - vr.x) / 50.0f);
        isoValue_.set(iso * (vr.y - vr.x) + vr.x);
        isoValue_.setCurrentStateAsDefault();
    });
}

void MarchingTetrahedra::process() {
    auto volume = volume_.getData()->getRepresentation<VolumeRAM>();
    MeshHelper mesh(volume_.getData());

    const auto dims = volume->getDimensions();
    MarchingTetrahedra::HashFunc::max = dims.x * dims.y * dims.z;

    float iso = isoValue_.get();

    util::IndexMapper3D index(dims);

    const static size_t tetrahedraIds[6][4] = {{0, 1, 2, 5}, {1, 3, 2, 5}, {3, 2, 5, 7},
                                               {0, 2, 4, 5}, {6, 4, 2, 5}, {6, 7, 5, 2}};

    size3_t pos;
    for (pos.z = 0; pos.z < dims.z - 1; ++pos.z) {
        for (pos.y = 0; pos.y < dims.y - 1; ++pos.y) {
            for (pos.x = 0; pos.x < dims.x - 1; ++pos.x) {
                // Step 1 (TASK 5): create current cell
                // Use volume->getAsDouble to query values from the volume
                // Spatial position should be between 0 and 1
                // The voxel index should be the 1D-index for the voxel
                Cell c;
				Voxel vxl;
				size_t idx = 0;
				for (size_t z = 0; z < 2; ++z) {
					for (size_t y = 0; y < 2; ++y) {
						for (size_t x = 0; x < 2; ++x) {
							
							vec3 globalPos(pos.x + x, pos.y + y, pos.z + z);
							vxl.pos = globalPos; // Spatial position [0, 1]???
							//vxl.pos = vec3(x, y, z); // Spatial position [0, 1]
							vxl.index = index(globalPos); // Get global index at vertice position 
							vxl.value = volume->getAsDouble(globalPos); // Get floating point value at global position
							c.voxels[idx] = vxl;
							idx++;
						}
					}
				}

                // Step 2 (TASK 6): Subdivide cell into tetrahedra (hint: use tetrahedraIds)
                std::vector<Tetrahedra> tetrahedras;
				Tetrahedra temp;

				// Loop over 6 tetrahedras
				for (size_t i = 0; i < 6; i++) {
					
					// Loop over the 4 ids and copy correct voxel from cell
					for (size_t j = 0; j < 4; j++) {
						temp.voxels[j] = c.voxels[tetrahedraIds[i][j]];
					}
					// Add tetrahedra to vector
					tetrahedras.push_back(temp);
				}

                for (const Tetrahedra& tetrahedra : tetrahedras) {
                    // Step 3 (TASK 7): Calculate for tetra case index

                    // Should give 0, 1 or 2 triangles depending on case
					int caseId = 0;
					// Find out if values in voxels are higher or lower than iso value and add binary to caseId
					if (tetrahedra.voxels[0].value < iso) caseId += 1;
					if (tetrahedra.voxels[1].value < iso) caseId += 2;
					if (tetrahedra.voxels[2].value < iso) caseId += 4;
					if (tetrahedra.voxels[3].value < iso) caseId += 8;
					

                    // Step 4 (TASK 7): Extract triangles

					// Store voxel information locally (just for easier names in later calculations)
					vec3 pos0 = tetrahedra.voxels[0].pos;
					vec3 pos1 = tetrahedra.voxels[1].pos;
					vec3 pos2 = tetrahedra.voxels[2].pos;
					vec3 pos3 = tetrahedra.voxels[3].pos;
					float val0 = tetrahedra.voxels[0].value;
					float val1 = tetrahedra.voxels[1].value;
					float val2 = tetrahedra.voxels[2].value;
					float val3 = tetrahedra.voxels[3].value;
					size_t idx0 = tetrahedra.voxels[0].index;
					size_t idx1 = tetrahedra.voxels[1].index;
					size_t idx2 = tetrahedra.voxels[2].index;
					size_t idx3 = tetrahedra.voxels[3].index;
								
					// Declare variables for interpolated values and their corresponding vertices
					vec3 interVal0, interVal1, interVal2, interVal3;
					size_t vert0, vert1, vert2, vert3;

					// Create triangles depending on case
					switch (caseId) {

					case 0:
					case 15: 
						// All voxels inside or outside - do nothing
						break;

					case 1:
					case 14:
						// Create 1 triangle: 0 || 1-3-2

						// Find interpolated values
						// value = origin + dir * dist	// dist = (iso - min) / (max - min)
						interVal0 = pos0 + (pos1 - pos0) * (iso - val0) / (val1 - val0); // 0-1
						interVal1 = pos0 + (pos3 - pos0) * (iso - val0) / (val3 - val0); // 0-3
						interVal2 = pos0 + (pos2 - pos0) * (iso - val0) / (val2 - val0); // 0-2

						// Add vertices for the interpolated values
						idx0 = mesh.addVertex(interVal0, tetrahedra.voxels[0].index, tetrahedra.voxels[1].index); // 0-1
						idx1 = mesh.addVertex(interVal1, tetrahedra.voxels[0].index, tetrahedra.voxels[3].index); // 0-3
						idx2 = mesh.addVertex(interVal2, tetrahedra.voxels[0].index, tetrahedra.voxels[2].index); // 0-2

						// Find out what part points outwards and add a triangle with new vertices with correct normal
						if (caseId == 1)
							mesh.addTriangle(idx0, idx1, idx2);
						else
							mesh.addTriangle(idx0, idx2, idx1);

						break;

					case 2:
					case 13:
						// Create 1 triangle: 1 || 0-2-3

						// Find interpolated values
						// value = origin + dir * dist	// dist = (iso - min) / (max - min)
						interVal0 = pos1 + (pos3 - pos1) * (iso - val1) / (val3 - val1); // 1-3
						interVal1 = pos1 + (pos0 - pos1) * (iso - val1) / (val0 - val1); // 1-0
						interVal2 = pos1 + (pos2 - pos1) * (iso - val1) / (val2 - val1); // 1-2

						// Add vertices for the interpolated values
						idx0 = mesh.addVertex(interVal0, tetrahedra.voxels[1].index, tetrahedra.voxels[3].index); // 1-3
						idx1 = mesh.addVertex(interVal1, tetrahedra.voxels[1].index, tetrahedra.voxels[0].index); // 1-0
						idx2 = mesh.addVertex(interVal2, tetrahedra.voxels[1].index, tetrahedra.voxels[2].index); // 1-2

						// Find out what part points outwards and add a triangle with new vertices with correct normal
						if (caseId == 2)
							mesh.addTriangle(idx0, idx1, idx2);
						else
							mesh.addTriangle(idx0, idx2, idx1);

						break;

					case 3:
					case 12:
						// Create 2 triangles: 0, 1 || 2, 3

						// Find interpolated values
						// value = origin + dir * dist	// dist = (iso - min) / (max - min)
						interVal0 = pos0 + (pos3 - pos0) * (iso - val0) / (val3 - val0); // 0-3
						interVal1 = pos0 + (pos2 - pos0) * (iso - val0) / (val2 - val0); // 0-2
						interVal2 = pos1 + (pos2 - pos1) * (iso - val1) / (val2 - val1); // 1-2
						interVal3 = pos1 + (pos3 - pos1) * (iso - val1) / (val3 - val1); // 1-3

						// Add vertices for the interpolated values
						idx0 = mesh.addVertex(interVal0, tetrahedra.voxels[0].index, tetrahedra.voxels[3].index); // 0-3
						idx1 = mesh.addVertex(interVal1, tetrahedra.voxels[0].index, tetrahedra.voxels[2].index); // 0-2
						idx2 = mesh.addVertex(interVal2, tetrahedra.voxels[1].index, tetrahedra.voxels[2].index); // 1-2
						idx3 = mesh.addVertex(interVal3, tetrahedra.voxels[1].index, tetrahedra.voxels[3].index); // 1-3

						// Find out what part points outwards and add a triangle with new vertices with correct normal
						if (caseId == 3) {
							mesh.addTriangle(idx0, idx1, idx2);
							mesh.addTriangle(idx0, idx2, idx3);
						}
						else {
							mesh.addTriangle(idx0, idx3, idx2);
							mesh.addTriangle(idx0, idx2, idx1);
						}

						break;

					case 4:
					case 11:
						// Create 1 triangle: 2 || 0-3-1

						// Find interpolated values
						// value = origin + dir * dist	// dist = (iso - min) / (max - min)
						interVal0 = pos2 + (pos0 - pos2) * (iso - val2) / (val0 - val2); // 2-0
						interVal1 = pos2 + (pos3 - pos2) * (iso - val2) / (val3 - val2); // 2-3
						interVal2 = pos2 + (pos1 - pos2) * (iso - val2) / (val1 - val2); // 2-1

						// Add vertices for the interpolated values
						idx0 = mesh.addVertex(interVal0, tetrahedra.voxels[2].index, tetrahedra.voxels[0].index); // 2-0
						idx1 = mesh.addVertex(interVal1, tetrahedra.voxels[2].index, tetrahedra.voxels[3].index); // 2-3
						idx2 = mesh.addVertex(interVal2, tetrahedra.voxels[2].index, tetrahedra.voxels[1].index); // 2-1

						// Find out what part points outwards and add a triangle with new vertices with correct normal
						if (caseId == 4)
							mesh.addTriangle(idx0, idx1, idx2);
						else
							mesh.addTriangle(idx0, idx2, idx1);

						break;

					case 5:
					case 10:
						// Create 2 triangles: 0, 2 || 1, 3

						// Find interpolated values
						// value = origin + dir * dist	// dist = (iso - min) / (max - min)
						interVal0 = pos0 + (pos3 - pos0) * (iso - val0) / (val3 - val0); // 0-3
						interVal1 = pos2 + (pos3 - pos2) * (iso - val2) / (val3 - val2); // 2-3
						interVal2 = pos2 + (pos1 - pos2) * (iso - val2) / (val1 - val2); // 2-1
						interVal3 = pos0 + (pos1 - pos0) * (iso - val0) / (val1 - val0); // 0-1

						// Add vertices for the interpolated values
						idx0 = mesh.addVertex(interVal0, tetrahedra.voxels[0].index, tetrahedra.voxels[3].index); // 0-3
						idx1 = mesh.addVertex(interVal1, tetrahedra.voxels[2].index, tetrahedra.voxels[3].index); // 2-3
						idx2 = mesh.addVertex(interVal2, tetrahedra.voxels[2].index, tetrahedra.voxels[1].index); // 2-1
						idx3 = mesh.addVertex(interVal3, tetrahedra.voxels[0].index, tetrahedra.voxels[1].index); // 0-1

						// Find out what part points outwards and add a triangle with new vertices with correct normal
						if (caseId == 5) {
							mesh.addTriangle(idx0, idx1, idx2);
							mesh.addTriangle(idx0, idx2, idx3);
						}
						else {
							mesh.addTriangle(idx0, idx3, idx2);
							mesh.addTriangle(idx0, idx2, idx1);
						}

						break;

					case 6: 
					case 9:
						// Create 2 triangles: 0, 3 || 1, 2

						// Find interpolated values
						// value = origin + dir * dist	// dist = (iso - min) / (max - min)
						interVal0 = pos0 + (pos2 - pos0) * (iso - val0) / (val2 - val0); // 0-2
						interVal1 = pos0 + (pos1 - pos0) * (iso - val0) / (val1 - val0); // 0-1
						interVal2 = pos1 + (pos3 - pos1) * (iso - val1) / (val3 - val1); // 1-3
						interVal3 = pos2 + (pos3 - pos2) * (iso - val2) / (val3 - val2); // 2-3

						// Add vertices for the interpolated values
						idx0 = mesh.addVertex(interVal0, tetrahedra.voxels[0].index, tetrahedra.voxels[2].index); // 0-2
						idx1 = mesh.addVertex(interVal1, tetrahedra.voxels[0].index, tetrahedra.voxels[1].index); // 0-1
						idx2 = mesh.addVertex(interVal2, tetrahedra.voxels[1].index, tetrahedra.voxels[3].index); // 1-3
						idx3 = mesh.addVertex(interVal3, tetrahedra.voxels[2].index, tetrahedra.voxels[3].index); // 2-3

						// Find out what part points outwards and add a triangle with new vertices with correct normal
						if (caseId == 9) {
							mesh.addTriangle(idx0, idx1, idx2);
							mesh.addTriangle(idx0, idx2, idx3);
						}
						else {
							mesh.addTriangle(idx0, idx3, idx2);
							mesh.addTriangle(idx0, idx2, idx1);
						}

						break;

					case 7:
					case 8:
						// Create 1 triangle: 3 || 0, 1, 2

						// Find interpolated values
						// value = origin + dir * dist	// dist = (iso - min) / (max - min)
						interVal0 = pos3 + (pos0 - pos3) * (iso - val3) / (val0 - val3); // 3-0
						interVal1 = pos3 + (pos1 - pos3) * (iso - val3) / (val1 - val3); // 3-1
						interVal2 = pos3 + (pos2 - pos3) * (iso - val3) / (val2 - val3); // 3-2

						// Add vertices for the interpolated values
						idx0 = mesh.addVertex(interVal0, tetrahedra.voxels[3].index, tetrahedra.voxels[0].index); // 3-0
						idx1 = mesh.addVertex(interVal1, tetrahedra.voxels[3].index, tetrahedra.voxels[1].index); // 3-1
						idx2 = mesh.addVertex(interVal2, tetrahedra.voxels[3].index, tetrahedra.voxels[2].index); // 3-2

						// Find out what part points outwards and add a triangle with new vertices with correct normal
						if (caseId == 8)
							mesh.addTriangle(idx0, idx1, idx2);
						else
							mesh.addTriangle(idx0, idx2, idx1);

						break;
					}
				}
            }
        }
    }

    mesh_.setData(mesh.toBasicMesh());
}

MarchingTetrahedra::MeshHelper::MeshHelper(std::shared_ptr<const Volume> vol)
    : edgeToVertex_()
    , vertices_()
    , mesh_(std::make_shared<BasicMesh>())
    , indexBuffer_(mesh_->addIndexBuffer(DrawType::Triangles, ConnectivityType::None)) {
    mesh_->setModelMatrix(vol->getModelMatrix());
    mesh_->setWorldMatrix(vol->getWorldMatrix());
}

void MarchingTetrahedra::MeshHelper::addTriangle(size_t i0, size_t i1, size_t i2) {
    ivwAssert(i0 != i1, "i0 and i1 should not be the same value");
    ivwAssert(i0 != i2, "i0 and i2 should not be the same value");
    ivwAssert(i1 != i2, "i1 and i2 should not be the same value");

    indexBuffer_->add(static_cast<glm::uint32_t>(i0));
    indexBuffer_->add(static_cast<glm::uint32_t>(i1));
    indexBuffer_->add(static_cast<glm::uint32_t>(i2));

    auto a = vertices_[i0].pos;
    auto b = vertices_[i1].pos;
    auto c = vertices_[i2].pos;

    vec3 n = glm::normalize(glm::cross(b - a, c - a));
    vertices_[i0].normal += n;
    vertices_[i1].normal += n;
    vertices_[i2].normal += n;
}

std::shared_ptr<BasicMesh> MarchingTetrahedra::MeshHelper::toBasicMesh() {
    for (auto& vertex : vertices_) {
        vertex.normal = glm::normalize(vertex.normal);
    }
    mesh_->addVertices(vertices_);
    return mesh_;
}

std::uint32_t MarchingTetrahedra::MeshHelper::addVertex(vec3 pos, size_t i, size_t j) {
    ivwAssert(i != j, "i and j should not be the same value");
    if (j < i) {
        return addVertex(pos, j, i);
    }

    auto edge = std::make_pair(i, j);

    auto it = edgeToVertex_.find(edge);

    if (it == edgeToVertex_.end()) {
        edgeToVertex_[edge] = vertices_.size();
        vertices_.push_back({pos, vec3(0, 0, 0), pos, vec4(0.7f, 0.7f, 0.7f, 1.0f)});
        return static_cast<std::uint32_t>(vertices_.size() - 1);
    }

    return static_cast<std::uint32_t>(it->second);
}

}  // namespace inviwo
