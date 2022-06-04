#include "renderSystem.h"
//#include "AnimationSystem.h"
#include "../GameManager.h"
#include "../EcsCore/Coordinator.h"


using namespace glm;
extern Coordinator gCoordinator;

particleSys* sprayParticleSys;
shared_ptr<Program> partProg;
shared_ptr<Texture> particleTexture;

vec3 worldShift = vec3(-0.5f, 0, -0.5f);

vec4 Left, Right, Bottom, Top, Near, Far;
vec4 planes[6];

bool debugCullCount = false;

int objCount;
int cullCount;

Tree::TreeNode tree;

/* ============== GROUND ============== */

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
int g_GiboLen;
// ground VAO
GLuint GroundVertexArrayID;

void initGround(float grndSize) {
	float g_groundSize = grndSize / 2.0 + 2;
	float g_groundY = -0.25;

	// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
	float GrndPos[] = {
		-g_groundSize, g_groundY, -g_groundSize,
		-g_groundSize, g_groundY,  g_groundSize,
		g_groundSize, g_groundY,  g_groundSize,
		g_groundSize, g_groundY, -g_groundSize
	};

	float GrndNorm[] = {
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0
	};

	GLfloat num_tex = g_groundSize / 10;
	static GLfloat GrndTex[] = {
		0, 0, // back
		0, num_tex,
		num_tex, num_tex,
		num_tex, 0 };

	unsigned short idx[] = { 0, 1, 2, 0, 2, 3 };
	//generate the ground VAO
	glGenVertexArrays(1, &GroundVertexArrayID);
	glBindVertexArray(GroundVertexArrayID);

	g_GiboLen = 6;
	glGenBuffers(1, &GrndBuffObj);
	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

	glGenBuffers(1, &GrndNorBuffObj);
	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

	glGenBuffers(1, &GrndTexBuffObj);
	glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

	glGenBuffers(1, &GIndxBuffObj);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
}

void RenderSys::init(float grndSize, shared_ptr<Program> ptProg, shared_ptr<Texture> ptTex)
{
	initGround(grndSize);
	sprayParticleSys = new particleSys(vec3(0, 5, 0), 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 0.4f); // start off screen
	sprayParticleSys->setnumP(90);
	sprayParticleSys->gpuSetup();

	partProg = ptProg;
	particleTexture = ptTex;
}

void SetModel(vec3 trans, float rotZ, float rotY, float rotX, vec3 sc, shared_ptr<Program> curS) {
	mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
	mat4 RotX = glm::rotate(glm::mat4(1.0f), rotX, vec3(1, 0, 0));
	mat4 RotY = glm::rotate(glm::mat4(1.0f), rotY, vec3(0, 1, 0));
	mat4 RotZ = glm::rotate(glm::mat4(1.0f), rotZ, vec3(0, 0, 1));
	mat4 ScaleS = glm::scale(glm::mat4(1.0f), sc);
	mat4 ctm = Trans * RotX * RotY * RotZ * ScaleS;
	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
}

void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
}

void SetModelLookAt(vec3 trans, float rotZ, float rotY, float rotX, vec3 sc, shared_ptr<Program> curS, mat4 _look) {
	mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
	mat4 RotX = glm::rotate(glm::mat4(1.0f), rotX, vec3(1, 0, 0));
	mat4 RotY = glm::rotate(glm::mat4(1.0f), rotY, vec3(0, 1, 0));
	mat4 RotZ = glm::rotate(glm::mat4(1.0f), rotZ, vec3(0, 0, 1));
	mat4 ScaleS = glm::scale(glm::mat4(1.0f), sc);
	mat4 ctm = Trans * RotX * RotY * RotZ * ScaleS * _look;
	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
}

