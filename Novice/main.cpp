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
	Vector3 min;
	Vector3 max;
};

struct Sphere {
	Vector3 center;
	float radius;
};

struct OBB {
	Vector3 center;
	Vector3 orientations[3]; // 各軸の方向ベクトル (正規化済み)
	Vector3 size;            // 各軸方向の半径
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

// OBBと球の衝突判定
bool IsCollision(const OBB& obb, const Sphere& sphere) {
	// OBBのローカル空間に変換するための回転行列 (転置 = 逆行列)
	Matrix4x4 obbLocalMatrix = {};
	obbLocalMatrix.m[0][0] = obb.orientations[0].x;
	obbLocalMatrix.m[0][1] = obb.orientations[1].x;
	obbLocalMatrix.m[0][2] = obb.orientations[2].x;
	obbLocalMatrix.m[0][3] = 0;
	obbLocalMatrix.m[1][0] = obb.orientations[0].y;
	obbLocalMatrix.m[1][1] = obb.orientations[1].y;
	obbLocalMatrix.m[1][2] = obb.orientations[2].y;
	obbLocalMatrix.m[1][3] = 0;
	obbLocalMatrix.m[2][0] = obb.orientations[0].z;
	obbLocalMatrix.m[2][1] = obb.orientations[1].z;
	obbLocalMatrix.m[2][2] = obb.orientations[2].z;
	obbLocalMatrix.m[2][3] = 0;
	obbLocalMatrix.m[3][0] = 0;
	obbLocalMatrix.m[3][1] = 0;
	obbLocalMatrix.m[3][2] = 0;
	obbLocalMatrix.m[3][3] = 1;

	// 球の中心とOBBの中心をローカル空間へ変換
	Vector3 localSphereCenter = Transform(sphere.center, obbLocalMatrix);
	Vector3 localOBBCenter = Transform(obb.center, obbLocalMatrix);

	// ローカル空間での差ベクトル → clampで最近傍点を求める
	Vector3 localDiff = Subtract(localSphereCenter, localOBBCenter);
	Vector3 closestPoint = {
	    std::clamp(localDiff.x, -obb.size.x, obb.size.x),
	    std::clamp(localDiff.y, -obb.size.y, obb.size.y),
	    std::clamp(localDiff.z, -obb.size.z, obb.size.z),
	};

	// 最近傍点と球の中心の距離を比較
	Vector3 d = Subtract(localDiff, closestPoint);
	float distSq = Dot(d, d);
	return distSq <= sphere.radius * sphere.radius;
}

// OBBの描画: 8頂点を求めて各辺を結ぶ
void DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 ax = Multiply(obb.size.x, obb.orientations[0]);
	Vector3 ay = Multiply(obb.size.y, obb.orientations[1]);
	Vector3 az = Multiply(obb.size.z, obb.orientations[2]);

	// 8頂点
	Vector3 vertices[8] = {
	    Add(obb.center, Add(Multiply(-1, ax), Add(Multiply(-1, ay), Multiply(-1, az)))),
	    Add(obb.center, Add(ax, Add(Multiply(-1, ay), Multiply(-1, az)))),
	    Add(obb.center, Add(Multiply(-1, ax), Add(ay, Multiply(-1, az)))),
	    Add(obb.center, Add(ax, Add(ay, Multiply(-1, az)))),
	    Add(obb.center, Add(Multiply(-1, ax), Add(Multiply(-1, ay), az))),
	    Add(obb.center, Add(ax, Add(Multiply(-1, ay), az))),
	    Add(obb.center, Add(Multiply(-1, ax), Add(ay, az))),
	    Add(obb.center, Add(ax, Add(ay, az))),
	};

	// スクリーン座標に変換
	Vector3 s[8];
	for (int i = 0; i < 8; ++i)
		s[i] = Transform(Transform(vertices[i], viewProjectionMatrix), viewportMatrix);

	// 12辺を描画
	auto line = [&](int a, int b) { Novice::DrawLine(int(s[a].x), int(s[a].y), int(s[b].x), int(s[b].y), color); };
	// 底面 (z=min側)
	line(0, 1);
	line(1, 3);
	line(3, 2);
	line(2, 0);
	// 上面 (z=max側)
	line(4, 5);
	line(5, 7);
	line(7, 6);
	line(6, 4);
	// 縦辺
	line(0, 4);
	line(1, 5);
	line(2, 6);
	line(3, 7);
}

