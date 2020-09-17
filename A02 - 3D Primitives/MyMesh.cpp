#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader);

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));

	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue, -fValue, fValue); //0
	vector3 point1(fValue, -fValue, fValue); //1
	vector3 point2(fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue, -fValue, -fValue); //4
	vector3 point5(fValue, -fValue, -fValue); //5
	vector3 point6(fValue, fValue, -fValue); //6
	vector3 point7(-fValue, fValue, -fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// get half the height, used to center the pivot point (and the position) in the middle of the 3d cone
	float halfHeight = a_fHeight / 2;

	// the bottom vertex of the cone (base of cone)
	vector3 bottom = vector3(0, 0, -halfHeight);
	// the top vertex of the cone (point of cone)
	vector3 top = vector3(0, 0, halfHeight);

	// the vertices
	std::vector<vector3> vertices;

	// get the vertices for the base of the cone
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		float rads = (PI * 2) * ((float)i / (float)a_nSubdivisions);
		float xLoc = cos(rads) * a_fRadius;
		float yLoc = sin(rads) * a_fRadius;
		vertices.push_back(vector3(xLoc, yLoc, bottom.z));
	}

	// draw the triangles for the base of the cone
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddTri(bottom, vertices[i], vertices[(i + 1) % a_nSubdivisions]);
	}

	// reset vertices for the top part of the cone
	vertices.clear();

	// create the vertices for the top part of the cone (the point)
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		float rads = (PI * 2) * ((float)i / (float)a_nSubdivisions);
		float xLoc = cos(rads) * a_fRadius;
		float yLoc = sin(rads) * a_fRadius;
		vertices.push_back(vector3(xLoc, yLoc, bottom.z));
	}

	// draw the triangle for the top part of the cone
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddTri(top, vertices[(i + 1) % a_nSubdivisions], vertices[i]);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// get half the height, used to center the pivot point (and the position) in the middle of the 3d cylinder
	float halfHeight = a_fHeight / 2;

	// the vertices
	std::vector<vector3> vertices;

	std::vector<float> rads;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		rads.push_back((PI * 2) * ((float)i / (float)a_nSubdivisions));
	}

	vector3 baseTop = vector3(0, 0, halfHeight);
	vector3 baseBottom = vector3(0, 0, -halfHeight);

	// draw both bases of the cylinder
	for (int side = 0; side < 2; side++)
	{
		float baseHeight = side == 0 ? -halfHeight : halfHeight;
		vector3 baseVec = vector3(0, 0, baseHeight);

		for (int i = 0; i < a_nSubdivisions; i++)
		{
			float xLoc = cos(rads[i]) * a_fRadius;
			float yLoc = sin(rads[i]) * a_fRadius;
			vertices.push_back(vector3(xLoc, yLoc, baseVec.z));
		}

		for (int i = 0; i < a_nSubdivisions; i++)
		{
			// reverse drawning order depending on the base of the cylinder
			if (side == 1)
				AddTri(baseVec, vertices[i], vertices[(i + 1) % a_nSubdivisions]);
			else
				AddTri(baseVec, vertices[(i + 1) % a_nSubdivisions], vertices[i]);
		}
		vertices.clear();
	}

	// reset vertices for the side parts of the cylinder
	vertices.clear();

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		float xLoc = cos(rads[i]) * a_fRadius;
		float yLoc = sin(rads[i]) * a_fRadius;

		float xLocNext = cos(rads[(i + 1) % a_nSubdivisions]) * a_fRadius;
		float yLocNext = sin(rads[(i + 1) % a_nSubdivisions]) * a_fRadius;

		vertices.push_back(vector3(xLoc, yLoc, baseTop.z));
		vertices.push_back(vector3(xLoc, yLoc, baseBottom.z));
		vertices.push_back(vector3(xLocNext, yLocNext, baseTop.z));
		vertices.push_back(vector3(xLocNext, yLocNext, baseBottom.z));
	}

	// draw quads from vertices
	for (int i = 0; i < a_nSubdivisions * 4; i += 4)
	{
		std::vector<vector3> quadVerts;
		for (int v = 0; v < 4; v++)
		{
			quadVerts.push_back(vertices[i + v]);
		}
		AddQuad(quadVerts[0], quadVerts[1], quadVerts[2], quadVerts[3]);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	float halfHeight = a_fHeight / 2;

	vector3 baseTop = vector3(0, 0, halfHeight);
	vector3 baseBottom = vector3(0, 0, -halfHeight);

	std::vector<float> rads;
	std::vector<vector3> vertices;

	// get the rads of the divisions
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		rads.push_back((PI * 2) * ((float)i / (float)a_nSubdivisions));
	}

	
	for (int p = 0; p < a_nSubdivisions; p++)
	{
		// calculate the vertices
		float outX = cos(rads[p]) * a_fOuterRadius;
		float outY = sin(rads[p]) * a_fOuterRadius;
		float innerX = cos(rads[p]) * a_fInnerRadius;
		float innerY = sin(rads[p]) * a_fInnerRadius;

		// calculate the vertices for the next
		float nextRad = rads[(p + 1) % a_nSubdivisions];
		float outXNext = cos(nextRad) * a_fOuterRadius;
		float outYNext = sin(nextRad) * a_fOuterRadius;
		float innerXNext = cos(nextRad) * a_fInnerRadius;
		float innerYNext = sin(nextRad) * a_fInnerRadius;

		// add all vertices to the vector
		vertices.push_back(vector3(outX, outY, baseTop.z));
		vertices.push_back(vector3(outXNext, outYNext, baseTop.z));

		vertices.push_back(vector3(innerX, innerY, baseTop.z));
		vertices.push_back(vector3(innerXNext, innerYNext, baseTop.z));

		vertices.push_back(vector3(innerX, innerY, baseBottom.z));
		vertices.push_back(vector3(innerXNext, innerYNext, baseBottom.z));

		vertices.push_back(vector3(outX, outY, baseBottom.z));
		vertices.push_back(vector3(outXNext, outYNext, baseBottom.z));

		// construct the quads
		int vertSize = vertices.size();
		for (int i = 0; i < vertSize; i += 2)
		{
			AddQuad(vertices[i % vertSize], vertices[(i + 1) % vertSize], vertices[(i + 2) % vertSize], vertices[(i + 3) % vertSize]);
		}

		vertices.clear();
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// This code doesn't work, I've been having some difficulties with this shape

	float torusRadius = a_fOuterRadius - a_fInnerRadius;

	std::vector<float> rads;
	std::vector<vector3> vertices;

	float pieceRadSize = (PI * 2) * ((float)1 / (float)a_nSubdivisionsA);

	for (int i = 0; i < a_nSubdivisionsA; i++)
	{
		rads.push_back(i * pieceRadSize);
	}

	// for each piece (slice)
	for (int p = 0; p < a_nSubdivisionsA; p++)
	{
		float radiusFromCenterOfPieceToCenterOfObject = torusRadius + a_fInnerRadius;
		float centPX = (cos(rads[p]) * radiusFromCenterOfPieceToCenterOfObject);
		float centPY = (sin(rads[p]) * radiusFromCenterOfPieceToCenterOfObject);
		vector3 centerOfPiece = vector3(centPX, centPY, 0);

		// for each face in a slice
		for (int f = 0; f < a_nSubdivisionsB; f++)
		{
			float radOfSlice = (PI * 2) * ((float)f / (float)a_nSubdivisionsB);
			//float radiusFromSliceToCenterOfObject = radiusFromCenterOfPieceToCenterOfObject + cos(radOfSlice);

			float x = centerOfPiece.x + cos(radOfSlice);
			float y = centerOfPiece.y + sin(radOfSlice);
			float z = sin(radOfSlice);

			vertices.push_back(vector3(x, y, z));
		}
	}

	// build the quads for the torus
	int ab = (vertices.size() / 2);
	for (int i = 0; i < ab; i++)
	{
		AddQuad(vertices[i], vertices[(i + a_nSubdivisionsB) % vertices.size()], vertices[i + 1], vertices[(i + a_nSubdivisionsB + 1) % vertices.size()]);
	}
	

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 11)
		a_nSubdivisions = 11;

	Release();
	Init();

	std::vector<float> rads;
	std::vector<vector3> vertices;

	// calculate all the rads for each division
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		rads.push_back((PI * 2) * ((float)i / (float)a_nSubdivisions));
	}

	vector3 sphereTop = vector3(0, 0, a_fRadius);
	vector3 sphereBottom = vector3(0, 0, -a_fRadius);

	float heightOfEachRing = (a_fRadius * 2) / a_nSubdivisions;

	// for each piece (slices)
	for (int p = 0; p < a_nSubdivisions; p++)
	{
		// for each face in a slice
		for (int f = 0; f <= a_nSubdivisions; f++)
		{
			float currentHeight = sphereBottom.z + (heightOfEachRing * f);
			//float nextHeight = sphereBottom.z + (heightOfEachRing * (f + 1));

			// get the radius for the currect height of the face
			float radiusForCurrentHeight = cos(asin(currentHeight / a_fRadius)) * a_fRadius;
			//float radiusForNextHeight = cos(asin(nextHeight / a_fRadius)) * a_fRadius;

			// calculate the vertex position
			float x = cos(rads[p]) * radiusForCurrentHeight;
			float y = sin(rads[p]) * radiusForCurrentHeight;
			float z = currentHeight;

			// calculate the vertex position of the adjecent face
			float radsOfNext = rads[(p + 1) % a_nSubdivisions];
			float xNext = cos(radsOfNext) * radiusForCurrentHeight;
			float yNext = sin(radsOfNext) * radiusForCurrentHeight;

			// add the vertices to the list
			vertices.push_back(vector3(x, y, z));
			vertices.push_back(vector3(xNext, yNext, z));
		}

		// build the quads
		int vertSize = vertices.size();
		for (int i = 0; i < vertSize - 2; i += 2)
		{
			AddQuad(vertices[i % vertSize], vertices[(i + 1) % vertSize], vertices[(i + 2) % vertSize], vertices[(i + 3) % vertSize]);
		}

		vertices.clear();
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}