mat4 lookDirToMat(vec3 lookDir) {
	return glm::lookAt(vec3(0.0), glm::normalize(vec3(-lookDir.x, lookDir.y, lookDir.z)), vec3(0, 1, 0));
}
void setModelRC(shared_ptr<Program> curS, Transform* tr) {

	mat4 Trans = glm::translate(glm::mat4(1.0f), tr->pos + worldShift);
	mat4 ScaleS = glm::scale(glm::mat4(1.0f), tr->scale);
	mat4 RotX = glm::rotate(glm::mat4(1.0f), tr->rotation.x, vec3(1, 0, 0));
	mat4 RotY = glm::rotate(glm::mat4(1.0f), tr->rotation.y, vec3(0, 1, 0));
	mat4 RotZ = glm::rotate(glm::mat4(1.0f), tr->rotation.z, vec3(0, 0, 1));
	mat4 RampRotX = glm::rotate(glm::mat4(1.0f), tr->rampRotation.x, vec3(1, 0, 0));
	mat4 RampRotY = glm::rotate(glm::mat4(1.0f), tr->rampRotation.y, vec3(0, 1, 0));
	mat4 RampRotZ = glm::rotate(glm::mat4(1.0f), tr->rampRotation.z, vec3(0, 0, 1));
	//if (tr->rampRotation.x != 0 || tr->rampRotation.z != 0) {
	//	cout << tr->rampRotation.x << endl;
	//	cout << tr->rampRotation.z << endl << endl;
	//}
	mat4 ctm = Trans * RampRotX * RampRotY * RampRotZ * lookDirToMat(tr->lookDir) * RotX * RotY * RotZ * ScaleS;
	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm)); 
}
//
//void setModelRC_Origin(shared_ptr<Program> curS, Transform* tr) {
//
//	mat4 Trans = glm::translate(glm::mat4(1.0f), tr->pos);
//	mat4 ScaleS = glm::scale(glm::mat4(1.0f), tr->scale);
//	mat4 RotX = glm::rotate(glm::mat4(1.0f), tr->rotation.x, vec3(1, 0, 0));
//	mat4 RotY = glm::rotate(glm::mat4(1.0f), tr->rotation.y, vec3(0, 1, 0));
//	mat4 RotZ = glm::rotate(glm::mat4(1.0f), tr->rotation.z, vec3(0, 0, 1));
//	mat4 ctm = Trans * lookDirToMat(tr->lookDir) * RotX * RotY * RotZ * ScaleS;
//	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
//}

void setModelRC_Origin(shared_ptr<Program> curS, Transform* tr) {

	mat4 Trans = glm::translate(glm::mat4(1.0f), tr->pos);
	mat4 ScaleS = glm::scale(glm::mat4(1.0f), tr->scale);
	mat4 RotX = glm::rotate(glm::mat4(1.0f), tr->rotation.x, vec3(1, 0, 0));
	mat4 RotY = glm::rotate(glm::mat4(1.0f), tr->rotation.y, vec3(0, 1, 0));
	mat4 RotZ = glm::rotate(glm::mat4(1.0f), tr->rotation.z, vec3(0, 0, 1));
	mat4 ctm = Trans * lookDirToMat(tr->lookDir) * RotX * RotY * RotZ * ScaleS;
	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
}

void RenderSys::draw(shared_ptr<MatrixStack> Projection, mat4 View, RenderComponent* rc, Transform* tr, GLuint depthMap, mat4 LSpace, bool isGrey)
{
	//only extract the planes for the game camaera
	ExtractVFPlanes(Projection->topMatrix(), View);

	shared_ptr<Program> curS = rc->shader;
	glCullFace(rc->cullDir);
	curS->bind();
	glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
	glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
	glUniform1f(curS->getUniform("alpha"), rc->transparency);
	glUniform3f(curS->getUniform("lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(curS->getUniform("shadowDepth"), 1);
  	//glUniform3f(ShadowProg->getUniform("lightDir"), g_light.x, g_light.y, g_light.z);
	glUniformMatrix4fv(curS->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace));
	setModelRC(curS, tr);

	bool useCubeMap = false;
	if (rc->texID != 999)
	{
		//TODO: send useCube to gpu
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, rc->texID);
		useCubeMap = true;
	}
	glUniform1i(curS->getUniform("useCubeTex"), useCubeMap);
	glUniform1i(curS->getUniform("isGrey"), isGrey);
	//check to cull
	if (!ViewFrustCull(tr->pos, 4.0f) || curS->getFShaderName() == "../resources/cube_frag.glsl") {
		// non-textured shapes draw
		if ((rc->sg)->textures.size() == 0 || useCubeMap)
		{
			for (int i = 0; i < (rc->sg)->shapes.size(); i++) {
				(rc->sg)->shapes[i]->draw(curS);
			}
		}
		else {
			// textured shapes draw
			for (int i = 0; i < (rc->sg)->shapes.size(); i++) {
				(rc->sg)->textures[i]->bind(curS->getUniform("Texture0"));
				(rc->sg)->shapes[i]->draw(curS);
			}
		}
	}
	curS->unbind();
		
}




