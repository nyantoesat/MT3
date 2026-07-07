#define _USE_MATH_DEFINES
#include <Novice.h>
#include <cmath>
#include <imgui.h>

const char kWindowTitle[] = "GC2B_03_ニャン_トー_セッ";
const float kPi = 3.14159265358979323846f;
const int kWindowWidth = 1280;
const int kWindowHeight = 720;

struct Vector3 {
	float x;
	float y;
	float z;

	Vector3 operator+(const Vector3& v) const { return {x + v.x, y + v.y, z + v.z}; }
	Vector3 operator-(const Vector3& v) const { return {x - v.x, y - v.y, z - v.z}; }
	Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }
	Vector3 operator/(float s) const { return {x / s, y / s, z / s}; }
};

Vector3 Cross(const Vector3& a, const Vector3& b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
float Length(const Vector3& v) { return std::sqrt(Dot(v, v)); }
Vector3 Normalize(const Vector3& v) {
	float len = Length(v);
	if (len == 0.0f)
		return {0.0f, 0.0f, 0.0f};
	return v / len;
}

struct Matrix4x4 {
	float m[4][4];

	Matrix4x4 operator*(const Matrix4x4& other) const {
		Matrix4x4 result = {};
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				for (int k = 0; k < 4; k++)
					result.m[i][j] += m[i][k] * other.m[k][j];
		return result;
	}
};

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 r = {};
	for (int i = 0; i < 4; i++)
		r.m[i][i] = 1.0f;
	return r;
}


Matrix4x4 Inverse(const Matrix4x4& mat) {
	float a[4][8];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			a[i][j] = mat.m[i][j];
		for (int j = 0; j < 4; j++)
			a[i][4 + j] = (i == j) ? 1.0f : 0.0f;
	}

	for (int col = 0; col < 4; col++) {
		int pivotRow = col;
		float maxVal = std::fabs(a[col][col]);
		for (int row = col + 1; row < 4; row++) {
			if (std::fabs(a[row][col]) > maxVal) {
				maxVal = std::fabs(a[row][col]);
				pivotRow = row;
			}
		}
		if (pivotRow != col) {
			for (int k = 0; k < 8; k++)
				std::swap(a[col][k], a[pivotRow][k]);
		}

		float pivot = a[col][col];
		if (pivot != 0.0f) {
			for (int k = 0; k < 8; k++)
				a[col][k] /= pivot;
		}

		for (int row = 0; row < 4; row++) {
			if (row == col)
				continue;
			float factor = a[row][col];
			for (int k = 0; k < 8; k++)
				a[row][k] -= factor * a[col][k];
		}
	}

	Matrix4x4 result = {};
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			result.m[i][j] = a[i][4 + j];
	return result;
}

Matrix4x4 MakeRotateXMatrix(float angle) {
	Matrix4x4 r = {};
	r.m[0][0] = 1.0f;
	r.m[1][1] = std::cos(angle);
	r.m[1][2] = std::sin(angle);
	r.m[2][1] = -std::sin(angle);
	r.m[2][2] = std::cos(angle);
	r.m[3][3] = 1.0f;
	return r;
}

Matrix4x4 MakeRotateYMatrix(float angle) {
	Matrix4x4 r = {};
	r.m[0][0] = std::cos(angle);
	r.m[0][2] = -std::sin(angle);
	r.m[1][1] = 1.0f;
	r.m[2][0] = std::sin(angle);
	r.m[2][2] = std::cos(angle);
	r.m[3][3] = 1.0f;
	return r;
}

Matrix4x4 MakeRotateZMatrix(float angle) {
	Matrix4x4 r = {};
	r.m[0][0] = std::cos(angle);
	r.m[0][1] = std::sin(angle);
	r.m[1][0] = -std::sin(angle);
	r.m[1][1] = std::cos(angle);
	r.m[2][2] = 1.0f;
	r.m[3][3] = 1.0f;
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

Matrix4x4 MakeTranslateMatrix(const Vector3& t) {
	Matrix4x4 r = {};
	r.m[0][0] = 1.0f;
	r.m[1][1] = 1.0f;
	r.m[2][2] = 1.0f;
	r.m[3][0] = t.x;
	r.m[3][1] = t.y;
	r.m[3][2] = t.z;
	r.m[3][3] = 1.0f;
	return r;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 S = MakeScaleMatrix(scale);
	Matrix4x4 R = MakeRotateXMatrix(rotate.x) * MakeRotateYMatrix(rotate.y) * MakeRotateZMatrix(rotate.z);
	Matrix4x4 T = MakeTranslateMatrix(translate);
	return S * R * T;
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 r = {};
	float cot = 1.0f / std::tan(fovY / 2.0f);
	r.m[0][0] = cot / aspectRatio;
	r.m[1][1] = cot;
	r.m[2][2] = farClip / (farClip - nearClip);
	r.m[2][3] = 1.0f;
	r.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	return r;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 r = {};
	r.m[0][0] = width / 2.0f;
	r.m[1][1] = -height / 2.0f;
	r.m[2][2] = maxDepth - minDepth;
	r.m[3][0] = left + width / 2.0f;
	r.m[3][1] = top + height / 2.0f;
	r.m[3][2] = minDepth;
	r.m[3][3] = 1.0f;
	return r;
}

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	float x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + matrix.m[3][0];
	float y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + matrix.m[3][1];
	float z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + matrix.m[3][3];
	if (w != 0.0f) {
		x /= w;
		y /= w;
		z /= w;
	}
	return {x, y, z};
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);

	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + xIndex * kGridEvery;
		Vector3 start = Transform(Transform({x, 0.0f, -kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		Vector3 end = Transform(Transform({x, 0.0f, kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		unsigned int color = (x == 0.0f) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), color);
	}
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + zIndex * kGridEvery;
		Vector3 start = Transform(Transform({-kGridHalfWidth, 0.0f, z}, viewProjectionMatrix), viewportMatrix);
		Vector3 end = Transform(Transform({kGridHalfWidth, 0.0f, z}, viewProjectionMatrix), viewportMatrix);
		unsigned int color = (z == 0.0f) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), color);
	}
}

