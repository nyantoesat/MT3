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

struct Segment {
	Vector3 origin;
	Vector3 diff;
};

struct OBB {
	Vector3 center;          //!< 中心点
	Vector3 orientations[3]; //!< 座標軸 (正規化済み)
	Vector3 size;            //!< 各軸の半サイズ
};

Vector3 Add(const Vector3& a, const Vector3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vector3 Subtract(const Vector3& a, const Vector3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vector3 Multiply(float s, const Vector3& v) { return {s * v.x, s * v.y, s * v.z}; }
float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vector3 Cross(const Vector3& a, const Vector3& b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
Vector3 Normalize(const Vector3& v) {
	float len = std::sqrt(Dot(v, v));
	assert(len != 0.0f);
	return {v.x / len, v.y / len, v.z / len};
}

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

// OBBのローカル空間への逆行列を作成する
Matrix4x4 MakeOBBInverseMatrix(const OBB& obb) {
	// OBBのワールド行列: 回転 + 平行移動 (スケールなし)
	Matrix4x4 obbWorld = {};
	obbWorld.m[0][0] = obb.orientations[0].x;
	obbWorld.m[0][1] = obb.orientations[0].y;
	obbWorld.m[0][2] = obb.orientations[0].z;
	obbWorld.m[0][3] = 0.0f;
	obbWorld.m[1][0] = obb.orientations[1].x;
	obbWorld.m[1][1] = obb.orientations[1].y;
	obbWorld.m[1][2] = obb.orientations[1].z;
	obbWorld.m[1][3] = 0.0f;
	obbWorld.m[2][0] = obb.orientations[2].x;
	obbWorld.m[2][1] = obb.orientations[2].y;
	obbWorld.m[2][2] = obb.orientations[2].z;
	obbWorld.m[2][3] = 0.0f;
	obbWorld.m[3][0] = obb.center.x;
	obbWorld.m[3][1] = obb.center.y;
	obbWorld.m[3][2] = obb.center.z;
	obbWorld.m[3][3] = 1.0f;
	return Inverse(obbWorld);
}

// AABBと線分の衝突判定 (OBBローカル空間で使用)
bool IsCollision(const AABB& aabb, const Segment& segment) {
	// 各軸でのスラブ判定
	float tmin = 0.0f, tmax = 1.0f;
	float dirs[3] = {segment.diff.x, segment.diff.y, segment.diff.z};
	float origs[3] = {segment.origin.x, segment.origin.y, segment.origin.z};
	float mins[3] = {aabb.min.x, aabb.min.y, aabb.min.z};
	float maxs[3] = {aabb.max.x, aabb.max.y, aabb.max.z};

	for (int i = 0; i < 3; ++i) {
		if (std::fabs(dirs[i]) < 1e-6f) {
			// 方向がゼロ: 原点がスラブ外なら非衝突
			if (origs[i] < mins[i] || origs[i] > maxs[i])
				return false;
		} else {
			float t1 = (mins[i] - origs[i]) / dirs[i];
			float t2 = (maxs[i] - origs[i]) / dirs[i];
			if (t1 > t2)
				std::swap(t1, t2);
			tmin = (std::max)(tmin, t1);
			tmax = (std::min)(tmax, t2);
			if (tmin > tmax)
				return false;
		}
	}
	return true;
}

// OBBと線分の衝突判定
// 線分をOBBのローカル空間に変換してAABBとして判定
bool IsCollision(const Segment& segment, const OBB& obb) {
	Matrix4x4 obbInverse = MakeOBBInverseMatrix(obb);

	// 線分の始点と終点をローカル空間に変換
	Vector3 localOrigin = Transform(segment.origin, obbInverse);
	Vector3 localEnd = Transform(Add(segment.origin, segment.diff), obbInverse);

	// ローカル空間でのAABB (半サイズ)
	AABB localAABB{
	    {-obb.size.x, -obb.size.y, -obb.size.z},
	    {+obb.size.x, +obb.size.y, +obb.size.z},
	};

	// ローカル空間での線分 (差分を終点-始点で再計算)
	Segment localSegment;
	localSegment.origin = localOrigin;
	localSegment.diff = Subtract(localEnd, localOrigin);

	return IsCollision(localAABB, localSegment);
}

// OBBをワールド空間に描画
void DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// OBBの8頂点をワールド空間で構築
	Vector3 axes[3] = {
	    Multiply(obb.size.x, obb.orientations[0]),
	    Multiply(obb.size.y, obb.orientations[1]),
	    Multiply(obb.size.z, obb.orientations[2]),
	};

	Vector3 vertices[8];
	for (int i = 0; i < 8; ++i) {
		vertices[i] = obb.center;
		vertices[i] = Add(vertices[i], Multiply((i & 1) ? 1.0f : -1.0f, axes[0]));
		vertices[i] = Add(vertices[i], Multiply((i & 2) ? 1.0f : -1.0f, axes[1]));
		vertices[i] = Add(vertices[i], Multiply((i & 4) ? 1.0f : -1.0f, axes[2]));
	}

	Vector3 s[8];
	for (int i = 0; i < 8; ++i) {
		s[i] = Transform(Transform(vertices[i], viewProjectionMatrix), viewportMatrix);
	}

	// 12辺
	int edges[12][2] = {
	    {0, 1},
        {2, 3},
        {4, 5},
        {6, 7}, // x方向
	    {0, 2},
        {1, 3},
        {4, 6},
        {5, 7}, // y方向
	    {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}  // z方向
	};
	for (auto& e : edges) {
		Novice::DrawLine(int(s[e[0]].x), int(s[e[0]].y), int(s[e[1]].x), int(s[e[1]].y), color);
	}
}

void DrawSegment(const Segment& segment, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 worldEnd = Add(segment.origin, segment.diff);
	Vector3 screenStart = Transform(Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
	Vector3 screenEnd = Transform(Transform(worldEnd, viewProjectionMatrix), viewportMatrix);
	Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + kGridEvery * float(xIndex);
		Vector3 s = Transform(Transform({x, 0.0f, -kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		Vector3 e = Transform(Transform({x, 0.0f, kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(s.x), int(s.y), int(e.x), int(e.y), (xIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF);
	}
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + kGridEvery * float(zIndex);
		Vector3 s = Transform(Transform({-kGridHalfWidth, 0.0f, z}, viewProjectionMatrix), viewportMatrix);
		Vector3 e = Transform(Transform({kGridHalfWidth, 0.0f, z}, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(s.x), int(s.y), int(e.x), int(e.y), (zIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF);
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

	// OBBの回転用 (ImGuiで変更)
	Vector3 rotate = {0.0f, 0.0f, 0.0f};

	// OBB (スライドの初期値)
	OBB obb{
	    {-1.0f,              0.0f,	                      0.0f                         }, // center
	    {
         {1.0f, 0.0f, 0.0f}, // orientations[0]
	        {0.0f, 1.0f, 0.0f}, // orientations[1]
	        {0.0f, 0.0f, 1.0f},	                                         // orientations[2]
	    },
	    {0.5f,	               0.5f, 0.5f	                                               }  // size
	};

	// 線分 (スライドの初期値)
	Segment segment{
	    {-0.8f, -0.3f, 0.0f}, // origin
	    {0.5f,  0.5f,  0.5f}  // diff
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
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("OBB.Center", &obb.center.x, 0.01f);
		ImGui::DragFloat3("OBB.Size", &obb.size.x, 0.01f);
		ImGui::DragFloat3("Rotate", &rotate.x, 0.01f);
		ImGui::DragFloat3("Segment.Origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment.Diff", &segment.diff.x, 0.01f);
		ImGui::End();

		// OBBのorientationsをrotateから更新
		Matrix4x4 rotMat = Multiply(MakeRotateXMatrix(rotate.x), Multiply(MakeRotateYMatrix(rotate.y), MakeRotateZMatrix(rotate.z)));
		obb.orientations[0] = {rotMat.m[0][0], rotMat.m[0][1], rotMat.m[0][2]};
		obb.orientations[1] = {rotMat.m[1][0], rotMat.m[1][1], rotMat.m[1][2]};
		obb.orientations[2] = {rotMat.m[2][0], rotMat.m[2][1], rotMat.m[2][2]};

		// OBBと線分の衝突判定
		bool hit = IsCollision(segment, obb);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		uint32_t color = hit ? 0xFF0000FF : 0xFFFFFFFF;
		DrawOBB(obb, viewProjectionMatrix, viewportMatrix, color);
		DrawSegment(segment, viewProjectionMatrix, viewportMatrix, color);

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