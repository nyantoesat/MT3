#include <Novice.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <imgui.h>

const char kWindowTitle[] = "GC2B_03_ニャン_トー_セッ";

static const int kWindowWidth = 1280;
static const int kWindowHeight = 720;

struct Vector3 {
	float x, y, z;
};

struct Matrix4x4 {
	float m[4][4];
};

struct AABB {
	Vector3 min; //!< 最小点
	Vector3 max; //!< 最大点
};

Vector3 Add(const Vector3& a, const Vector3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vector3 Subtract(const Vector3& a, const Vector3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vector3 Multiply(float s, const Vector3& v) { return {s * v.x, s * v.y, s * v.z}; }
float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 r = {};
	for (int i = 0; i < 4; ++i)
		r.m[i][i] = 1.0f;
	return r;
}

Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b) {
	Matrix4x4 r = {};
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 4; ++k)
				r.m[i][j] += a.m[i][k] * b.m[k][j];
	return r;
}

Vector3 Transform(const Vector3& v, const Matrix4x4& m) {
	Vector3 r;
	r.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
	r.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
	r.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
	float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];
	assert(w != 0.0f);
	r.x /= w;
	r.y /= w;
	r.z /= w;
	return r;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& t) {
	Matrix4x4 r = MakeIdentity4x4();
	r.m[3][0] = t.x;
	r.m[3][1] = t.y;
	r.m[3][2] = t.z;
	return r;
}