void DrawSphere(const Vector3& center, float radius, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, unsigned int color) {
	const uint32_t kSubdivision = 16;
	const float kLatEvery = kPi / float(kSubdivision);
	const float kLonEvery = 2.0f * kPi / float(kSubdivision);

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -kPi / 2.0f + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;

			Vector3 a = {center.x + radius * std::cos(lat) * std::cos(lon), center.y + radius * std::sin(lat), center.z + radius * std::cos(lat) * std::sin(lon)};
			Vector3 b = {center.x + radius * std::cos(lat + kLatEvery) * std::cos(lon), center.y + radius * std::sin(lat + kLatEvery), center.z + radius * std::cos(lat + kLatEvery) * std::sin(lon)};
			Vector3 c = {center.x + radius * std::cos(lat) * std::cos(lon + kLonEvery), center.y + radius * std::sin(lat), center.z + radius * std::cos(lat) * std::sin(lon + kLonEvery)};

			Vector3 screenA = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
			Vector3 screenB = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
			Vector3 screenC = Transform(Transform(c, viewProjectionMatrix), viewportMatrix);

			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenB.x), int(screenB.y), color);
			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenC.x), int(screenC.y), color);
		}
	}
}

// 振り子を表す構造体
struct Pendulum {
	Vector3 anchor;            // アンカーポイント。固定された端の位置
	float length;              // 紐の長さ
	float angle;               // 現在の角度
	float angularVelocity;     // 角速度ω
	float angularAcceleration; // 角加速度
};

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	char keys[256] = {0};
	char preKeys[256] = {0};

	// Camera transform (world position/rotation of the camera itself)
	Vector3 cameraTranslate{0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate{0.26f, 0.0f, 0.0f};

	const float kDeltaTime = 1.0f / 60.0f;

	// 振り子の初期値（課題実装例の通り）
	Pendulum pendulum;
	pendulum.anchor = {0.0f, 1.0f, 0.0f};
	pendulum.length = 0.8f;
	pendulum.angle = 0.7f;
	pendulum.angularVelocity = 0.0f;
	pendulum.angularAcceleration = 0.0f;

	const float kBallRadius = 0.05f;
	const unsigned int kBallColor = BLUE;

	bool isStarted = false;

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Matrix4x4 cameraWorldMatrix = MakeAffineMatrix({1.0f, 1.0f, 1.0f}, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraWorldMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix;
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		if (isStarted) {
			pendulum.angularAcceleration = -(9.8f / pendulum.length) * std::sin(pendulum.angle);
			pendulum.angularVelocity += pendulum.angularAcceleration * kDeltaTime;
			pendulum.angle += pendulum.angularVelocity * kDeltaTime;
		}

		// θ=0のとき(0,-1)方向を指すように振り子先端の位置を計算
		Vector3 ballPosition;
		ballPosition.x = pendulum.anchor.x + std::sin(pendulum.angle) * pendulum.length;
		ballPosition.y = pendulum.anchor.y - std::cos(pendulum.angle) * pendulum.length;
		ballPosition.z = pendulum.anchor.z;

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 紐の描画（アンカーからボール位置まで直線を引く）
		{
			Vector3 anchorScreen = Transform(Transform(pendulum.anchor, viewProjectionMatrix), viewportMatrix);
			Vector3 ballScreen = Transform(Transform(ballPosition, viewProjectionMatrix), viewportMatrix);
			Novice::DrawLine(int(anchorScreen.x), int(anchorScreen.y), int(ballScreen.x), int(ballScreen.y), WHITE);
		}

		DrawSphere(ballPosition, kBallRadius, viewProjectionMatrix, viewportMatrix, kBallColor);

		ImGui::Begin("Window");
		if (ImGui::Button("Start")) {
			isStarted = true;
		}
		ImGui::End();

		///
		/// ↑描画処理ここまで
		///

		Novice::EndFrame();
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0)
			break;
	}

	Novice::Finalize();
	return 0;
}