Tree::TreeNode RenderSys::getTree() {
	//cout << "getting tree" << endl;
	if ( tree.children.empty() ) {
		//cout << "making tree!" << endl;
		tree = Tree::initTree(mEntities);
		//cout << "Tree radius: " << tree.radius << endl;
		//cout << "Tree next root: " << endl;
		for (Tree::TreeNode tn : tree.children){
			//cout<<"TN: " << tn.pos.x << " " << tn.pos.z << " rad: " << tn.radius <<endl;
		}
	}
	return tree;
}

// mat4 GetProjectionMatrix() {
//     	int width, height;
//     	glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
//     	float aspect = width/(float)height;
//     	mat4 Projection = perspective(radians(50.0f), aspect, 0.1f, 200.0f);
//     	return Projection;
// }

// /* helper functions for transforms */
// mat4 GetView(shared_ptr<Program>  shader) {
// 	glm::mat4 Cam = glm::lookAt(g_eye, g_lookAt, vec3(0, 1, 0));
// 	return Cam;
// }

#pragma region Animation
const char* vertexShaderSource = R"(
	#version 440 core
	layout (location = 0) in vec3 position; 
	layout (location = 1) in vec3 normal;
	layout (location = 2) in vec2 uv;
	layout (location = 3) in vec4 boneIds;
	layout (location = 4) in vec4 boneWeights;
	out vec2 tex_cord;
	out vec3 v_normal;
	out vec3 v_pos;
	out vec4 bw;
	uniform mat4 bone_transforms[50];
	uniform mat4 view_projection_matrix;
	uniform mat4 model_matrix;
	void main()
	{
		bw = vec4(0);
		if(int(boneIds.x) == 1)
		bw.z = boneIds.x;
		//boneWeights = normalize(boneWeights);
		mat4 boneTransform  =  mat4(0.0);
		boneTransform  +=    bone_transforms[int(boneIds.x)] * boneWeights.x;
		boneTransform  +=    bone_transforms[int(boneIds.y)] * boneWeights.y;
		boneTransform  +=    bone_transforms[int(boneIds.z)] * boneWeights.z;
		boneTransform  +=    bone_transforms[int(boneIds.w)] * boneWeights.w;
		vec4 pos =boneTransform * vec4(position, 1.0);
		gl_Position = view_projection_matrix * model_matrix * pos;
		v_pos = vec3(model_matrix * boneTransform * pos);
		tex_cord = uv;
		v_normal = mat3(transpose(inverse(model_matrix * boneTransform))) * normal;
		v_normal = normalize(v_normal);
	}
	)";

const char* fragmentShaderSource = R"(
	#version 440 core
	in vec2 tex_cord;
	in vec3 v_normal;
	in vec3 v_pos;
	in vec4 bw;
	out vec4 color;
	uniform sampler2D diff_texture;
	vec3 lightPos = vec3(0.2, 1.0, -3.0);
	
	void main()
	{
		vec3 lightDir = normalize(lightPos - v_pos);
		float diff = max(dot(v_normal, lightDir), 0.2);
		vec3 dCol = diff * texture(diff_texture, tex_cord).rgb; 
		color = vec4(dCol, 1);
	}
	)";

std::pair<uint, float> getTimeFraction(std::vector<float>& times, float& dt) {
	uint segment = 0;
	while (dt > times[segment])
		segment++;
	float start = times[segment - 1];
	float end = times[segment];
	float frac = (dt - start) / (end - start);
	return { segment, frac };
}


glm::mat4 convertMatrix(const aiMatrix4x4& aiMat)
{
	return {
	aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
	aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
	aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
	aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
	};
}