Matrix4x4 MakeScaleMatrix(const Vector3& s) {
	Matrix4x4 r = {};
	r.m[0][0] = s.x;
	r.m[1][1] = s.y;
	r.m[2][2] = s.z;
	r.m[3][3] = 1.0f;
	return r;
}

Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 r = MakeIdentity4x4();
	r.m[1][1] = std::cos(radian);
	r.m[1][2] = std::sin(radian);
	r.m[2][1] = -std::sin(radian);
	r.m[2][2] = std::cos(radian);
	return r;
}

Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 r = MakeIdentity4x4();
	r.m[0][0] = std::cos(radian);
	r.m[0][2] = -std::sin(radian);
	r.m[2][0] = std::sin(radian);
	r.m[2][2] = std::cos(radian);
	return r;
}

Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 r = MakeIdentity4x4();
	r.m[0][0] = std::cos(radian);
	r.m[0][1] = std::sin(radian);
	r.m[1][0] = -std::sin(radian);
	r.m[1][1] = std::cos(radian);
	return r;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 s = MakeScaleMatrix(scale);
	Matrix4x4 rot = Multiply(MakeRotateXMatrix(rotate.x), Multiply(MakeRotateYMatrix(rotate.y), MakeRotateZMatrix(rotate.z)));
	Matrix4x4 t = MakeTranslateMatrix(translate);
	return Multiply(Multiply(s, rot), t);
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 r = {};
	float f = 1.0f / std::tan(fovY / 2.0f);
	r.m[0][0] = f / aspectRatio;
	r.m[1][1] = f;
	r.m[2][2] = farClip / (farClip - nearClip);
	r.m[2][3] = 1.0f;
	r.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	return r;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 r = MakeIdentity4x4();
	r.m[0][0] = width / 2.0f;
	r.m[1][1] = -height / 2.0f;
	r.m[2][2] = maxDepth - minDepth;
	r.m[3][0] = left + width / 2.0f;
	r.m[3][1] = top + height / 2.0f;
	r.m[3][2] = minDepth;
	return r;
}

Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 r;
	float A2323 = m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2];
	float A1323 = m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1];
	float A1223 = m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1];
	float A0323 = m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0];
	float A0223 = m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0];
	float A0123 = m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0];
	float A2313 = m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2];
	float A1313 = m.m[1][1] * m.m[3][3] - m.m[1][3] * m.m[3][1];
	float A1213 = m.m[1][1] * m.m[3][2] - m.m[1][2] * m.m[3][1];
	float A2312 = m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2];
	float A1312 = m.m[1][1] * m.m[2][3] - m.m[1][3] * m.m[2][1];
	float A1212 = m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1];
	float A0313 = m.m[1][0] * m.m[3][3] - m.m[1][3] * m.m[3][0];
	float A0213 = m.m[1][0] * m.m[3][2] - m.m[1][2] * m.m[3][0];
	float A0312 = m.m[1][0] * m.m[2][3] - m.m[1][3] * m.m[2][0];
	float A0212 = m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0];
	float A0113 = m.m[1][0] * m.m[3][1] - m.m[1][1] * m.m[3][0];
	float A0112 = m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0];
	float det = m.m[0][0] * (m.m[1][1] * A2323 - m.m[1][2] * A1323 + m.m[1][3] * A1223) - m.m[0][1] * (m.m[1][0] * A2323 - m.m[1][2] * A0323 + m.m[1][3] * A0223) +
	            m.m[0][2] * (m.m[1][0] * A1323 - m.m[1][1] * A0323 + m.m[1][3] * A0123) - m.m[0][3] * (m.m[1][0] * A1223 - m.m[1][1] * A0223 + m.m[1][2] * A0123);
	assert(det != 0.0f);
	float invDet = 1.0f / det;
	r.m[0][0] = invDet * (m.m[1][1] * A2323 - m.m[1][2] * A1323 + m.m[1][3] * A1223);
	r.m[0][1] = -invDet * (m.m[0][1] * A2323 - m.m[0][2] * A1323 + m.m[0][3] * A1223);
	r.m[0][2] = invDet * (m.m[0][1] * A2313 - m.m[0][2] * A1313 + m.m[0][3] * A1213);
	r.m[0][3] = -invDet * (m.m[0][1] * A2312 - m.m[0][2] * A1312 + m.m[0][3] * A1212);
	r.m[1][0] = -invDet * (m.m[1][0] * A2323 - m.m[1][2] * A0323 + m.m[1][3] * A0223);
	r.m[1][1] = invDet * (m.m[0][0] * A2323 - m.m[0][2] * A0323 + m.m[0][3] * A0223);
	r.m[1][2] = -invDet * (m.m[0][0] * A2313 - m.m[0][2] * A0313 + m.m[0][3] * A0213);
	r.m[1][3] = invDet * (m.m[0][0] * A2312 - m.m[0][2] * A0312 + m.m[0][3] * A0212);
	r.m[2][0] = invDet * (m.m[1][0] * A1323 - m.m[1][1] * A0323 + m.m[1][3] * A0123);
	r.m[2][1] = -invDet * (m.m[0][0] * A1323 - m.m[0][1] * A0323 + m.m[0][3] * A0123);
	r.m[2][2] = invDet * (m.m[0][0] * A1313 - m.m[0][1] * A0313 + m.m[0][3] * A0113);
	r.m[2][3] = -invDet * (m.m[0][0] * A1312 - m.m[0][1] * A0312 + m.m[0][3] * A0112);
	r.m[3][0] = -invDet * (m.m[1][0] * A1223 - m.m[1][1] * A0223 + m.m[1][2] * A0123);
	r.m[3][1] = invDet * (m.m[0][0] * A1223 - m.m[0][1] * A0223 + m.m[0][2] * A0123);
	r.m[3][2] = -invDet * (m.m[0][0] * A1213 - m.m[0][1] * A0213 + m.m[0][2] * A0113);
	r.m[3][3] = invDet * (m.m[0][0] * A1212 - m.m[0][1] * A0212 + m.m[0][2] * A0112);
	return r;
}

