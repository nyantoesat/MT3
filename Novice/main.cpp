#define _USE_MATH_DEFINES
#include <Novice.h>
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <stdio.h>

const char kWindowTitle[] = "GC2B_03_ニャン_トー_セッ";
const int kWindowHeight = 720;
const int kWindowWidth = 1280;

constexpr float kPi = 3.14159265358979323846f;

struct Vector3 {
	float x, y, z;
};

struct Matrix4x4 {
	float m[4][4];
};

struct OBB {
	Vector3 center;
	Vector3 orientations[3];
	Vector3 size;
};

Vector3 Subtract(const Vector3& a, const Vector3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vector3 Add(const Vector3& a, const Vector3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
float LengthSquared(const Vector3& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
Vector3 Scale(const Vector3& v, float s) { return {v.x * s, v.y * s, v.z * s}; }
float Length(const Vector3& v) { return sqrtf(LengthSquared(v)); }

Vector3 Normalize(const Vector3& v) {
	float len = Length(v);
	if (len < 1e-6f)
		return {0, 0, 0};
	return {v.x / len, v.y / len, v.z / len};
}

Vector3 Cross(const Vector3& a, const Vector3& b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }

Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				result.m[i][j] += a.m[i][k] * b.m[k][j];
	return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& t) {
	Matrix4x4 m = {
	    {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {t.x, t.y, t.z, 1}}
    };
	return m;
}

Matrix4x4 MakeRotateXMatrix(float angle) {
	float c = cosf(angle), s = sinf(angle);
	Matrix4x4 m = {
	    {{1, 0, 0, 0}, {0, c, s, 0}, {0, -s, c, 0}, {0, 0, 0, 1}}
    };
	return m;
}

Matrix4x4 MakeRotateYMatrix(float angle) {
	float c = cosf(angle), s = sinf(angle);
	Matrix4x4 m = {
	    {{c, 0, -s, 0}, {0, 1, 0, 0}, {s, 0, c, 0}, {0, 0, 0, 1}}
    };
	return m;
}

Matrix4x4 MakeRotateZMatrix(float angle) {
	float c = cosf(angle), s = sinf(angle);
	Matrix4x4 m = {
	    {{c, s, 0, 0}, {-s, c, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}
    };
	return m;
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspect, float nearZ, float farZ) {
	float f = 1.0f / tanf(fovY / 2.0f);
	Matrix4x4 m = {};
	m.m[0][0] = f / aspect;
	m.m[1][1] = f;
	m.m[2][2] = farZ / (farZ - nearZ);
	m.m[2][3] = 1.0f;
	m.m[3][2] = -nearZ * farZ / (farZ - nearZ);
	return m;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minD, float maxD) {
	Matrix4x4 m = {};
	m.m[0][0] = width / 2.0f;
	m.m[1][1] = -height / 2.0f;
	m.m[2][2] = maxD - minD;
	m.m[3][0] = left + width / 2.0f;
	m.m[3][1] = top + height / 2.0f;
	m.m[3][2] = minD;
	m.m[3][3] = 1.0f;
	return m;
}

Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 inv = {};
	float det = 0.0f;
	float mat[4][4];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			mat[i][j] = m.m[i][j];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			float sub[3][3];
			int si = 0;
			for (int row = 0; row < 4; row++) {
				if (row == i)
					continue;
				int sj = 0;
				for (int col = 0; col < 4; col++) {
					if (col == j)
						continue;
					sub[si][sj++] = mat[row][col];
				}
				si++;
			}
			float minor =
			    sub[0][0] * (sub[1][1] * sub[2][2] - sub[1][2] * sub[2][1]) - sub[0][1] * (sub[1][0] * sub[2][2] - sub[1][2] * sub[2][0]) + sub[0][2] * (sub[1][0] * sub[2][1] - sub[1][1] * sub[2][0]);
			float cofactor = ((i + j) % 2 == 0 ? 1.0f : -1.0f) * minor;
			inv.m[j][i] = cofactor;
			if (j == 0)
				det += mat[i][0] * cofactor;
		}
	}
	if (fabsf(det) < 1e-6f)
		return inv;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			inv.m[i][j] /= det;
	return inv;
}