void getPose(Animation& animation, Bone& skeleton, float dt, std::vector<glm::mat4>& output, glm::mat4& parentTransform, glm::mat4& globalInverseTransform) {
	cout << "1\n";
	BoneTransformTrack& btt = animation.boneTransforms[skeleton.name];
	dt = fmod(dt, animation.duration);
	std::pair<uint, float> fp;
	//calculate interpolated position
	cout << "2\n";
	fp = getTimeFraction(btt.positionTimestamps, dt);
	cout << "2.5\n";

	glm::vec3 position1 = btt.positions[fp.first - 1];
	glm::vec3 position2 = btt.positions[fp.first];

	glm::vec3 position = glm::mix(position1, position2, fp.second);

	//calculate interpolated rotation
	cout << "3\n";
	fp = getTimeFraction(btt.rotationTimestamps, dt);
	cout << "4\n";
	glm::quat rotation1 = btt.rotations[fp.first - 1];
	glm::quat rotation2 = btt.rotations[fp.first];

	glm::quat rotation = glm::slerp(rotation1, rotation2, fp.second);

	//calculate interpolated scale
	cout << "5\n";
	fp = getTimeFraction(btt.scaleTimestamps, dt);
	cout << "6\n";
	glm::vec3 scale1 = btt.scales[fp.first - 1];
	glm::vec3 scale2 = btt.scales[fp.first];

	glm::vec3 scale = glm::mix(scale1, scale2, fp.second);

	glm::mat4 positionMat = glm::mat4(1.0),
		scaleMat = glm::mat4(1.0);


	// calculate localTransform
	positionMat = glm::translate(positionMat, position);
	glm::mat4 rotationMat = toMat4(rotation); // toMat4 >> mat4_cast
	scaleMat = glm::scale(scaleMat, scale);
	glm::mat4 localTransform = positionMat * rotationMat * scaleMat;
	glm::mat4 globalTransform = parentTransform * localTransform;
	cout << "7\n";
	output[skeleton.id] = globalInverseTransform * globalTransform * skeleton.offset;
	//update values for children bones
	cout << "8\n";
	for (Bone& child : skeleton.children) {
		getPose(animation, child, dt, output, globalTransform, globalInverseTransform);
	}
	cout << "9\n";
	//std::cout << dt << " => " << position.x << ":" << position.y << ":" << position.z << ":" << std::endl;
}

void RenderSys::drawSkeletal(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, shared_ptr<Texture> tex, float elapsedTime, SkeletalComponent sc) {
	//for (Entity const& entity : mEntities) {
	//	SkeletalComponent& sc = gCoordinator.GetComponent<SkeletalComponent>(entity);
	cout << "creating shader\n";
	uint shader = createShader(vertexShaderSource, fragmentShaderSource);
	cout << "made shader\n";

	// get all shader uniform locations
	uint viewProjectionMatrixLocation = glGetUniformLocation(shader, "view_projection_matrix");
	uint modelMatrixLocation = glGetUniformLocation(shader, "model_matrix");
	uint boneMatricesLocation = glGetUniformLocation(shader, "bone_transforms");
	uint textureLocation = glGetUniformLocation(shader, "diff_texture");

	// initialize projection view and model matrix
	//glm::mat4 projectionMatrix = glm::perspective(75.0f, (float)windowWidth / windowHeight, 0.01f, 100.0f);

	//glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.2f, -5.0f)
	//	, glm::vec3(0.0f, .0f, 0.0f),
	//	glm::vec3(0, 1, 0));
	//glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

	glm::mat4 modelMatrix(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(.2f, .2f, .2f));


	//update loop
	//while (true) { // render loop call
		//SDL_Event ev;
		//while (SDL_PollEvent(&ev)) {
		//	if (ev.type == SDL_QUIT)
		//		isRunning = false;
		//}

		//float elapsedTime = (float)SDL_GetTicks() / 1000;
	cout << "Math part\n";
	float dAngle = elapsedTime * 0.002;
	modelMatrix = glm::rotate(modelMatrix, dAngle, glm::vec3(0, 1, 0));
	glm::mat4 identity(1.0);
	cout << "getting pose\n";
	getPose(sc.animation, sc.skeleton, elapsedTime, sc.currentPose, identity, sc.globalInverseTransform);
	cout << "gl 1\n";
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shader);
	glUniformMatrix4fv(viewProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix));
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(boneMatricesLocation, sc.boneCount, GL_FALSE, glm::value_ptr(sc.currentPose[0]));
	cout << "gl 2\n";
	glBindVertexArray(sc.vao);
	glActiveTexture(GL_TEXTURE0);
	uint diffuseTexture = tex->getUnit(); // chase added
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glUniform1i(textureLocation, 0);
	cout << "gl 3 \n";
	glDrawElements(GL_TRIANGLES, sc.indices.size(), GL_UNSIGNED_INT, 0);
	cout << "gl done\n";

	//SDL_GL_SwapWindow(window);
