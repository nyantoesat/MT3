#include <Novice.h>
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

struct Segment {
	Vector3 origin;
	Vector3 diff;
};

struct Plane {
	Vector3 normal;
	float distance;
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

Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return {-vector.y, vector.x, 0.0f};
	}
	return {0.0f, -vector.z, vector.y};
}

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 r = {};
	for (int i = 0; i < 4; ++i) {
		r.m[i][i] = 1.0f;
	}
	return r;
}

Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b) {
	Matrix4x4 r = {};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			for (int k = 0; k < 4; ++k) {
				r.m[i][j] += a.m[i][k] * b.m[k][j];
			}
		}
	}
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
	Matrix4x4 rx = MakeRotateXMatrix(rotate.x);
	Matrix4x4 ry = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rz = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rot = Multiply(rx, Multiply(ry, rz));
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

// 線分と平面の衝突判定
bool IsCollision(const Segment& segment, const Plane& plane) {
	float denom = Dot(plane.normal, segment.diff);
	if (std::fabs(denom) < 1e-6f) {
		return false;
	}
	float t = (plane.distance - Dot(plane.normal, segment.origin)) / denom;
	return t >= 0.0f && t <= 1.0f;
}

void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 center = Multiply(plane.distance, plane.normal);
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal));
	perpendiculars[1] = {-perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z};
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);
	perpendiculars[3] = {-perpendiculars[2].x, -perpendiculars[2].y, -perpendiculars[2].z};

	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = Multiply(2.0f, perpendiculars[index]);
		Vector3 point = Add(center, extend);
		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
	}
	Novice::DrawLine(int(points[0].x), int(points[0].y), int(points[2].x), int(points[2].y), color);
	Novice::DrawLine(int(points[2].x), int(points[2].y), int(points[1].x), int(points[1].y), color);
	Novice::DrawLine(int(points[1].x), int(points[1].y), int(points[3].x), int(points[3].y), color);
	Novice::DrawLine(int(points[3].x), int(points[3].y), int(points[0].x), int(points[0].y), color);
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
		Vector3 worldStart = {x, 0.0f, -kGridHalfWidth};
		Vector3 worldEnd = {x, 0.0f, kGridHalfWidth};
		Vector3 screenStart = Transform(Transform(worldStart, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform(worldEnd, viewProjectionMatrix), viewportMatrix);
		unsigned int color = (xIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}

	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + kGridEvery * float(zIndex);
		Vector3 worldStart = {-kGridHalfWidth, 0.0f, z};
		Vector3 worldEnd = {kGridHalfWidth, 0.0f, z};
		Vector3 screenStart = Transform(Transform(worldStart, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform(worldEnd, viewProjectionMatrix), viewportMatrix);
		unsigned int color = (zIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	// カメラ
	Vector3 cameraScale = {1.0f, 1.0f, 1.0f};
	Vector3 cameraRotate = {0.26f, 0.0f, 0.0f};
	Vector3 cameraTranslate = {0.0f, 1.9f, -6.49f};

	// 平面
	Plane plane{
	    {0.0f, 1.0f, 0.0f},
        1.0f
    };

	// 線分
	Segment segment{
	    {-2.0f, 1.0f,  0.0f}, // origin
	    {3.0f,  -2.0f, 0.0f}  // diff
	};

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
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
		ImGui::DragFloat3("Plane.Normal", &plane.normal.x, 0.01f);
		ImGui::DragFloat("Plane.Distance", &plane.distance, 0.01f);
		ImGui::DragFloat3("Segment.Origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment.Diff", &segment.diff.x, 0.01f);
		ImGui::End();

		plane.normal = Normalize(plane.normal);

		// 線分と平面の衝突判定
		bool segHit = IsCollision(segment, plane);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);
		DrawPlane(plane, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

		// 衝突時は赤、非衝突時は白
		uint32_t segColor = segHit ? 0xFF0000FF : 0xFFFFFFFF;
		DrawSegment(segment, viewProjectionMatrix, viewportMatrix, segColor);

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}