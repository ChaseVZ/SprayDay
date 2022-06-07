//#include <SDL.h>	
#include "AnimationSystem.h"

using namespace std;
using namespace glm;

extern Coordinator gCoordinator;

// a recursive function to read all bones and form skeleton
bool readSkeleton(Bone& boneOutput, aiNode* node, std::unordered_map<std::string, std::pair<int, glm::mat4>>& boneInfoTable) {

	if (boneInfoTable.find(node->mName.C_Str()) != boneInfoTable.end()) { // if node is actually a bone
		boneOutput.name = node->mName.C_Str();
		boneOutput.id = boneInfoTable[boneOutput.name].first;
		boneOutput.offset = boneInfoTable[boneOutput.name].second;

		for (int i = 0; i < node->mNumChildren; i++) {
			Bone child;
			readSkeleton(child, node->mChildren[i], boneInfoTable);
			boneOutput.children.push_back(child);
		}
		return true;
	}
	else { // find bones in children
		for (int i = 0; i < node->mNumChildren; i++) {
			if (readSkeleton(boneOutput, node->mChildren[i], boneInfoTable)) {
				return true;
			}

		}
	}
	return false;
}

void loadModel(const aiScene* scene, aiMesh* mesh, std::vector<Vertex>& verticesOutput, std::vector<uint>& indicesOutput, Bone& skeletonOutput, uint& nBoneCount) {

	verticesOutput = {};
	indicesOutput = {};

	//load position, normal, uv
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		//process position 
		Vertex vertex;
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;
		//process normal
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector;
		//process uv
		glm::vec2 vec;
		vec.x = mesh->mTextureCoords[0][i].x;
		vec.y = mesh->mTextureCoords[0][i].y;
		vertex.uv = vec;

		vertex.boneIds = glm::ivec4(0);
		vertex.boneWeights = glm::vec4(0.0f);

		verticesOutput.push_back(vertex);
	}

	//load boneData to vertices
	std::unordered_map<std::string, std::pair<int, glm::mat4>> boneInfo = {};
	std::vector<uint> boneCounts;
	boneCounts.resize(verticesOutput.size(), 0);
	nBoneCount = mesh->mNumBones;

	//loop through each bone
	for (uint i = 0; i < nBoneCount; i++) {
		aiBone* bone = mesh->mBones[i];
		glm::mat4 m = assimpToGlmMatrix(bone->mOffsetMatrix); //assimpToGlmMatrix >> convertMatrix
		boneInfo[bone->mName.C_Str()] = { i, m };

		//loop through each vertex that have that bone
		for (int j = 0; j < bone->mNumWeights; j++) {
			uint id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;
			boneCounts[id]++;
			//cout << "weight " << weight << endl;

			switch (boneCounts[id]) {
			case 1:
				verticesOutput[id].boneIds.x = i;
				verticesOutput[id].boneWeights.x = weight;
				break;
			case 2:
				verticesOutput[id].boneIds.y = i;
				verticesOutput[id].boneWeights.y = weight;
				break;
			case 3:
				verticesOutput[id].boneIds.z = i;
				verticesOutput[id].boneWeights.z = weight;
				break;
			case 4:
				verticesOutput[id].boneIds.w = i;
				verticesOutput[id].boneWeights.w = weight;
				break;
			default:
				//std::cout << "err: unable to allocate bone to vertex" << std::endl;
				break;

			}
		}
	}



	//normalize weights to make all weights sum 1
	for (int i = 0; i < verticesOutput.size(); i++) {
		glm::vec4& boneWeights = verticesOutput[i].boneWeights;
		float totalWeight = boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w;
		if (totalWeight > 0.0f) {
			verticesOutput[i].boneWeights = glm::vec4(
				boneWeights.x / totalWeight,
				boneWeights.y / totalWeight,
				boneWeights.z / totalWeight,
				boneWeights.w / totalWeight
			);
		}
	}


	//load indices
	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace& face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indicesOutput.push_back(face.mIndices[j]);
	}

	// create bone hirerchy
	readSkeleton(skeletonOutput, scene->mRootNode, boneInfo);
}