//	}

	//cleanup
	//SDL_GLContext context = SDL_GL_GetCurrentContext();
	//SDL_GL_DeleteContext(context);
	//SDL_DestroyWindow(window);
	//SDL_Quit();


	//return 0;
//	}
}
#pragma endregion

void RenderSys::update(shared_ptr<MatrixStack> Projection, mat4 View, GLuint depthMap, mat4 LSpace, bool isGrey)
{
	cullCount = 0;
	objCount = 0;
	vector<Entity> transparentEnts;

	Tree::TreeNode hierTree = RenderSys::getTree();
	for (Entity const& entity : mEntities) {
		RenderComponent& rc = gCoordinator.GetComponent<RenderComponent>(entity);
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		//if (rc.isSkeletal) {
		//	cout << "grabbing skeletal\n";
		//	SkeletalComponent& sc = gCoordinator.GetComponent<SkeletalComponent>(entity);
		//	cout << "drawing skeletal\n";
		//	drawSkeletal(Projection->topMatrix(), View, rc.sg->textures[0], gameTime, sc);
		//	cout << "skeletal produced\n";
		//	continue;
		//}

		if (rc.transparency < 1.0) {
			transparentEnts.push_back(entity);
		}
		else {
			draw(Projection, View, &rc, &tr, depthMap, LSpace, isGrey);
		}
	}
	drawSprayParticles(View, Projection->topMatrix(), mat4(1.0));
	// draw all transparent entities second
	for (Entity entity : transparentEnts) {
		RenderComponent& rc = gCoordinator.GetComponent<RenderComponent>(entity);
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		draw(Projection, View, &rc ,&tr, depthMap, LSpace, isGrey);
	}
	
	if (debugCullCount) {
		cout << "cull count: " << cullCount << endl;
		cout << "object Count: " << objCount << endl;
	}
}

void RenderSys::drawDepth(shared_ptr<Program> curS) {
	vector<Entity> transparentEnts;
	for (Entity const& entity : mEntities) {
		RenderComponent& rc = gCoordinator.GetComponent<RenderComponent>(entity);
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		if (rc.transparency < 1.0) {
			transparentEnts.push_back(entity);
		}
		else {
			drawShadows(&rc, &tr, curS);
		}
	}
	// draw all transparent entities second
	for (Entity entity : transparentEnts) {
		RenderComponent& rc = gCoordinator.GetComponent<RenderComponent>(entity);
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		drawShadows(&rc, &tr, curS);
	}
}

void RenderSys::drawShadows(RenderComponent* rc, Transform* tr, shared_ptr<Program> curS)
{
	setModelRC(curS, tr);

	for (int i = 0; i < (rc->sg)->shapes.size(); i++) {
			(rc->sg)->shapes[i]->draw(curS);
	}
}

