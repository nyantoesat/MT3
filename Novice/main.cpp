#include <Novice.h>
#include <cmath>
#include <stdio.h>

const char kWindowTitle[] = "GC2B_03_ニャン_トー_セッ";

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Matrix4x4 {
	float m[4][4];
};

static const int kRowHeight = 20;
static const int kColumnWidth = 60;
static const int kWindowWidth = 1280;
static const int kWindowHeight = 720;

Vector3 Cross(const Vector3& v1, const Vector3& v2) { return {v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x}; }

void VectorScreenPrintf(int x, int y, const Vector3& v, const char* label) { Novice::ScreenPrintf(x, y, "%s: (%.2f, %.2f, %.2f)", label, v.x, v.y, v.z); }

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 result = {};
	float cosX = std::cos(rotate.x), sinX = std::sin(rotate.x);
	float cosY = std::cos(rotate.y), sinY = std::sin(rotate.y);
	float cosZ = std::cos(rotate.z), sinZ = std::sin(rotate.z);

	result.m[0][0] = scale.x * (cosY * cosZ + sinX * sinY * sinZ);
	result.m[0][1] = scale.x * (-cosX * sinZ);
	result.m[0][2] = scale.x * (sinY * cosZ + sinX * cosY * sinZ);
	result.m[0][3] = 0.0f;

	result.m[1][0] = scale.y * (cosY * sinZ + sinX * sinY * cosZ);
	result.m[1][1] = scale.y * (cosX * cosZ);
	result.m[1][2] = scale.y * (sinY * sinZ - sinX * cosY * cosZ);
	result.m[1][3] = 0.0f;

	result.m[2][0] = scale.z * (-cosX * sinY);
	result.m[2][1] = scale.z * (sinX);
	result.m[2][2] = scale.z * (cosX * cosY);
	result.m[2][3] = 0.0f;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				result.m[i][j] += a.m[i][k] * b.m[k][j];
	return result;
}

Matrix4x4 Inverse(const Matrix4x4& mat) {
	Matrix4x4 result = {};
	float m[4][8] = {};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			m[i][j] = mat.m[i][j];
		m[i][i + 4] = 1.0f;
	}
	for (int i = 0; i < 4; i++) {
		float pivot = m[i][i];
		for (int j = 0; j < 8; j++)
			m[i][j] /= pivot;
		for (int k = 0; k < 4; k++) {
			if (k == i)
				continue;
			float factor = m[k][i];
			for (int j = 0; j < 8; j++)
				m[k][j] -= factor * m[i][j];
		}
	}
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			result.m[i][j] = m[i][j + 4];
	return result;
}

Vector3 Transform(const Vector3& v, const Matrix4x4& mat) {
	float x = v.x * mat.m[0][0] + v.y * mat.m[1][0] + v.z * mat.m[2][0] + mat.m[3][0];
	float y = v.x * mat.m[0][1] + v.y * mat.m[1][1] + v.z * mat.m[2][1] + mat.m[3][1];
	float z = v.x * mat.m[0][2] + v.y * mat.m[1][2] + v.z * mat.m[2][2] + mat.m[3][2];
	float w = v.x * mat.m[0][3] + v.y * mat.m[1][3] + v.z * mat.m[2][3] + mat.m[3][3];
	if (w != 0.0f) {
		x /= w;
		y /= w;
		z /= w;
	}
	return {x, y, z};
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result = {};
	float cot = 1.0f / std::tan(fovY / 2.0f);
	result.m[0][0] = cot / aspectRatio;
	result.m[1][1] = cot;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	return result;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result = {};
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;
	return result;
}

static const Vector3 kLocalVertices[3] = {
    {0.0f,  1.0f,  0.0f},
    {1.0f,  -1.0f, 0.0f},
    {-1.0f, -1.0f, 0.0f},
};

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 rotate = {0.0f, 0.0f, 0.0f};
	Vector3 translate = {0.0f, 0.0f, 5.0f};
	Vector3 cameraPosition = {0.0f, 0.0f, -10.0f};

	Vector3 v1 = {1.2f, -3.9f, 2.5f};
	Vector3 v2 = {2.8f, 0.4f, -1.3f};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		const float kMoveSpeed = 0.05f;
		rotate.y += 0.02f;

		if (keys[DIK_W])
			translate.z += kMoveSpeed;
		if (keys[DIK_S])
			translate.z -= kMoveSpeed;
		if (keys[DIK_A])
			translate.x -= kMoveSpeed;
		if (keys[DIK_D])
			translate.x += kMoveSpeed;

		Matrix4x4 worldMatrix = MakeAffineMatrix({1.0f, 1.0f, 1.0f}, rotate, translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix({1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, cameraPosition);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		Vector3 screenVertices[3];
		for (uint32_t i = 0; i < 3; ++i) {
			Vector3 ndcVertex = Transform(kLocalVertices[i], worldViewProjectionMatrix);
			screenVertices[i] = Transform(ndcVertex, viewportMatrix);
		}

		Vector3 cross = Cross(v1, v2);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		Novice::ScreenPrintf(0, 0, "translate: %.2f %.2f %.2f  rotate.y: %.2f", translate.x, translate.y, translate.z, rotate.y);
		VectorScreenPrintf(0, kRowHeight, v1, "v1");
		VectorScreenPrintf(0, kRowHeight * 2, v2, "v2");
		VectorScreenPrintf(0, kRowHeight * 3, cross, "Cross");

		Novice::DrawTriangle(
		    int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[1].x), int(screenVertices[1].y), int(screenVertices[2].x), int(screenVertices[2].y), RED, kFillModeSolid);

		///
		/// ↑描画処理ここまで
		///

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}