void loadAnimation(const aiScene* scene, Animation& animation) {
	//loading  first Animation
	if (scene->mNumAnimations == 0) { cout << "zero animations found\n"; return; }
	aiAnimation* anim = scene->mAnimations[0];
	//cout << scene->mNumMeshes << " meshes found\n";
	//cout <<  scene->mNumAnimations << " animations found\n";
	//cout << anim->mTicksPerSecond << " ticks found\n";
	//cout << anim->mDuration * anim->mTicksPerSecond << " is the duration found\n";

	if (anim->mTicksPerSecond != 0.0f)
		animation.ticksPerSecond = anim->mTicksPerSecond;
	else
		animation.ticksPerSecond = 1;


	animation.duration = anim->mDuration * anim->mTicksPerSecond;
	animation.boneTransforms = {};

	//load positions rotations and scales for each bone
	// each channel represents each bone
	for (int i = 0; i < anim->mNumChannels; i++) {
		aiNodeAnim* channel = anim->mChannels[i];
		//cout << i << " >> " << channel->mNumPositionKeys << endl;
		BoneTransformTrack track;
		for (int j = 0; j < channel->mNumPositionKeys; j++) {
			track.positionTimestamps.push_back(channel->mPositionKeys[j].mTime);
			track.positions.push_back(assimpToGlmVec3(channel->mPositionKeys[j].mValue));
		}
		for (int j = 0; j < channel->mNumRotationKeys; j++) {
			track.rotationTimestamps.push_back(channel->mRotationKeys[j].mTime);
			track.rotations.push_back(assimpToGlmQuat(channel->mRotationKeys[j].mValue)); //assimpToGlmQuat

		}
		for (int j = 0; j < channel->mNumScalingKeys; j++) {
			track.scaleTimestamps.push_back(channel->mScalingKeys[j].mTime);
			track.scales.push_back(assimpToGlmVec3(channel->mScalingKeys[j].mValue));

		}
		animation.boneTransforms[channel->mNodeName.C_Str()] = track;
	}
}

unsigned int createVertexArray(std::vector<Vertex>& vertices, std::vector<uint> indices) {
	uint
		vao = 0,
		vbo = 0,
		ebo = 0;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // vertTex standin
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, boneIds));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, boneWeights));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), &indices[0], GL_STATIC_DRAW);
	glBindVertexArray(0);
	return vao;
}

void AnimationSys::init(ShapeGroup* wolf, ShapeGroup* bear) {
	Assimp::Importer importer;

	//for (Entity const& entity : mEntities) {
	//	SkeletalComponent& sc = gCoordinator.GetComponent<SkeletalComponent>(entity);

	vector<ShapeGroup*> enemies = { wolf, bear };

	for (int i = 0; i < 2; i++){

		//load model file
		//const char* filePath = sc.filename;
		//const char* filePath = "../resources/Animation_Stuff/model.dae";
		const char* filePath = enemies[i]->filename.c_str();
		const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
		}
		aiMesh* mesh = scene->mMeshes[0];

		std::vector<Vertex> vertices = {};
		std::vector<uint> indices = {};
		uint boneCount = 0;
		Animation animation;
		uint vao = 0;
		Bone skeleton;
		//uint diffuseTexture;

		//as the name suggests just inverse the global transform
		//glm::mat4 globalInverseTransform = glm::inverse(convertMatrix(scene->mRootNode->mTransformation));
		glm::mat4 globalInverseTransform = assimpToGlmMatrix(scene->mRootNode->mTransformation);
		globalInverseTransform = glm::inverse(globalInverseTransform);

		loadModel(scene, mesh, vertices, indices, skeleton, boneCount);
		//cout << "num bones: " << boneCount << endl;
		//cout << "indicies size: " << indices.size();
		//cout << "\nvertices size: " << vertices.size();
		//cout << "\nskeley: " << skeleton.name << endl;
		//cout << animation.boneTransforms.size() << endl << endl;
		loadAnimation(scene, animation);

		vao = createVertexArray(vertices, indices);
		//diffuseTexture = createTexture("man/diffuse.png");

		glm::mat4 identity(1.0);

		//currentPose is held in this vector and uploaded to gpu as a matrix array uniform
		std::vector<glm::mat4> currentPose = {};
		currentPose.resize(boneCount, identity); // use this for no animation

		//sc.animation = animation;
		//sc.skeleton = skeleton;
		//sc.boneCount = boneCount;
		//sc.indices = indices;
		//sc.currentPose = currentPose;
		//sc.globalInverseTransform = globalInverseTransform;
		//sc.vao = vao;
		//sc.animDur = animation.duration / animation.ticksPerSecond;

		enemies[i]->animation = animation;
		enemies[i]->skeleton = skeleton;
		enemies[i]->boneCount = boneCount;
		enemies[i]->indices = indices;
		enemies[i]->currentPose = currentPose;
		enemies[i]->globalInverseTransform = globalInverseTransform;
		enemies[i]->vao = vao;
		enemies[i]->animDur = animation.duration / animation.ticksPerSecond;
	}
}



//uint createTexture(std::string filepath) {
//	uint textureId = 0;
//	int width, height, nrChannels;
//	byte* data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 4);
//	glGenTextures(1, &textureId);
//	glBindTexture(GL_TEXTURE_2D, textureId);
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
//
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
//
//	stbi_image_free(data);
//	glBindTexture(GL_TEXTURE_2D, 0);
//	return textureId;
//}