Vector3 Transform(const Vector3& v, const Matrix4x4& m) {
	float x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
	float y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
	float z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
	float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];
	if (fabsf(w) > 1e-6f)
		return {x / w, y / w, z / w};
	return {x, y, z};
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;
	const int kSubdivision = 10;
	const float kStep = kGridHalfWidth * 2.0f / kSubdivision;
	for (int i = 0; i <= kSubdivision; i++) {
		float x = -kGridHalfWidth + kStep * i;
		Vector3 s = Transform(Transform({x, 0, -kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		Vector3 e = Transform(Transform({x, 0, kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		uint32_t color = (i == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine((int)s.x, (int)s.y, (int)e.x, (int)e.y, color);
	}
	for (int i = 0; i <= kSubdivision; i++) {
		float z = -kGridHalfWidth + kStep * i;
		Vector3 s = Transform(Transform({-kGridHalfWidth, 0, z}, viewProjectionMatrix), viewportMatrix);
		Vector3 e = Transform(Transform({kGridHalfWidth, 0, z}, viewProjectionMatrix), viewportMatrix);
		uint32_t color = (i == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine((int)s.x, (int)s.y, (int)e.x, (int)e.y, color);
	}
}

// OBBとOBBの衝突判定（分離軸定理 SAT）
bool IsCollision(const OBB& obb1, const OBB& obb2) {
	// 2つのOBBの中心間ベクトル
	Vector3 d = Subtract(obb2.center, obb1.center);

	// サイズを配列で扱う
	float s1[3] = {obb1.size.x, obb1.size.y, obb1.size.z};
	float s2[3] = {obb2.size.x, obb2.size.y, obb2.size.z};

	// 分離軸候補：obb1の3軸、obb2の3軸、それらのクロス積9軸 = 計15軸
	Vector3 axes[15];
	// obb1の軸
	axes[0] = obb1.orientations[0];
	axes[1] = obb1.orientations[1];
	axes[2] = obb1.orientations[2];
	// obb2の軸
	axes[3] = obb2.orientations[0];
	axes[4] = obb2.orientations[1];
	axes[5] = obb2.orientations[2];
	// クロス積軸
	int idx = 6;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			axes[idx++] = Cross(obb1.orientations[i], obb2.orientations[j]);
		}
	}

	for (int a = 0; a < 15; a++) {
		// ゼロベクトルのクロス積軸はスキップ
		if (LengthSquared(axes[a]) < 1e-10f)
			continue;

		Vector3 axis = Normalize(axes[a]);

		// 中心間距離をこの軸に投影
		float dist = fabsf(Dot(d, axis));

		// obb1の投影半径
		float r1 = 0.0f;
		for (int i = 0; i < 3; i++)
			r1 += fabsf(Dot(obb1.orientations[i], axis)) * s1[i];

		// obb2の投影半径
		float r2 = 0.0f;
		for (int i = 0; i < 3; i++)
			r2 += fabsf(Dot(obb2.orientations[i], axis)) * s2[i];

		// 分離軸が見つかれば非衝突
		if (dist > r1 + r2)
			return false;
	}

	// すべての軸で重なり → 衝突
	return true;
}

// OBBを描画する関数
void DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 axes[3] = {
	    Scale(obb.orientations[0], obb.size.x),
	    Scale(obb.orientations[1], obb.size.y),
	    Scale(obb.orientations[2], obb.size.z),
	};

	Vector3 v[8];
	for (int i = 0; i < 8; i++) {
		v[i] = obb.center;
		v[i] = Add(v[i], Scale(axes[0], (i & 1) ? 1.0f : -1.0f));
		v[i] = Add(v[i], Scale(axes[1], (i & 2) ? 1.0f : -1.0f));
		v[i] = Add(v[i], Scale(axes[2], (i & 4) ? 1.0f : -1.0f));
	}

	Vector3 s[8];
	for (int i = 0; i < 8; i++) {
		s[i] = Transform(Transform(v[i], viewProjectionMatrix), viewportMatrix);
	}

	int edges[12][2] = {
	    {0, 1},
        {2, 3},
        {4, 5},
        {6, 7},
        {0, 2},
        {1, 3},
        {4, 6},
        {5, 7},
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7},
	};
	for (auto& e : edges) {
		Novice::DrawLine((int)s[e[0]].x, (int)s[e[0]].y, (int)s[e[1]].x, (int)s[e[1]].y, color);
	}
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	Vector3 cameraTranslate{0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate{0.26f, 0.0f, 0.0f};

	// ImGuiで操作する回転角（度数法）
	Vector3 rotate1{0.0f, 0.0f, 0.0f};
	Vector3 rotate2{-0.05f, -2.49f, 0.15f};

	OBB obb1{
	    .center = {0.0f, 0.0f, 0.0f},
	    .orientations =
	        {
	               {1.0f, 0.0f, 0.0f},
	               {0.0f, 1.0f, 0.0f},
	               {0.0f, 0.0f, 1.0f},
	               },
	    .size = {0.83f, 0.26f, 0.24f},
	};

	OBB obb2{
	    .center = {0.9f, 0.66f, 0.78f},
	    .orientations =
	        {
	               {1.0f, 0.0f, 0.0f},
	               {0.0f, 1.0f, 0.0f},
	               {0.0f, 0.0f, 1.0f},
	               },
	    .size = {0.5f, 0.37f, 0.5f},
	};

	char keys[256] = {0};
	char preKeys[256] = {0};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		ImGui::Begin("Window");
		ImGui::DragFloat3("obb1.center", &obb1.center.x, 0.01f);
		ImGui::DragFloat("obb1.rotateX", &rotate1.x, 1.0f);
		ImGui::DragFloat("obb1.rotateY", &rotate1.y, 1.0f);
		ImGui::DragFloat("obb1.rotateZ", &rotate1.z, 1.0f);
		ImGui::DragFloat3("obb1.size", &obb1.size.x, 0.01f);
		ImGui::DragFloat3("obb2.center", &obb2.center.x, 0.01f);
		ImGui::DragFloat("obb2.rotateX", &rotate2.x, 1.0f);
		ImGui::DragFloat("obb2.rotateY", &rotate2.y, 1.0f);
		ImGui::DragFloat("obb2.rotateZ", &rotate2.z, 1.0f);
		ImGui::DragFloat3("obb2.size", &obb2.size.x, 0.01f);
		ImGui::End();

		// 回転行列からorientationsを更新
		auto UpdateOBBOrientation = [&](OBB& obb, const Vector3& rotate) {
			float rx = rotate.x * kPi / 180.0f;
			float ry = rotate.y * kPi / 180.0f;
			float rz = rotate.z * kPi / 180.0f;
			Matrix4x4 rotMat = Multiply(Multiply(MakeRotateXMatrix(rx), MakeRotateYMatrix(ry)), MakeRotateZMatrix(rz));
			obb.orientations[0] = Normalize({rotMat.m[0][0], rotMat.m[0][1], rotMat.m[0][2]});
			obb.orientations[1] = Normalize({rotMat.m[1][0], rotMat.m[1][1], rotMat.m[1][2]});
			obb.orientations[2] = Normalize({rotMat.m[2][0], rotMat.m[2][1], rotMat.m[2][2]});
		};

		UpdateOBBOrientation(obb1, rotate1);
		UpdateOBBOrientation(obb2, rotate2);

		Matrix4x4 cameraRotateMatrix = Multiply(Multiply(MakeRotateXMatrix(cameraRotate.x), MakeRotateYMatrix(cameraRotate.y)), MakeRotateZMatrix(cameraRotate.z));
		Matrix4x4 cameraMatrix = Multiply(cameraRotateMatrix, MakeTranslateMatrix(cameraTranslate));
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		bool collision = IsCollision(obb1, obb2);
		uint32_t color = collision ? 0xFF0000FF : 0xFFFFFFFF;

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);
		DrawOBB(obb1, viewProjectionMatrix, viewportMatrix, color);
		DrawOBB(obb2, viewProjectionMatrix, viewportMatrix, color);

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