// AABBとAABBの衝突判定
bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	return (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && (aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && (aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);
}

// AABBの描画: 8頂点を求めて各辺を結ぶ
void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// 8頂点: min/maxの全組み合わせ
	Vector3 vertices[8] = {
	    {aabb.min.x, aabb.min.y, aabb.min.z}, // 0: ---
	    {aabb.max.x, aabb.min.y, aabb.min.z}, // 1: +--
	    {aabb.min.x, aabb.max.y, aabb.min.z}, // 2: -+-
	    {aabb.max.x, aabb.max.y, aabb.min.z}, // 3: ++-
	    {aabb.min.x, aabb.min.y, aabb.max.z}, // 4: --+
	    {aabb.max.x, aabb.min.y, aabb.max.z}, // 5: +-+
	    {aabb.min.x, aabb.max.y, aabb.max.z}, // 6: -++
	    {aabb.max.x, aabb.max.y, aabb.max.z}, // 7: +++
	};

	// スクリーン座標に変換
	Vector3 s[8];
	for (int i = 0; i < 8; ++i) {
		s[i] = Transform(Transform(vertices[i], viewProjectionMatrix), viewportMatrix);
	}

	// 12辺を描画
	// 底面 (z=min)
	Novice::DrawLine(int(s[0].x), int(s[0].y), int(s[1].x), int(s[1].y), color);
	Novice::DrawLine(int(s[1].x), int(s[1].y), int(s[3].x), int(s[3].y), color);
	Novice::DrawLine(int(s[3].x), int(s[3].y), int(s[2].x), int(s[2].y), color);
	Novice::DrawLine(int(s[2].x), int(s[2].y), int(s[0].x), int(s[0].y), color);
	// 上面 (z=max)
	Novice::DrawLine(int(s[4].x), int(s[4].y), int(s[5].x), int(s[5].y), color);
	Novice::DrawLine(int(s[5].x), int(s[5].y), int(s[7].x), int(s[7].y), color);
	Novice::DrawLine(int(s[7].x), int(s[7].y), int(s[6].x), int(s[6].y), color);
	Novice::DrawLine(int(s[6].x), int(s[6].y), int(s[4].x), int(s[4].y), color);
	// 縦辺
	Novice::DrawLine(int(s[0].x), int(s[0].y), int(s[4].x), int(s[4].y), color);
	Novice::DrawLine(int(s[1].x), int(s[1].y), int(s[5].x), int(s[5].y), color);
	Novice::DrawLine(int(s[2].x), int(s[2].y), int(s[6].x), int(s[6].y), color);
	Novice::DrawLine(int(s[3].x), int(s[3].y), int(s[7].x), int(s[7].y), color);
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);

	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + kGridEvery * float(xIndex);
		Vector3 screenStart = Transform(Transform({x, 0.0f, -kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform({x, 0.0f, kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		unsigned int color = (xIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + kGridEvery * float(zIndex);
		Vector3 screenStart = Transform(Transform({-kGridHalfWidth, 0.0f, z}, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform({kGridHalfWidth, 0.0f, z}, viewProjectionMatrix), viewportMatrix);
		unsigned int color = (zIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	char keys[256] = {0};
	char preKeys[256] = {0};

	// カメラ
	Vector3 cameraScale = {1.0f, 1.0f, 1.0f};
	Vector3 cameraRotate = {0.26f, 0.0f, 0.0f};
	Vector3 cameraTranslate = {0.0f, 1.9f, -6.49f};

	// 2つのAABB (スライドの初期値通り)
	AABB aabb1{
	    {-0.5f, -0.5f, -0.5f}, // min
	    {0.0f,  0.0f,  0.0f }, // max
	};
	AABB aabb2{
	    {0.2f, 0.2f, 0.2f}, // min
	    {1.0f, 1.0f, 1.0f}, // max
	};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraScale, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		ImGui::Begin("Window");
		ImGui::DragFloat3("aabb1.min", &aabb1.min.x, 0.01f);
		ImGui::DragFloat3("aabb1.max", &aabb1.max.x, 0.01f);
		ImGui::DragFloat3("aabb2.min", &aabb2.min.x, 0.01f);
		ImGui::DragFloat3("aabb2.max", &aabb2.max.x, 0.01f);
		ImGui::End();

		// minとmaxが入れ替わらないように補正
		aabb1.min.x = (std::min)(aabb1.min.x, aabb1.max.x);
		aabb1.max.x = (std::max)(aabb1.min.x, aabb1.max.x);
		aabb1.min.y = (std::min)(aabb1.min.y, aabb1.max.y);
		aabb1.max.y = (std::max)(aabb1.min.y, aabb1.max.y);
		aabb1.min.z = (std::min)(aabb1.min.z, aabb1.max.z);
		aabb1.max.z = (std::max)(aabb1.min.z, aabb1.max.z);

		aabb2.min.x = (std::min)(aabb2.min.x, aabb2.max.x);
		aabb2.max.x = (std::max)(aabb2.min.x, aabb2.max.x);
		aabb2.min.y = (std::min)(aabb2.min.y, aabb2.max.y);
		aabb2.max.y = (std::max)(aabb2.min.y, aabb2.max.y);
		aabb2.min.z = (std::min)(aabb2.min.z, aabb2.max.z);
		aabb2.max.z = (std::max)(aabb2.min.z, aabb2.max.z);

		// AABBとAABBの衝突判定
		bool hit = IsCollision(aabb1, aabb2);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 衝突時は赤、非衝突時は白
		uint32_t color = hit ? 0xFF0000FF : 0xFFFFFFFF;
		DrawAABB(aabb1, viewProjectionMatrix, viewportMatrix, color);
		DrawAABB(aabb2, viewProjectionMatrix, viewportMatrix, color);

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