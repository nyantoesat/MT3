#define _USE_MATH_DEFINES
#include <Novice.h>
#include <cmath>
#include <imgui.h>

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

struct Sphere {
	Vector3 center;
	float radius;
};

float LengthSquared(const Vector3& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
float Length(const Vector3& v) { return sqrtf(LengthSquared(v)); }

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

Matrix4x4 MakeScaleMatrix(const Vector3& s) {
	Matrix4x4 m = {
	    {{s.x, 0, 0, 0}, {0, s.y, 0, 0}, {0, 0, s.z, 0}, {0, 0, 0, 1}}
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

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 S = MakeScaleMatrix(scale);
	Matrix4x4 R = Multiply(Multiply(MakeRotateXMatrix(rotate.x), MakeRotateYMatrix(rotate.y)), MakeRotateZMatrix(rotate.z));
	Matrix4x4 T = MakeTranslateMatrix(translate);
	return Multiply(Multiply(S, R), T);
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

void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const int kLatDiv = 8;
	const int kLonDiv = 8;
	for (int lat = 0; lat < kLatDiv; lat++) {
		float theta0 = kPi * float(lat) / float(kLatDiv) - kPi / 2.0f;
		float theta1 = kPi * float(lat + 1) / float(kLatDiv) - kPi / 2.0f;
		for (int lon = 0; lon < kLonDiv; lon++) {
			float phi0 = 2.0f * kPi * float(lon) / float(kLonDiv);
			float phi1 = 2.0f * kPi * float(lon + 1) / float(kLonDiv);
			Vector3 a = {sphere.center.x + sphere.radius * cosf(theta0) * cosf(phi0), sphere.center.y + sphere.radius * sinf(theta0), sphere.center.z + sphere.radius * cosf(theta0) * sinf(phi0)};
			Vector3 b = {sphere.center.x + sphere.radius * cosf(theta1) * cosf(phi0), sphere.center.y + sphere.radius * sinf(theta1), sphere.center.z + sphere.radius * cosf(theta1) * sinf(phi0)};
			Vector3 c = {sphere.center.x + sphere.radius * cosf(theta0) * cosf(phi1), sphere.center.y + sphere.radius * sinf(theta0), sphere.center.z + sphere.radius * cosf(theta0) * sinf(phi1)};
			Vector3 sa = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
			Vector3 sb = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
			Vector3 sc = Transform(Transform(c, viewProjectionMatrix), viewportMatrix);
			Novice::DrawLine((int)sa.x, (int)sa.y, (int)sb.x, (int)sb.y, color);
			Novice::DrawLine((int)sa.x, (int)sa.y, (int)sc.x, (int)sc.y, color);
		}
	}
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	Vector3 cameraTranslate{0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate{0.26f, 0.0f, 0.0f};

	// 階層構造：[0]=肩, [1]=肘, [2]=手
	Vector3 translates[3] = {
	    {0.2f, 1.0f, 0.0f},
	    {0.4f, 0.0f, 0.0f},
	    {0.3f, 0.0f, 0.0f},
	};
	Vector3 rotates[3] = {
	    {0.0f, 0.0f, -6.8f},
	    {0.0f, 0.0f, -1.4f},
	    {0.0f, 0.0f, 0.0f },
	};
	Vector3 scales[3] = {
	    {1.0f, 1.0f, 1.0f},
	    {1.0f, 1.0f, 1.0f},
	    {1.0f, 1.0f, 1.0f},
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

		ImGui::SeparatorText("Shoulder [0]");
		ImGui::DragFloat3("translate[0]", &translates[0].x, 0.01f);
		ImGui::DragFloat3("rotate[0]", &rotates[0].x, 0.01f);
		ImGui::DragFloat3("scale[0]", &scales[0].x, 0.01f);

		ImGui::SeparatorText("Elbow [1]");
		ImGui::DragFloat3("translate[1]", &translates[1].x, 0.01f);
		ImGui::DragFloat3("rotate[1]", &rotates[1].x, 0.01f);
		ImGui::DragFloat3("scale[1]", &scales[1].x, 0.01f);

		ImGui::SeparatorText("Hand [2]");
		ImGui::DragFloat3("translate[2]", &translates[2].x, 0.01f);
		ImGui::DragFloat3("rotate[2]", &rotates[2].x, 0.01f);
		ImGui::DragFloat3("scale[2]", &scales[2].x, 0.01f);

		ImGui::End();

		// 階層行列を計算
		Matrix4x4 worldShoulder = MakeAffineMatrix(scales[0], rotates[0], translates[0]);
		Matrix4x4 worldElbow = Multiply(MakeAffineMatrix(scales[1], rotates[1], translates[1]), worldShoulder);
		Matrix4x4 worldHand = Multiply(MakeAffineMatrix(scales[2], rotates[2], translates[2]), worldElbow);

		// 各関節のワールド座標
		Vector3 posShoulder = Transform({0, 0, 0}, worldShoulder);
		Vector3 posElbow = Transform({0, 0, 0}, worldElbow);
		Vector3 posHand = Transform({0, 0, 0}, worldHand);

		Matrix4x4 cameraRotateMatrix = Multiply(Multiply(MakeRotateXMatrix(cameraRotate.x), MakeRotateYMatrix(cameraRotate.y)), MakeRotateZMatrix(cameraRotate.z));
		Matrix4x4 cameraMatrix = Multiply(cameraRotateMatrix, MakeTranslateMatrix(cameraTranslate));
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 関節間の線：肩-肘、肘-手
		Vector3 sShoulder = Transform(Transform(posShoulder, viewProjectionMatrix), viewportMatrix);
		Vector3 sElbow = Transform(Transform(posElbow, viewProjectionMatrix), viewportMatrix);
		Vector3 sHand = Transform(Transform(posHand, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine((int)sShoulder.x, (int)sShoulder.y, (int)sElbow.x, (int)sElbow.y, 0xFFFFFFFF);
		Novice::DrawLine((int)sElbow.x, (int)sElbow.y, (int)sHand.x, (int)sHand.y, 0xFFFFFFFF);

		// 肩：赤、肘：緑、手：青
		DrawSphere({posShoulder, 0.05f}, viewProjectionMatrix, viewportMatrix, 0xFF0000FF);
		DrawSphere({posElbow, 0.05f}, viewProjectionMatrix, viewportMatrix, 0x00FF00FF);
		DrawSphere({posHand, 0.05f}, viewProjectionMatrix, viewportMatrix, 0x0000FFFF);

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