void RenderSys::ExtractVFPlanes(mat4 P, mat4 V) {

  /* composite matrix */
  mat4 comp = P*V;
  vec3 n; //use to pull out normal
  float l; //length of normal for plane normalization

  Left.x = comp[0][3] + comp[0][0]; 
  Left.y = comp[1][3] + comp[1][0]; 
  Left.z = comp[2][3] + comp[2][0]; 
  Left.w = comp[3][3] + comp[3][0];
  n = vec3(Left.x, Left.y, Left.z);
  l = length(n);
  planes[0] = Left = Left/l;
  //normalize plane eq for every plane
  //cout << "Left' " << Left.x << " " << Left.y << " " <<Left.z << " " << Left.w << endl;
  
  Right.x = comp[0][3] - comp[0][0];
  Right.y = comp[1][3] - comp[1][0];
  Right.z = comp[2][3] - comp[2][0];
  Right.w = comp[3][3] - comp[3][0];
  n = vec3(Right.x, Right.y, Right.z);
  l = length(n);
  planes[1] = Right = Right/l;
  //cout << "Right " << Right.x << " " << Right.y << " " <<Right.z << " " << Right.w << endl;

  Bottom.x = comp[0][3] + comp[0][1];
  Bottom.y = comp[1][3] + comp[1][1];
  Bottom.z = comp[2][3] + comp[2][1];
  Bottom.w = comp[3][3] + comp[3][1];
  n = vec3(Bottom.x, Bottom.y, Bottom.z);
  l = length(n);
  planes[2] = Bottom = Bottom/l;
  //cout << "Bottom " << Bottom.x << " " << Bottom.y << " " <<Bottom.z << " " << Bottom.w << endl;
  
  Top.x = comp[0][3] - comp[0][1];
  Top.y = comp[1][3] - comp[1][1];
  Top.z = comp[2][3] - comp[2][1];
  Top.w = comp[3][3] - comp[3][1];
  n = vec3(Top.x, Top.y, Top.z);
  l = length(n);
  planes[3] = Top = Top/l;
  //cout << "Top " << Top.x << " " << Top.y << " " <<Top.z << " " << Top.w << endl;

  Near.x = comp[0][2];
  Near.y = comp[1][2];
  Near.z = comp[2][2];
  Near.w = comp[3][2];
  n = vec3(Near.x, Near.y, Near.z);
  l = length(n);
  planes[4] = Near = Near/l;
  //cout << "Near " << Near.x << " " << Near.y << " " <<Near.z << " " << Near.w << endl;

  Far.x = comp[0][3] - comp[0][2];
  Far.y = comp[1][3] - comp[1][2];
  Far.z = comp[2][3] - comp[2][2];
  Far.w = comp[3][3] - comp[3][2];
  n = vec3(Far.x, Far.y, Far.z);
  l = length(n);
  planes[5] = Far = Far/l;
  //cout << "Far " << Far.x << " " << Far.y << " " <<Far.z << " " << Far.w << endl;
}

/* helper function to compute distance to the plane */
// ASSUMPTION input is normalized
float RenderSys::DistToPlane(float A, float B, float C, float D, vec3 point) {
  return A*point.x + B*point.y + C*point.z + D;
}

/* Actual cull on planes */
//returns 1 to CULL
int RenderSys::ViewFrustCull(vec3 center, float radius) {

  	float dist;
  	objCount++;
	for (int i=0; i < 6; i++) {
		dist = DistToPlane(planes[i].x, planes[i].y, planes[i].z, planes[i].w, center);
		if (dist < (-1)*radius){
			cullCount++;
			return 1;
		}
	}
	return 0; 
}

void RenderSys::drawSprayParticles(mat4 view, mat4 projection, mat4 model) {
	sprayParticleSys->setCamera(view);
	partProg->bind();
	particleTexture->bind(partProg->getUniform("alphaTexture"));
	CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(projection)));
	CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("V"), 1, GL_FALSE, value_ptr(view)));
	CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(model)));
	sprayParticleSys->drawMe(partProg);
	sprayParticleSys->update();
	partProg->unbind();
}

namespace RenderSystem {
	//code to draw the ground plane

	void drawGround(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, 
		mat4 View, shared_ptr<Texture> grassTexture, bool isGrey, GLuint depthMap) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		
		curS->bind();
		glUniform1i(curS->getUniform("useCubeTex"), false);
		glUniform1i(curS->getUniform("isGrey"), isGrey);
		//glUniform3f(texProg->getUniform("lightPos"), 20.0, 10.0, 70.9);
		//glUniform3f(curS->getUniform("lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glDepthFunc(GL_LEQUAL);
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		glCullFace(GL_BACK);

		shared_ptr<MatrixStack> Model = make_shared<MatrixStack>();
		Model->loadIdentity();
		Model->pushMatrix();
		glBindVertexArray(GroundVertexArrayID);

		grassTexture->bind(curS->getUniform("Texture0"));
		glUniform1f(curS->getUniform("alpha"), 1.0f);
		Model->translate(vec3(0, -1, 0));
		Model->scale(vec3(1, 1, 1));

		setModel(curS, Model);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// draw!
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		
		Model->popMatrix();

		curS->unbind();
	}
}