// 球の描画
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const int kLatDiv = 16, kLonDiv = 16;
	for (int lat = 0; lat < kLatDiv; ++lat) {
		float latA = float(lat) / kLatDiv * 3.14159f - 3.14159f / 2.0f;
		float latB = float(lat + 1) / kLatDiv * 3.14159f - 3.14159f / 2.0f;
		for (int lon = 0; lon < kLonDiv; ++lon) {
			float lonA = float(lon) / kLonDiv * 2.0f * 3.14159f;
			float lonB = float(lon + 1) / kLonDiv * 2.0f * 3.14159f;
			Vector3 a = {
			    sphere.center.x + sphere.radius * std::cos(latA) * std::cos(lonA), sphere.center.y + sphere.radius * std::sin(latA), sphere.center.z + sphere.radius * std::cos(latA) * std::sin(lonA)};
			Vector3 b = {
			    sphere.center.x + sphere.radius * std::cos(latB) * std::cos(lonA), sphere.center.y + sphere.radius * std::sin(latB), sphere.center.z + sphere.radius * std::cos(latB) * std::sin(lonA)};
			Vector3 c = {
			    sphere.center.x + sphere.radius * std::cos(latA) * std::cos(lonB), sphere.center.y + sphere.radius * std::sin(latA), sphere.center.z + sphere.radius * std::cos(latA) * std::sin(lonB)};
			Vector3 sa = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
			Vector3 sb = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
			Vector3 sc = Transform(Transform(c, viewProjectionMatrix), viewportMatrix);
			Novice::DrawLine(int(sa.x), int(sa.y), int(sb.x), int(sb.y), color);
			Novice::DrawLine(int(sa.x), int(sa.y), int(sc.x), int(sc.y), color);
		}
	}
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

	// OBBの回転用変数
	Vector3 rotate{0.0f, 0.0f, 0.0f};

	OBB obb;
	obb.center = {-1.0f, 0.0f, 0.0f};
	obb.orientations[0] = {1.0f, 0.0f, 0.0f};
	obb.orientations[1] = {0.0f, 1.0f, 0.0f};
	obb.orientations[2] = {0.0f, 0.0f, 1.0f};
	obb.size = {0.5f, 0.5f, 0.5f};

	// 球 (スライドの初期値通り)
	Sphere sphere;
	sphere.center = {0.0f, 0.0f, 0.0f};
	sphere.radius = 0.5f;

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

		// ImGui
		ImGui::Begin("Window");
		ImGui::DragFloat3("rotate", &rotate.x, 0.01f);
		ImGui::DragFloat3("obb.center", &obb.center.x, 0.01f);
		ImGui::DragFloat3("obb.size", &obb.size.x, 0.01f);
		ImGui::DragFloat3("sphere.center", &sphere.center.x, 0.01f);
		ImGui::DragFloat("sphere.radius", &sphere.radius, 0.01f);
		ImGui::End();

		// 回転行列を生成
		Matrix4x4 rotateMatrix = Multiply(MakeRotateXMatrix(rotate.x), Multiply(MakeRotateYMatrix(rotate.y), MakeRotateZMatrix(rotate.z)));

		// 回転行列から軸を抽出してOBBのorientationsに設定
		obb.orientations[0] = {rotateMatrix.m[0][0], rotateMatrix.m[0][1], rotateMatrix.m[0][2]};
		obb.orientations[1] = {rotateMatrix.m[1][0], rotateMatrix.m[1][1], rotateMatrix.m[1][2]};
		obb.orientations[2] = {rotateMatrix.m[2][0], rotateMatrix.m[2][1], rotateMatrix.m[2][2]};

		// OBBと球の衝突判定
		bool hit = IsCollision(obb, sphere);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 衝突時は赤、非衝突時は白
		uint32_t color = hit ? 0xFF0000FF : 0xFFFFFFFF;
		DrawOBB(obb, viewProjectionMatrix, viewportMatrix, color);
		DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, color);

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